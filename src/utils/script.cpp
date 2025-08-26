#include "../../include/utils/script.h"
#include "../../include/shell.h"
#include <utils/colors.h>
#include <iostream>
#include <fstream>
#include <sstream>


namespace olsh::Utils {

ScriptInterpreter::ScriptInterpreter(olsh::Shell* shellInstance) : shell(shellInstance) {}

bool ScriptInterpreter::isScriptFile(const std::string& filename) {
    return filename.size() >= 5 && filename.substr(filename.size() - 5) == ".olsh";
}

int ScriptInterpreter::executeScript(const std::string& filename) {
    if (!isScriptFile(filename)) {
        std::cerr << RED << "Error: Not a valid .olsh script file: " << filename << RESET << std::endl;
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << RED << "Error: Cannot open script file: " << filename << RESET << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return executeScriptContent(buffer.str());
}

int ScriptInterpreter::executeScriptContent(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    int lineNumber = 0;
    int lastExitCode = 0;

    while (std::getline(stream, line)) {
        lineNumber++;

        // skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t pos = 0;
        while ((pos = line.find("$?", pos)) != std::string::npos) {
            line.replace(pos, 2, std::to_string(lastExitCode));
            pos += std::to_string(lastExitCode).length();
        }

        try {
            shell->processCommand(line);
            lastExitCode = 0; // assuming we win cz idk how to capture the exit code
        } catch (const std::exception& e) {
            std::cerr << RED << "Script error at line " << lineNumber << ": " << e.what() << RESET << std::endl;
            lastExitCode = 1;
        }
    }

    return lastExitCode;
}

}
