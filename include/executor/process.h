#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#endif

namespace olsh {

class Process {
public:
    int execute(const std::string& command, const std::vector<std::string>& args);

    static bool interruptActive();

    static bool isRunning();

private:
    std::string buildCommandLine(const std::string& command, const std::vector<std::string>& args);

    static std::atomic<bool> s_running;
#ifdef _WIN32
    static HANDLE s_processHandle;        // handle to the active process
    static DWORD  s_processId;            // active child pid (also process group id)
#else
    static pid_t  s_childPid;             // active child pid
    static pid_t  s_childPgid;            // active child process group id
#endif
};

} // namespace olsh

#endif //PROCESS_H
