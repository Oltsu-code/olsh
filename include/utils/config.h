#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <filesystem>

namespace olsh::Utils {

class Config {
private:
    std::unordered_map<std::string, std::string> settings;
    std::filesystem::path configFilePath;
    
    void createDefaultConfig();
    void ensureConfigDirectoryExists();
    std::string getConfigDirectory();
    
public:
    Config();
    ~Config() = default;

    bool loadConfig();
    bool saveConfig();
    std::string getSetting(const std::string& key, const std::string& defaultValue = "") const;
    void setSetting(const std::string& key, const std::string& value);
    std::string getPrompt() const;
    void setPrompt(const std::string& prompt);
    bool configExists() const;
};

}  // namespace olsh::Utils

#endif // CONFIG_H
