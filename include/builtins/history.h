#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class History {
private:
    std::vector<std::string> historyList;
    std::string historyFile;
    size_t maxHistorySize;

    void loadHistory();
    void saveHistory();

public:
    History();
    int execute(const std::vector<std::string>& args);
    void addCommand(const std::string& command);
    std::vector<std::string> getHistory() const;
    std::string getCommand(size_t index) const;
    size_t size() const;
    
    // public file operations for shell integration
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename);
};

}

#endif //HISTORY_H
