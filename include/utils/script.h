#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>

namespace olsh {
    class Shell;
}

namespace olsh::Utils {

class ScriptInterpreter {
private:
    olsh::Shell* shell;

public:
    ScriptInterpreter(olsh::Shell* shellInstance);
    int executeScript(const std::string& filename);
    int executeScriptContent(const std::string& content);
    bool isScriptFile(const std::string& filename);
};

}

#endif //SCRIPT_H
