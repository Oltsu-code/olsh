#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>

namespace olsh {

class Process {
public:
    int execute(const std::string& command, const std::vector<std::string>& args);
private:
    std::string buildCommandLine(const std::string& command, const std::vector<std::string>& args);
};

}

#endif //PROCESS_H
