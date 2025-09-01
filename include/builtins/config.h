#ifndef CONFIG_BUILTIN_H
#define CONFIG_BUILTIN_H

#include <string>
#include <vector>

namespace olsh {
    class Shell;

namespace Builtins {

class Config {
public:
    int execute(const std::vector<std::string>& args);
    
    // set shell instance for config operations
    static void setShellInstance(Shell* shell);

private:
    void showHelp();
    void showCurrentConfig();
    int setConfig(const std::string& key, const std::string& value);
    int getConfig(const std::string& key);
    
    static Shell* s_shell;
};

} // namespace Builtins
} // namespace olsh

#endif // CONFIG_BUILTIN_H
