#include "../../include/builtins/alias.h"
#include <utils/colors.h>
#include <filesystem>
#include "../../include/utils/fs.h"
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
        aliasFile = std::string(homeDir) + "/.olsh_aliases";
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
    std::filesystem::create_directory(path.parent_path()); // generate the dir if it doesnt exist

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

    // TODO: all this :arrow_down: is absolute shit and should be replaced with a for loop
    if (args.size() >= 1 && args[0] == "-d") {
        // delete alias
        if (args.size() != 2) {
            std::cerr << RED << "alias: -d requires exactly one argument\n" << RESET;
            return 1;
        }

        auto it = aliases.find(args[1]);
        if (it != aliases.end()) {
            aliases.erase(it);
            saveAliases();
            std::cout << "Alias '" << args[1] << "' deleted.\n";
            return 0;
        } else {
            std::cerr << RED << "alias: " << args[1] << ": not found\n" << RESET;
            return 1;
        }
    }

    // this i should propably make better
    if (args.empty()) {
        // list all
        if (aliases.empty()) {
            std::cout << RED << "alias: no aliases defined." << RESET << std::endl;
        } else {
            for (const auto& pair : aliases) {
                std::cout << GREEN << "alias " << pair.first << "='" << pair.second << "'\n" << RESET;
            }
        }
        return 0;
    }

    if (args.size() == 1) {
        // show alias
        auto it = aliases.find(args[0]);
        if (it != aliases.end()) {
            std::cout << "alias " << it->first << "='" << it->second << "'\n";
        } else {
            std::cout << "alias: " << args[0] << ": not found\n";
            return 1;
        }
        return 0;
    }

    // set alias: alias name=value or alias name value
    std::string name = args[0];
    std::string value;

    if (args.size() == 2) {
        // check if in format name=value
        size_t equalPos = name.find('=');
        if (equalPos != std::string::npos) {
            value = name.substr(equalPos + 1) + " " + args[1];
            name = name.substr(0, equalPos);
        } else {
            value = args[1];
        }
    } else {
        // join args
        std::ostringstream oss;
        for (size_t i = 1; i < args.size(); i++) {
            if (i > 1) oss << " ";
            oss << args[i];
        }
        value = oss.str();
    }

    // handle name=value format in the name itself
    size_t equalPos = name.find('=');
    if (equalPos != std::string::npos) {
        value = name.substr(equalPos + 1);
        name = name.substr(0, equalPos);

        if (value.empty() && args.size() > 1) {
            std::ostringstream oss;
            for (size_t i = 1; i < args.size(); i++) {
                if (i > 1) oss << " ";
                oss << args[i];
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

}
