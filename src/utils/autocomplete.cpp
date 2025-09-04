#include "../../include/utils/autocomplete.h"
#include "../../include/utils/fs.h"
#include "../../include/utils/windows_compat.h"
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

// TODO: fix this

namespace olsh::Utils {

Autocomplete::Autocomplete() {
    // builtins
    builtinCommands = {
        "cd", "ls", "pwd", "echo", "rm", "help", "clear", "cat", "alias", "history", "exit"
    };

    loadPathExecutables();
}

void Autocomplete::loadPathExecutables() {
    pathExecutables.clear();

#ifdef _WIN32
    // get path
    char* pathEnv = getenv("PATH");
    if (pathEnv != nullptr) {
        std::string pathStr(pathEnv);

        std::stringstream ss(pathStr);
        std::string path;

        while (std::getline(ss, path, ';')) {
            if (path.empty()) continue;

            try {
                if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
                    for (const auto& entry : std::filesystem::directory_iterator(path)) {
                        if (entry.is_regular_file()) {
                            std::string filename = entry.path().stem().string();
                            std::string ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                            if (ext == ".exe" || ext == ".bat" || ext == ".cmd" || ext == ".com") {
                                pathExecutables.insert(filename);
                            }
                        }
                    }
                }
            } catch (const std::exception&) {
                // ignore
            }
        }
    }
#else
    // linux
    char* pathEnv = getenv("PATH");
    if (pathEnv != nullptr) {
        std::string pathStr(pathEnv);
        std::stringstream ss(pathStr);
        std::string path;

        while (std::getline(ss, path, ':')) {
            if (path.empty()) continue;

            try {
                if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
                    for (const auto& entry : std::filesystem::directory_iterator(path)) {
                        if (entry.is_regular_file()) {
                            // check if its executable
                            struct stat st;
                            if (stat(entry.path().c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
                                pathExecutables.insert(entry.path().filename().string());
                            }
                        }
                    }
                }
            } catch (const std::exception&) {
                // ignore
            }
        }
    }
#endif
}

void Autocomplete::updateAliases(const std::set<std::string>& aliasNames) {
    aliases = aliasNames;
}

std::vector<std::string> Autocomplete::getFilesInDirectory(const std::string& directory, const std::string& prefix) {
    std::vector<std::string> results;

    try {
        std::string searchDir = directory.empty() ? "." : directory;

        if (std::filesystem::exists(searchDir) && std::filesystem::is_directory(searchDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(searchDir)) {
                std::string name = entry.path().filename().string();

                if (prefix.empty() || name.substr(0, prefix.length()) == prefix) {
                    if (entry.is_directory()) {
                        results.push_back(name + "/");
                    } else {
                        results.push_back(name);
                    }
                }
            }
        }
    } catch (const std::exception&) {
        // ignore
    }

    std::sort(results.begin(), results.end());
    return results;
}

std::vector<std::string> Autocomplete::completeCommand(const std::string& prefix) {
    std::vector<std::string> results;

    // check for the builtins
    for (const auto& cmd : builtinCommands) {
        if (prefix.empty() || cmd.substr(0, prefix.length()) == prefix) {
            results.push_back(cmd);
        }
    }

    // check aliases
    for (const auto& alias : aliases) {
        if (prefix.empty() || alias.substr(0, prefix.length()) == prefix) {
            results.push_back(alias);
        }
    }

    // check PATH
    for (const auto& exe : pathExecutables) {
        if (prefix.empty() || exe.substr(0, prefix.length()) == prefix) {
            results.push_back(exe);
        }
    }

    // check files in current directory
    auto localFiles = getFilesInDirectory(".", prefix);
    results.insert(results.end(), localFiles.begin(), localFiles.end());

    std::sort(results.begin(), results.end());
    results.erase(std::unique(results.begin(), results.end()), results.end());

    return results;
}

std::vector<std::string> Autocomplete::completeFile(const std::string& prefix) {
    std::string directory;
    std::string filename;

    size_t lastSlash = prefix.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        directory = prefix.substr(0, lastSlash);
        filename = prefix.substr(lastSlash + 1);
    } else {
        directory = ".";
        filename = prefix;
    }

    return getFilesInDirectory(directory, filename);
}

std::vector<std::string> Autocomplete::complete(const std::string& input, size_t cursorPos) {
    if (input.empty()) {
        return completeCommand("");
    }

    size_t wordStart = cursorPos;
    while (wordStart > 0 && input[wordStart - 1] != ' ') {
        wordStart--;
    }

    std::string currentWord = input.substr(wordStart, cursorPos - wordStart);

    bool isFirstWord = true;
    for (size_t i = 0; i < wordStart; i++) {
        if (input[i] != ' ') {
            isFirstWord = false;
            break;
        }
    }

    if (isFirstWord) {
        return completeCommand(currentWord);
    } else {
        return completeFile(currentWord);
    }
}

} // namespace olsh::Utils
