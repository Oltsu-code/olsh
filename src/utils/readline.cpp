#include "utils/readline.h"
#include "builtins/history.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

// global state because sometimes you gotta keep it simple
static readlineCompletionCallback* completion_callback = nullptr;
static olsh::Builtins::History* history_instance = nullptr;
static int history_index = -1;
static int max_history = 100;

// undo state tracking
static std::string undo_buffer;
static size_t undo_cursor_pos = 0;
static bool undo_available = false;

// helper function to save state for undo
static void saveUndoState(const std::string& current_input, size_t current_cursor_pos) {
    undo_buffer = current_input;
    undo_cursor_pos = current_cursor_pos;
    undo_available = true;
}

extern "C" {

// set the history instance to use
void readlineSetHistoryInstance(olsh::Builtins::History* history) {
    history_instance = history;
}

// set the tab completion callback
void readlineSetCompletionCallback(readlineCompletionCallback* fn) {
    completion_callback = fn;
}

// add a completion option when tab is pressed
void readlineAddCompletion(readlineCompletions* lc, const char* str) {
    lc->cvec = (char**)realloc(lc->cvec, sizeof(char*) * (lc->len + 1));
    lc->cvec[lc->len] = strdup(str);
    lc->len++;
}

char* readline(const char* prompt) {
    const char* prompt_end =
        (prompt ? (strrchr(prompt, '\n') ? strrchr(prompt, '\n') + 1 : prompt) : "");
    std::cout << prompt << std::flush;

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD originalMode;
    DWORD originalOutMode;
    GetConsoleMode(hConsole, &originalMode);
    GetConsoleMode(hConsoleOut, &originalOutMode);

    DWORD newMode = originalMode;
    newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(hConsole, newMode);

    DWORD newOutMode = originalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsoleOut, newOutMode);

    // dont disable ctrl handler - let signals flow naturally

    std::string input;
    size_t cursor_pos = 0;
    history_index = -1;
    
    // reset undo state for new line
    undo_available = false;

    while (true) {
        DWORD numEvents = 0;
        GetNumberOfConsoleInputEvents(hConsole, &numEvents);
        if (numEvents == 0) {
            Sleep(10);
            continue;
        }

        INPUT_RECORD inputRecord;
        DWORD numRead;
        if (!ReadConsoleInput(hConsole, &inputRecord, 1, &numRead)) {
            break;
        }

        if (inputRecord.EventType != KEY_EVENT || !inputRecord.Event.KeyEvent.bKeyDown) {
            continue;
        }

        WORD keyCode = inputRecord.Event.KeyEvent.wVirtualKeyCode;
        char ch = inputRecord.Event.KeyEvent.uChar.AsciiChar;
        DWORD controlKeys = inputRecord.Event.KeyEvent.dwControlKeyState;

        // ctrl key
        if (controlKeys & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
            switch (keyCode) {
                case 'C': // ctrl+c
                    std::cout << "\033[31m^C\033[0m\n";
                    input.clear();
                    cursor_pos = 0;
                    history_index = -1;
                    std::cout << prompt << std::flush;
                    continue;
                case 'Z': // ctrl+z (undo)
                    if (undo_available) {
                        // clear current line
                        std::cout << "\r\033[K" << prompt_end;
                        
                        // restore from undo buffer
                        input = undo_buffer;
                        cursor_pos = undo_cursor_pos;
                        
                        // display restored line
                        std::cout << input;
                        
                        // position cursor correctly
                        if (cursor_pos < input.length()) {
                            std::cout << std::string(input.length() - cursor_pos, '\b');
                        }
                        std::cout << std::flush;
                        
                        // clear undo (can only undo once)
                        undo_available = false;
                    }
                    continue;
                case 'D': // ctrl+d (EOF)
                    if (input.empty()) {
                        // exit on empty input
                        SetConsoleMode(hConsole, originalMode);
                        SetConsoleMode(hConsoleOut, originalOutMode);
                        return nullptr;
                    } else {
                        // delete character under cursor
                        if (cursor_pos < input.length()) {
                            saveUndoState(input, cursor_pos); // save state for undo
                            input.erase(cursor_pos, 1);
                            std::cout << "\033[K" << input.substr(cursor_pos) << std::flush;
                            if (cursor_pos < input.length()) {
                                std::cout << std::string(input.length() - cursor_pos, '\b') << std::flush;
                            }
                        }
                    }
                    continue;
                case 'L': // ctrl+l (clear)
                    std::cout << "\033[2J\033[H" << std::flush; // clear screen and move to top
                    std::cout << prompt << input << std::flush;
                    if (cursor_pos < input.length()) {
                        std::cout << std::string(input.length() - cursor_pos, '\b') << std::flush;
                    }
                    continue;
                case 'A': // ctrl+a (beginning of line)
                    std::cout << std::string(cursor_pos, '\b') << std::flush;
                    cursor_pos = 0;
                    continue;
                case 'E': // ctrl+e (end of line)
                    std::cout << input.substr(cursor_pos) << std::flush;
                    cursor_pos = input.length();
                    continue;
                case 'K': // ctrl+k (kill to end of line)
                    if (cursor_pos < input.length()) {
                        saveUndoState(input, cursor_pos); // save state for undo
                        input.erase(cursor_pos);
                        std::cout << "\033[K" << std::flush; // clear to end of line
                    }
                    continue;
                case 'U': // ctrl+u (kill entire line)
                    if (!input.empty()) {
                        saveUndoState(input, cursor_pos); // save state for undo
                        std::cout << std::string(cursor_pos, '\b') << std::flush; // move to beginning
                        input.clear();
                        cursor_pos = 0;
                        std::cout << "\033[K" << std::flush; // clear line
                    }
                    continue;
                case 'W': // ctrl+w (kill word backwards)
                    if (cursor_pos > 0) {
                        saveUndoState(input, cursor_pos); // save state for undo
                        size_t word_start = cursor_pos;
                        // skip whitespace
                        while (word_start > 0 && input[word_start - 1] == ' ') {
                            word_start--;
                        }
                        // find start of word
                        while (word_start > 0 && input[word_start - 1] != ' ') {
                            word_start--;
                        }
                        std::cout << std::string(cursor_pos - word_start, '\b') << std::flush;
                        input.erase(word_start, cursor_pos - word_start);
                        cursor_pos = word_start;
                        std::cout << "\033[K" << input.substr(cursor_pos) << std::flush;
                        if (cursor_pos < input.length()) {
                            std::cout << std::string(input.length() - cursor_pos, '\b') << std::flush;
                        }
                    }
                    continue;
                case 'R': // TODO: ctrl+r (reverse search)
                    std::cout << "\033[33m(reverse-i-search)`': \033[0m" << std::flush;
                    continue;
            }
        }

        // alt key
        if (controlKeys & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
            switch (keyCode) {
                case 'F': // alt+f (forward word)
                    if (cursor_pos < input.length()) {
                        // skip current word
                        while (cursor_pos < input.length() && input[cursor_pos] != ' ') {
                            std::cout << input[cursor_pos] << std::flush;
                            cursor_pos++;
                        }
                        // skip whitespace
                        while (cursor_pos < input.length() && input[cursor_pos] == ' ') {
                            std::cout << input[cursor_pos] << std::flush;
                            cursor_pos++;
                        }
                    }
                    continue;
                case 'B': // alt+b (backward word)
                    if (cursor_pos > 0) {
                        // skip whitespace backwards
                        while (cursor_pos > 0 && input[cursor_pos - 1] == ' ') {
                            cursor_pos--;
                            std::cout << '\b' << std::flush;
                        }
                        // skip word backwards
                        while (cursor_pos > 0 && input[cursor_pos - 1] != ' ') {
                            cursor_pos--;
                            std::cout << '\b' << std::flush;
                        }
                    }
                    continue;
                case 'D': // alt+d (delete word forward)
                    if (cursor_pos < input.length()) {
                        size_t word_end = cursor_pos;
                        // find end of current word
                        while (word_end < input.length() && input[word_end] != ' ') {
                            word_end++;
                        }
                        // skip trailing whitespace
                        while (word_end < input.length() && input[word_end] == ' ') {
                            word_end++;
                        }
                        input.erase(cursor_pos, word_end - cursor_pos);
                        std::cout << "\033[K" << input.substr(cursor_pos) << std::flush;
                        if (cursor_pos < input.length()) {
                            std::cout << std::string(input.length() - cursor_pos, '\b') << std::flush;
                        }
                    }
                    continue;
            }
        }

        if (keyCode == VK_RETURN) {
            std::cout << '\n';
            break;
        }
        else if (keyCode == VK_BACK) {
            if (cursor_pos > 0) {
                saveUndoState(input, cursor_pos); // save state for undo
                input.erase(cursor_pos - 1, 1);
                cursor_pos--;
                std::cout << "\b \b" << std::flush;
            }
        }
        else if (keyCode == VK_TAB) {
            // TODO: add some way to cycle trough multiple completions (if theres less then e.g. 5)

            // tab completion
            if (completion_callback) {
                readlineCompletions completions = {0, nullptr};
                completion_callback(input.c_str(), &completions);

                if (completions.len == 1) {
                    // single completion
                    size_t word_start = cursor_pos;
                    while (word_start > 0 && input[word_start - 1] != ' ') {
                        word_start--;
                    }

                    input.erase(word_start, cursor_pos - word_start);
                    input.insert(word_start, completions.cvec[0]);
                    cursor_pos = word_start + strlen(completions.cvec[0]);

                    // redraw the line properly
                    std::cout << "\r\033[K" << prompt_end << input << std::flush;
                } else if (completions.len > 1) {
                    // multiple completions, show them
                    std::cout << '\n';
                    for (size_t i = 0; i < completions.len; ++i) {
                        std::cout << completions.cvec[i];
                        if ((i + 1) % 4 == 0 || i == completions.len - 1) {
                            std::cout << '\n';
                        } else {
                            std::cout << "  ";
                        }
                    }
                    // redraw prompt after showing completions
                    std::cout << prompt << input << std::flush;
                    if (cursor_pos < input.length()) {
                        std::cout << std::string(input.length() - cursor_pos, '\b') << std::flush;
                    }
                }

                // cleanup completion memory
                for (size_t i = 0; i < completions.len; ++i) {
                    free(completions.cvec[i]);
                }
                free(completions.cvec);
            }
        }
        else if (keyCode == VK_UP) {
            // history navigation up
            if (history_instance && history_instance->size() > 0 && history_index < (int)history_instance->size() - 1) {
                history_index++;
                input = history_instance->getCommand(history_instance->size() - 1 - history_index);
                cursor_pos = input.length();

                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(hConsoleOut, &csbi);
                COORD start = {0, csbi.dwCursorPosition.Y};
                DWORD written;
                FillConsoleOutputCharacterA(hConsoleOut, ' ', csbi.dwSize.X, start, &written);
                SetConsoleCursorPosition(hConsoleOut, start);

                std::cout << "\r\033[K" << prompt_end << input << std::flush;
            }
        }
        else if (keyCode == VK_DOWN) {
            // history navigation down
            if (history_index > 0) {
                history_index--;
                input = history_instance->getCommand(history_instance->size() - 1 - history_index);
                cursor_pos = input.length();
            } else if (history_index == 0) {
                history_index = -1;
                input.clear();
                cursor_pos = 0;
            }

            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsoleOut, &csbi);
            COORD start = {0, csbi.dwCursorPosition.Y};
            DWORD written;
            FillConsoleOutputCharacterA(hConsoleOut, ' ', csbi.dwSize.X, start, &written);
            SetConsoleCursorPosition(hConsoleOut, start);

            std::cout << "\r\033[K" << prompt_end << input << std::flush;
        }
        else if (keyCode == VK_LEFT) {
            // move cursor left
            if (controlKeys & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
                // Ctrl+Left: jump word backward
                if (cursor_pos > 0) {
                    // skip whitespace backwards
                    while (cursor_pos > 0 && input[cursor_pos - 1] == ' ') {
                        cursor_pos--;
                        std::cout << '\b' << std::flush;
                    }
                    // skip word backwards
                    while (cursor_pos > 0 && input[cursor_pos - 1] != ' ') {
                        cursor_pos--;
                        std::cout << '\b' << std::flush;
                    }
                }
            } else {
                // regular left arrow
                if (cursor_pos > 0) {
                    cursor_pos--;
                    std::cout << '\b' << std::flush;
                }
            }
        }
        else if (keyCode == VK_RIGHT) {
            // move cursor right
            if (controlKeys & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
                // Ctrl+Right: jump word forward
                if (cursor_pos < input.length()) {
                    // skip current word
                    while (cursor_pos < input.length() && input[cursor_pos] != ' ') {
                        std::cout << input[cursor_pos] << std::flush;
                        cursor_pos++;
                    }
                    // skip whitespace
                    while (cursor_pos < input.length() && input[cursor_pos] == ' ') {
                        std::cout << input[cursor_pos] << std::flush;
                        cursor_pos++;
                    }
                }
            } else {
                // regular right arrow
                if (cursor_pos < input.length()) {
                    std::cout << input[cursor_pos] << std::flush;
                    cursor_pos++;
                }
            }
        }
        else if (keyCode == VK_HOME) {
            // move to beginning of line
            std::cout << std::string(cursor_pos, '\b') << std::flush;
            cursor_pos = 0;
        }
        else if (keyCode == VK_END) {
            // move to end of line
            std::cout << input.substr(cursor_pos) << std::flush;
            cursor_pos = input.length();
        }
        else if (keyCode == VK_DELETE) {
            // delete character under cursor
            if (cursor_pos < input.length()) {
                saveUndoState(input, cursor_pos); // save state for undo
                input.erase(cursor_pos, 1);
                std::cout << "\033[K" << input.substr(cursor_pos) << std::flush;
                if (cursor_pos < input.length()) {
                    std::cout << std::string(input.length() - cursor_pos, '\b') << std::flush;
                }
            }
        }
        else if (ch >= 32 && ch <= 126) {
            // regular printable character
            input.insert(cursor_pos, 1, ch);
            cursor_pos++;

            // just print the character naturally
            std::cout << ch << std::flush;
            if (cursor_pos < input.length()) {
                // shift the rest of the line
                std::cout << input.substr(cursor_pos) << std::flush;
                std::cout << std::string(input.length() - cursor_pos, '\b') << std::flush;
            }
        }
    }

    SetConsoleMode(hConsole, originalMode);
    SetConsoleMode(hConsoleOut, originalOutMode);
    // don't modify ctrl handler here

