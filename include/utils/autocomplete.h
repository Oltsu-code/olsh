#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <string>
#include <vector>
#include <set>

namespace olsh::Utils {

class Autocomplete {
private:
    std::set<std::string> builtinCommands;
    std::set<std::string> pathExecutables;
    std::set<std::string> aliases;

    void loadPathExecutables();
    std::vector<std::string> getFilesInDirectory(const std::string& directory, const std::string& prefix = "");

public:
    Autocomplete();
    void updateAliases(const std::set<std::string>& aliasNames);
    std::vector<std::string> complete(const std::string& input, size_t cursorPos);
    std::vector<std::string> completeCommand(const std::string& prefix);
    std::vector<std::string> completeFile(const std::string& prefix);
};

} // namespace olsh::Utils

#endif //AUTOCOMPLETE_H
