#include "../../include/builtins/clear.h"
#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace olsh::Builtins {

int Clear::execute(const std::vector<std::string>& args) {
    bool clearScrollback = false;

    for (const auto& a : args) {
        if (!a.empty() && a[0] == '-') {
            if (a == "-x" || a == "--scrollback") clearScrollback = true;
            else if (a == "--") break; // ignore anything after --
            else {
                std::cerr << "clear: unknown option '" << a << "'\n";
                std::cerr << "Usage: clear [-x|--scrollback]" << std::endl;
                return 1;
            }
        }
    }

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    bool vt = (hOut != INVALID_HANDLE_VALUE) && GetConsoleMode(hOut, &mode) && (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    if (vt) {
        if (clearScrollback) {
            std::cout << "\x1b[3J\x1b[H\x1b[2J";
        } else {
            std::cout << "\x1b[2J\x1b[H";
        }
        std::cout.flush();
        return 0;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
        return 1;
    }

    SMALL_RECT win = csbi.srWindow;
    SHORT width = win.Right - win.Left + 1;
    SHORT height = win.Bottom - win.Top + 1;

    if (clearScrollback) {
        COORD origin{0, 0};
        DWORD total = static_cast<DWORD>(csbi.dwSize.X) * static_cast<DWORD>(csbi.dwSize.Y);
        DWORD written = 0;
        FillConsoleOutputCharacter(hOut, ' ', total, origin, &written);
        FillConsoleOutputAttribute(hOut, csbi.wAttributes, total, origin, &written);
        SetConsoleCursorPosition(hOut, origin);
    } else {
        DWORD written = 0;
        for (SHORT row = win.Top; row <= win.Bottom; ++row) {
            COORD start{win.Left, row};
            FillConsoleOutputCharacter(hOut, ' ', width, start, &written);
            FillConsoleOutputAttribute(hOut, csbi.wAttributes, width, start, &written);
        }
        COORD home{win.Left, win.Top};
        SetConsoleCursorPosition(hOut, home);
    }
    return 0;
#else
    if (clearScrollback) {
        std::cout << "\x1b[3J\x1b[H\x1b[2J";
    } else {
        std::cout << "\x1b[2J\x1b[H";
    }
    std::cout.flush();
    return 0;
#endif
}

}
