#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace olsh {
    class Shell;
}

namespace olsh::Utils {

struct FunctionDef {
    std::vector<std::string> body; // lines
};

class ScriptInterpreter {
private:
    olsh::Shell* shell;
    std::unordered_map<std::string, std::string> variables; // global vars
    std::unordered_map<std::string, FunctionDef> functions;

    // helpers
    std::string expandLine(const std::string& line,
                           int lastExitCode,
                           const std::vector<std::string>& args);
    std::string substituteBackticks(const std::string& line,
                                    int lastExitCode,
                                    const std::vector<std::string>& args);
    std::string expandVariables(const std::string& line,
                                int lastExitCode,
                                const std::vector<std::string>& args);
    std::string expandArithmetic(const std::string& line);
    long long evalArithmetic(const std::string& expr);
    bool evalCondition(const std::string& cond, int lastExitCode,
                       const std::vector<std::string>& args);

    int executeBlock(const std::vector<std::string>& lines,
                     std::vector<std::string> args,
                     int depth = 0);
    int executeLines(std::istringstream& stream,
                     std::vector<std::string> args,
                     int depth = 0);

public:
    ScriptInterpreter(olsh::Shell* shellInstance);
    bool isScriptFile(const std::string& filename);

    int executeScript(const std::string& filename);
    int executeScript(const std::string& filename,
                      const std::vector<std::string>& args);
    int executeScriptContent(const std::string& content);
    int executeScriptContent(const std::string& content,
                             const std::vector<std::string>& args);
};

}

#endif //SCRIPT_H
