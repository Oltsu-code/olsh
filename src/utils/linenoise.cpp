//* better way to do the terminal input handling

/*  
    TODO: every time a key is pressed, the fucking prompt gets duplicated
    why? i dont fucking know
    can someone smarter then me PLEASE HELP
*/

#include "utils/linenoise.h"
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
static linenoiseCompletionCallback* completion_callback = nullptr;
static olsh::Builtins::History* history_instance = nullptr;
static int history_index = -1;
static int max_history = 100;

extern "C" {

// set the history instance to use
void linenoiseSetHistoryInstance(olsh::Builtins::History* history) {
    history_instance = history;
}

// set the tab completion callback
void linenoiseSetCompletionCallback(linenoiseCompletionCallback* fn) {
    completion_callback = fn;
}

// add a completion option when tab is pressed
void linenoiseAddCompletion(linenoiseCompletions* lc, const char* str) {
    lc->cvec = (char**)realloc(lc->cvec, sizeof(char*) * (lc->len + 1));
    lc->cvec[lc->len] = strdup(str);
    lc->len++;
}

char* linenoise(const char* prompt) {
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
    
    SetConsoleCtrlHandler(NULL, TRUE);
    
    std::string input;
    size_t cursor_pos = 0;
    history_index = -1; 
    
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
        
        if ((controlKeys & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && (keyCode == 'C' || ch == 3)) {
            std::cout << "^C" << std::flush;
            input.clear();
            cursor_pos = 0;
            history_index = -1;
            std::cout << "\n" << prompt << std::flush;
            continue;
        }
        
        if (keyCode == VK_RETURN) {
            std::cout << '\n';
            break;
        }
        else if (keyCode == VK_BACK) {
            if (cursor_pos > 0) {
                input.erase(cursor_pos - 1, 1);
                cursor_pos--;
                
                std::cout << "\b \b" << std::flush;
            }
        }
        else if (keyCode == VK_TAB) {
            if (completion_callback) {
                linenoiseCompletions completions = {0, nullptr};
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
                    std::cout << "\r\033[K" << prompt << input << std::flush;
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
                
                std::cout << prompt << input << std::flush;
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
            
            std::cout << prompt << input << std::flush;
        }
        else if (keyCode == VK_LEFT) {
            // move cursor left
            if (cursor_pos > 0) {
                cursor_pos--;
                std::cout << '\b' << std::flush;
            }
        }
        else if (keyCode == VK_RIGHT) {
            // move cursor right
            if (cursor_pos < input.length()) {
                std::cout << input[cursor_pos] << std::flush;
                cursor_pos++;
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
    SetConsoleCtrlHandler(NULL, FALSE);
    
#else
    // TODO: make the linux users happy and do this shit
    std::string input;
    if (!std::getline(std::cin, input)) {
        return nullptr;
    }
#endif
    
    return strdup(input.c_str());
}

// add line to history
int linenoiseHistoryAdd(const char* line) {
    if (!line || strlen(line) == 0 || !history_instance) return 0;
    
    history_instance->addCommand(std::string(line));
    
    return 1;
}

int linenoiseHistorySetMaxLen(int len) {
    max_history = len;
    return 1;
}

int linenoiseHistorySave(const char* filename) {
    return 0;
}

int linenoiseHistoryLoad(const char* filename) {
    return 0;
}

void linenoiseClearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// free a linenoise result
void linenoiseFree(void* ptr) {
    free(ptr);
}

//* placeholder funcs
void linenoiseSetHintsCallback(linenoiseHintsCallback* fn) {  }
void linenoiseSetFreeHintsCallback(linenoiseFreeHintsCallback* fn) {  }
void linenoiseSetMultiLine(int ml) {  }
void linenoisePrintKeyCodes(void) {  }

}
