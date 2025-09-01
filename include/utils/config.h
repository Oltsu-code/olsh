#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <filesystem>

namespace olsh {
namespace Utils {

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
    
    // Load configuration from file
    bool loadConfig();
    
    // Save configuration to file
    bool saveConfig();
    
    // Get a setting value
    std::string getSetting(const std::string& key, const std::string& defaultValue = "") const;
    
    // Set a setting value
    void setSetting(const std::string& key, const std::string& value);
    
    // Get the configured prompt
    std::string getPrompt() const;
    
    // Set the prompt
    void setPrompt(const std::string& prompt);
    
    // Check if config file exists
    bool configExists() const;
};

} // namespace Utils
} // namespace olsh

#endif // CONFIG_H
