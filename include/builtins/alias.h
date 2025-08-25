#ifndef ALIAS_H
#define ALIAS_H

#include <string>
#include <vector>
#include <map>

namespace olsh::Builtins {

class Alias {
private:
    std::map<std::string, std::string> aliases;
    std::string aliasFile;

    void loadAliases();
    void saveAliases();

public:
    Alias();
    int execute(const std::vector<std::string>& args);
    std::string expandAlias(const std::string& command);
    std::map<std::string, std::string> getAliases() const;
    void setAlias(const std::string& name, const std::string& value);
    void removeAlias(const std::string& name);
};

}

#endif //ALIAS_H
