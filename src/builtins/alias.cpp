#include "../../include/builtins/alias.h"
#include "../../include/utils/fs.h"
#include <utils/colors.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>


#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace olsh::Builtins {

Alias::Alias() {
    // get home to store da aliases
#ifdef _WIN32
    char* homeDir = getenv("USERPROFILE");
    if (homeDir != nullptr) {
        aliasFile = std::string(homeDir) + "\\.olshell\\aliases";
    } else {
        aliasFile = ".olsh_aliases";
    }
#else
    char* homeDir = getenv("HOME");
    if (homeDir != nullptr) {
        aliasFile = std::string(homeDir) + "/.olshell/aliases";
    } else {
        aliasFile = ".olsh_aliases";
    }
#endif

    loadAliases();
}

void Alias::loadAliases() {
    std::ifstream file(aliasFile);
    if (!file.is_open()) {
        return; // no file
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string name = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);

            // remove quotes if present
            if (value.length() >= 2 && value[0] == '"' && value[value.length()-1] == '"') {
                value = value.substr(1, value.length() - 2);
            }

            aliases[name] = value;
        }
    }
}

void Alias::saveAliases() {
    std::filesystem::path path(aliasFile);

    // generate the dir if it doesnt exist
    if (!std::filesystem::exists(path.parent_path())) {
        std::filesystem::create_directory(path.parent_path());
    }

    std::ofstream file(aliasFile);
    if (!file.is_open()) {
        std::cerr << YELLOW << "Warning: Could not save aliases to " << aliasFile << RESET << std::endl;
        return;
    }

    file << "# OLShell aliases - automatically generated\n";
    for (const auto& pair : aliases) {
        file << pair.first << "=\"" << pair.second << "\"\n";
    }
}

int Alias::execute(const std::vector<std::string>& args) {

    // parse flags
    bool deleteMode = false;
    bool endOfOptions = false;
    std::vector<std::string> positional;

    for (const auto& arg : args) {
        if (!endOfOptions && !arg.empty() && arg[0] == '-' && arg.size() > 1) {
            if (arg == "--") { endOfOptions = true; continue; }
            if (arg.rfind("--", 0) == 0) {
                if (arg == "--delete") { deleteMode = true; }
                else {
                    std::cerr << RED << "alias: unrecognized option '" << arg << "'\n" << RESET;
                    std::cerr << "Usage: alias [-d|--delete] [--] [name[=value] | name value]" << std::endl;
                    return 1;
                }
                continue;
            }
            // short options bundle
            for (size_t j = 1; j < arg.size(); ++j) {
                switch (arg[j]) {
                    case 'd': deleteMode = true; break;
                    default:
                        std::cerr << RED << "alias: invalid option -- '" << arg[j] << "'\n" << RESET;
                        std::cerr << "Usage: alias [-d|--delete] [--] [name[=value] | name value]" << std::endl;
                        return 1;
                }
            }
        } else {
            positional.push_back(arg);
        }
    }

    // delete
    if (deleteMode) {
        if (positional.size() != 1) {
            std::cerr << RED << "alias: -d/--delete requires exactly one alias name\n" << RESET;
            std::cerr << "Usage: alias -d <name>" << std::endl;
            return 1;
        }
        const std::string& nameToDelete = positional[0];
        auto it = aliases.find(nameToDelete);
        if (it != aliases.end()) {
            aliases.erase(it);
            saveAliases();
            loadAliases();
            std::cout << "Alias '" << nameToDelete << "' deleted." << std::endl;
            return 0;
        } else {
            std::cerr << RED << "alias: " << nameToDelete << ": not found\n" << RESET;
            return 1;
        }
    }

    // list
    if (positional.empty()) {
        if (aliases.empty()) {
            std::cout << RED << "alias: no aliases defined." << RESET << std::endl;
        } else {
            for (const auto& pair : aliases) {
                std::cout << GREEN << "alias " << pair.first << "='" << pair.second << "'\n" << RESET;
            }
        }
        return 0;
    }

    // single positional (show alias or set with name=value without like spaces and shit)
    if (positional.size() == 1) {
        std::string name = positional[0];
        size_t eq = name.find('=');
        if (eq == std::string::npos) {
            // show alias
            auto it = aliases.find(name);
            if (it != aliases.end()) {
                std::cout << "alias " << it->first << "='" << it->second << "'\n";
                return 0;
            } else {
                std::cout << "alias: " << name << ": not found\n";
                return 1;
            }
        } else {
            // set alias with inline value
            std::string value = name.substr(eq + 1);
            name = name.substr(0, eq);
            if (name.empty() || value.empty()) {
                std::cerr << RED << "alias: invalid format. Use name=value or name value\n" << RESET;
                return 1;
            }
            aliases[name] = value;
            saveAliases();
            std::cout << "Alias '" << name << "' set to '" << value << "'\n";
            return 0;
        }
    }

    // multiple positionals (name = value)
    std::string name = positional[0];
    std::string value;

    size_t eq = name.find('=');
    if (eq != std::string::npos) {
        value = name.substr(eq + 1);
        name = name.substr(0, eq);
        if (positional.size() > 1) {
            std::ostringstream oss; if (!value.empty()) oss << value;
            for (size_t i = 1; i < positional.size(); ++i) {
                if (oss.tellp() > 0) oss << ' ';
                oss << positional[i];
            }
            value = oss.str();
        }
    } else {
        // name then value tokens
        std::ostringstream oss;
        for (size_t i = 1; i < positional.size(); ++i) {
            if (i > 1) oss << ' ';
            oss << positional[i];
        }
        value = oss.str();
    }

    if (name.empty()) {
        std::cerr << RED << "alias: invalid alias name\n" << RESET;
        return 1;
    }

    if (value.empty()) {
        std::cerr << RED << "alias: no value specified for alias '" << name << "'\n" << RESET;
        return 1;
    }

    aliases[name] = value;
    saveAliases();
    std::cout << "Alias '" << name << "' set to '" << value << "'\n";
    return 0;
}

std::string Alias::expandAlias(const std::string& command) {
    loadAliases(); // without this new aliases will not be found until restart
    auto it = aliases.find(command);
    if (it != aliases.end()) {
        return it->second;
    }
    return command;
}

std::map<std::string, std::string> Alias::getAliases() const {
    return aliases;
}

void Alias::setAlias(const std::string& name, const std::string& value) {
    aliases[name] = value;
    saveAliases();
}

void Alias::removeAlias(const std::string& name) {
    aliases.erase(name);
    saveAliases();
}

} // namespace olsh::Builtins
