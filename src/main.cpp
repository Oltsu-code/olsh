#include "../include/shell.h"
#include "../include/utils/script.h"
#include "../include/executor/process.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

#ifdef _WIN32
BOOL WINAPI signalHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        // try to interrup any running process
        olsh::Process::interruptActive();
        return TRUE; // We handled it
    }
    return FALSE;
}
#else
void signalHandler(int signal) {
    if (signal == SIGINT) {
        // Just try to interrupt any running process - don't notify shell
        olsh::Process::interruptActive();
    }
}
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleCtrlHandler(signalHandler, TRUE);

    // force utf 8 (windows only idk if it works on linux)
    SetConsoleOutputCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#else
    signal(SIGINT, signalHandler);
#endif

    olsh::Shell shell;

    // script
    if (argc > 1) {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) args.emplace_back(argv[i]);
        olsh::Utils::ScriptInterpreter si(&shell);
        std::string file = argv[1];
        if (si.isScriptFile(file)) {
            int rc = si.executeScript(file, args);
            return rc;
        }
    }

    shell.run();
    return 0;
}