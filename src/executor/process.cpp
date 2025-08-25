#include "../../include/executor/process.h"
#include <iostream>
#include <cstdlib>

namespace olsh {

    int Process::execute(const std::string& command, const std::vector<std::string>& args) {
        std::string cmdLine = buildCommandLine(command, args);

        try {
            int result = std::system(cmdLine.c_str());
            return result;
        } catch (const std::exception& e) {
            std::cerr << "Error executing command: " << e.what() << std::endl;
            return 1;
        }
    }

    std::string Process::buildCommandLine(const std::string& command, const std::vector<std::string>& args) {
        std::string cmdLine = command;

        for (const auto& arg : args) {
            cmdLine += " ";
            // quote arguments w spaces
            if (arg.find(' ') != std::string::npos) {
                cmdLine += "\"" + arg + "\"";
            } else {
                cmdLine += arg;
            }
        }

        return cmdLine;
    }

}