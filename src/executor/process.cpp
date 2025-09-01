#include "../../include/executor/process.h"
#include <utils/colors.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <atomic>

#ifdef _WIN32
#include <processthreadsapi.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#endif


namespace olsh {

// static state
std::atomic<bool> Process::s_running{false};
#ifdef _WIN32
HANDLE Process::s_processHandle = nullptr;
DWORD  Process::s_processId = 0;
#else
pid_t  Process::s_childPid = -1;
pid_t  Process::s_childPgid = -1;
#endif

    int Process::execute(const std::string& command, const std::vector<std::string>& args) {
        std::string cmdLine = buildCommandLine(command, args);

#ifdef _WIN32
        // Create process in its own group so we can deliver CTRL_C_EVENT to it without killing the shell
        STARTUPINFOA si{}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};

        // CreateProcess can modify the command line buffer; provide a writable copy
        std::vector<char> mutableCmd(cmdLine.begin(), cmdLine.end());
        mutableCmd.push_back('\0');

        DWORD creationFlags = CREATE_NEW_PROCESS_GROUP; // new process group for CTRL events

        if (!CreateProcessA(
                /*lpApplicationName*/ nullptr,
                /*lpCommandLine*/ mutableCmd.data(),
                /*lpProcessAttributes*/ nullptr,
                /*lpThreadAttributes*/ nullptr,
                /*bInheritHandles*/ TRUE, // allow inheriting redirected fds
                /*dwCreationFlags*/ creationFlags,
                /*lpEnvironment*/ nullptr,
                /*lpCurrentDirectory*/ nullptr,
                &si,
                &pi)) {
            std::cerr << RED << "Error: failed to start process: " << command << RESET << std::endl;
            return 1;
        }

        s_running.store(true, std::memory_order_release);
        s_processHandle = pi.hProcess;
        s_processId = pi.dwProcessId; // group id to target with GenerateConsoleCtrlEvent

        // We don't need thread handle
        CloseHandle(pi.hThread);

        // Wait
        DWORD waitRes;
        int exitCode = 0;
        while (true) {
            waitRes = WaitForSingleObject(s_processHandle, 50 /*ms*/);
            if (waitRes == WAIT_OBJECT_0) {
                DWORD code = 0;
                if (GetExitCodeProcess(s_processHandle, &code)) {
                    exitCode = static_cast<int>(code);
                } else {
                    exitCode = 1;
                }
                break;
            }
            // else WAIT_TIMEOUT: spin until complete; Ctrl-C handled by console handler
        }

        CloseHandle(s_processHandle);
        s_processHandle = nullptr;
        s_processId = 0;
        s_running.store(false, std::memory_order_release);
        return exitCode;
#else
        // Build argv for execvp
        std::vector<char*> argv;
        argv.reserve(args.size() + 2);
        argv.push_back(const_cast<char*>(command.c_str()));
        for (const auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);

        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << RED << "Error: fork failed for " << command << RESET << std::endl;
            return 1;
        }
        if (pid == 0) {
            // Child: create new process group and exec
            setpgid(0, 0);
            execvp(command.c_str(), argv.data());
            // If exec returns, it's an error
            std::perror("execvp");
            _exit(127);
        }

        // Parent: record child and wait
        s_childPid = pid;
        s_childPgid = pid; // as set by setpgid in child
        s_running.store(true, std::memory_order_release);

        int status = 0;
        for (;;) {
            pid_t w = waitpid(pid, &status, 0);
            if (w == -1) {
                if (errno == EINTR) continue; // interrupted by signal, retry
                std::perror("waitpid");
                status = 1;
                break;
            } else {
                break;
            }
        }

        s_running.store(false, std::memory_order_release);
        s_childPid = -1;
        s_childPgid = -1;

        if (WIFEXITED(status)) return WEXITSTATUS(status);
        if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
        return 1;
#endif
    }

    void Process::interruptActive() {
        if (!s_running.load(std::memory_order_acquire)) return;
#ifdef _WIN32
        // Send CTRL+C event to the child's process group
        if (s_processId != 0) {
            // Ensure this process ignores the event we might generate (handler already installed in shell)
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, s_processId);
        }
#else
        if (s_childPgid > 0) {
            kill(-s_childPgid, SIGINT);
        } else if (s_childPid > 0) {
            kill(s_childPid, SIGINT);
        }
#endif
    }

    bool Process::isRunning() {
        return s_running.load(std::memory_order_acquire);
    }

    std::string Process::buildCommandLine(const std::string& command, const std::vector<std::string>& args) {
#ifdef _WIN32
        // For CreateProcess, pass a single command line; quote args containing spaces/quotes
        std::string cmdLine;
        // Quote the command if it contains spaces
        auto needsQuote = [](const std::string& s){ return s.find_first_of(" \t\"\'") != std::string::npos; };
        if (needsQuote(command)) {
            cmdLine.push_back('"'); cmdLine += command; cmdLine.push_back('"');
        } else {
            cmdLine = command;
        }
        for (const auto& arg : args) {
            cmdLine.push_back(' ');
            if (!needsQuote(arg)) { cmdLine += arg; continue; }
            cmdLine.push_back('"');
            for (char c : arg) {
                if (c == '"') cmdLine.push_back('\\');
                cmdLine.push_back(c);
            }
            cmdLine.push_back('"');
        }
        return cmdLine;
#else
        // Not used on POSIX execvp path, but keep for parity (used nowhere)
        std::string cmdLine = command;
        for (const auto& a : args) { cmdLine.push_back(' '); cmdLine += a; }
        return cmdLine;
#endif
    }

}
