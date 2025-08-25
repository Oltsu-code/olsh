#include "../../include/builtins/clear.h"
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <cstdlib>
#endif

namespace olsh::Builtins {

int Clear::execute(const std::vector<std::string>& args) {
#ifdef _WIN32
    // windows clear
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0, 0};
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return 1;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten)) {
        return 1;
    }

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return 1;
    }

    if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) {
        return 1;
    }

    SetConsoleCursorPosition(hConsole, coordScreen);
#else
    // linux (better) clear
    system("clear");
#endif

    return 0;
}

}
