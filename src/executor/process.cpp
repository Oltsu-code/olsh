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
    STARTUPINFOA si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    std::vector<char> mutableCmd(cmdLine.begin(), cmdLine.end());
    mutableCmd.push_back('\0');

    DWORD creationFlags = CREATE_NEW_PROCESS_GROUP;

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
    s_processId = pi.dwProcessId;

    CloseHandle(pi.hThread);

    // wait
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
        if (!s_running.load(std::memory_order_acquire)) {
            exitCode = 130;
            break;
        }
    }

    CloseHandle(s_processHandle);
    s_processHandle = nullptr;
    s_processId = 0;
    s_running.store(false, std::memory_order_release);
    return exitCode;
#else
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

bool Process::interruptActive() {
    if (!s_running.load(std::memory_order_acquire)) return false;
#ifdef _WIN32
    if (s_processHandle && s_processId != 0) {
        if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, s_processId)) {
            TerminateProcess(s_processHandle, 130); // Standard Ctrl+C exit code
        }
        if (WaitForSingleObject(s_processHandle, 100) != WAIT_OBJECT_0) {
            TerminateProcess(s_processHandle, 130);
        }
        s_running.store(false, std::memory_order_release);
        return true;
    }
#else
    if (s_childPgid > 0) {
        // First try SIGINT (Ctrl+C equivalent)
        kill(-s_childPgid, SIGINT);
        // Immediately follow with SIGKILL for instant termination
        kill(-s_childPgid, SIGKILL);
        s_running.store(false, std::memory_order_release);
        return true;
    } else if (s_childPid > 0) {
        kill(s_childPid, SIGINT);
        kill(s_childPid, SIGKILL);
        s_running.store(false, std::memory_order_release);
        return true;
    }
#endif
    return false;
}

bool Process::isRunning() {
    return s_running.load(std::memory_order_acquire);
}

std::string Process::buildCommandLine(const std::string& command, const std::vector<std::string>& args) {
    std::string cmdLine = command;
    for (const auto& arg : args) {
        cmdLine.push_back(' ');
        cmdLine += arg;
    }
    return cmdLine;
}

}
