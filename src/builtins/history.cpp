#include "../../include/builtins/history.h"
#include "../../include/utils/readline.h"
#include <utils/colors.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace olsh::Builtins {

History::History() : maxHistorySize(1000) {
    // get home
#ifdef _WIN32
    char* homeDir = getenv("USERPROFILE");
    if (homeDir != nullptr) {
        historyFile = std::string(homeDir) + "\\.olshell\\history";
    } else {
        historyFile = ".olsh_history";
    }
#else
    char* homeDir = getenv("HOME");
    if (homeDir != nullptr) {
        historyFile = std::string(homeDir) + "/.olshell/history";
    } else {
        historyFile = ".olsh_history";
    }
#endif

    loadHistory();
}

void History::loadHistory() {
    std::ifstream file(historyFile);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            historyList.push_back(line);
        }
    }
}

void History::saveHistory() {
    std::filesystem::path path(historyFile);
    std::filesystem::create_directory(path.parent_path()); // create directory if it doesn't exist

    std::ofstream file(historyFile);
    if (!file.is_open()) {
        return;
    }

    size_t start = historyList.size() > maxHistorySize ? historyList.size() - maxHistorySize : 0;
    for (size_t i = start; i < historyList.size(); i++) {
        file << historyList[i] << std::endl;
    }
}

void History::addCommand(const std::string& command) {
    if (command.empty() || command == "history") {
        return;
    }

    if (!historyList.empty() && historyList.back() == command) {
        return;
    }

    historyList.push_back(command);

    // trim history if it's too long
    if (historyList.size() > maxHistorySize * 1.5) {
        historyList.erase(historyList.begin(), historyList.begin() + (historyList.size() - maxHistorySize));
    }

    saveHistory();
}

int History::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        // all history
        for (size_t i = 0; i < historyList.size(); i++) {
            std::cout << std::setw(5) << (i + 1) << "  " << historyList[i] << std::endl;
        }
        return 0;
    }

    if (args[0] == "-c") {
        // clear history command
        historyList.clear();
        saveHistory();
        readlineHistoryReset(); // Reset the readline history navigation index
        std::cout << GREEN << "History cleared." << RESET << std::endl;
        std::cout << historyList.size() << " commands in history." << std::endl;
        return 0;
    }

    try {
        int n = std::stoi(args[0]);
        if (n > 0 && n <= static_cast<int>(historyList.size())) {
            // show last n commands
            size_t start = historyList.size() > static_cast<size_t>(n) ? historyList.size() - n : 0;
            for (size_t i = start; i < historyList.size(); i++) {
                std::cout << std::setw(5) << (i + 1) << "  " << historyList[i] << std::endl;
            }
        } else {
            std::cerr << RED << "history: invalid number: " << args[0] << RESET << std::endl;
            return 1;
        }
    } catch (const std::exception&) {
        std::cerr << RED << "history: invalid argument: " << args[0] << RESET << std::endl;
        return 1;
    }

    return 0;
}

std::vector<std::string> History::getHistory() const {
    return historyList;
}

std::string History::getCommand(size_t index) const {
    if (index < historyList.size()) {
        return historyList[index];
    }
    return "";
}

size_t History::size() const {
    return historyList.size();
}

bool History::saveToFile(const std::string& filename) {
    std::filesystem::path path(filename);
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    size_t start = historyList.size() > maxHistorySize ? historyList.size() - maxHistorySize : 0;
    for (size_t i = start; i < historyList.size(); i++) {
        file << historyList[i] << std::endl;
    }
    return true;
}

bool History::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    historyList.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            historyList.push_back(line);
        }
    }
    return true;
}

} // namespace olsh::Builtins