#else
    // Linux implementation - basic for now but functional
    std::string input;
    if (!std::getline(std::cin, input)) {
        return nullptr; // EOF or error
    }
#endif

    return strdup(input.c_str());
}

// add line to history
int readlineHistoryAdd(const char* line) {
    if (!line || strlen(line) == 0 || !history_instance) return 0;

    history_instance->addCommand(std::string(line));

    return 1;
}

int readlineHistorySetMaxLen(int len) {
    max_history = len;
    return 1;
}

int readlineHistorySave(const char* filename) {
    if (!filename || !history_instance) return -1;
    return history_instance->saveToFile(std::string(filename)) ? 0 : -1;
}

int readlineHistoryLoad(const char* filename) {
    if (!filename || !history_instance) return -1;
    return history_instance->loadFromFile(std::string(filename)) ? 0 : -1;
}

void readlineHistoryReset(void) {
    history_index = -1;
}

void readlineClearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// free a readline result
void readlineFree(void* ptr) {
    free(ptr);
}

// *placeholder funcs for features not yet implemented
void readlineSetHintsCallback(readlineHintsCallback* fn) {  }
void readlineSetFreeHintsCallback(readlineFreeHintsCallback* fn) {  }
void readlineSetMultiLine(int ml) {  }
void readlinePrintKeyCodes(void) {  }

}
