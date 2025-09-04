#include "builtins/config.h"
#include "../include/shell.h"
#include <utils/colors.h>
#include <iostream>
#include <iomanip>

namespace olsh::Builtins {

// static shell instance for access to config manager
Shell* Config::s_shell = nullptr;

void Config::setShellInstance(Shell* shell) {
    s_shell = shell;
}

int Config::execute(const std::vector<std::string>& args) {
    if (!s_shell) {
        std::cerr << RED << "config: Shell instance not set" << RESET << "\n";
        return 1;
    }

    bool showHelp = false;
    bool showConfig = false;
    bool getFlag = false;
    bool setFlag = false;
    std::string key, value;
    std::vector<std::string> positionalArgs;

    // parse flags 
    for (const auto& arg : args) {
        if (!arg.empty() && arg[0] == '-' && arg.size() > 1) {
            if (arg == "--help") {
                showHelp = true;
            } else if (arg == "--show" || arg == "--list") {
                showConfig = true;
            } else if (arg == "--get") {
                getFlag = true;
            } else if (arg == "--set") {
                setFlag = true;
            } else {
                for (size_t i = 1; i < arg.size(); i++) {
                    switch(arg[i]) {
                        case 'h': showHelp = true; break;
                        case 's': showConfig = true; break;
                        case 'g': getFlag = true; break;
                        case 'S': setFlag = true; break;
                        default:
                            std::cerr << RED << "config: invalid option: -" << arg[i] << RESET << "\n";
                            std::cerr << "Usage: config [-h|--help] [-s|--show] [-g|--get] [-S|--set] [key] [value]\n";
                            return 1;
                    }
                }
            }
        } else {
            positionalArgs.push_back(arg);
        }
    }

    // if no flags specified, show config by default
    if (!showHelp && !showConfig && !getFlag && !setFlag && positionalArgs.empty()) {
        showConfig = true;
    }

    // execute based on flags
    if (showHelp) {
        this->showHelp();
        return 0;
    }
    
    if (showConfig) {
        this->showCurrentConfig();
        return 0;
    }
    
    if (getFlag) {
        if (positionalArgs.empty()) {
            std::cerr << RED << "config: --get requires a key" << RESET << "\n";
            return 1;
        }
        return getConfig(positionalArgs[0]);
    }
    
    if (setFlag) {
        if (positionalArgs.size() < 2) {
            std::cerr << RED << "config: --set requires key and value" << RESET << "\n";
            return 1;
        }
        // join remaining args as value (in case value has spaces)
        std::string combinedValue = positionalArgs[1];
        for (size_t i = 2; i < positionalArgs.size(); ++i) {
            combinedValue += " " + positionalArgs[i];
        }
        return setConfig(positionalArgs[0], combinedValue);
    }

    // fallback - if positional args without flags, try to guess intent
    if (positionalArgs.size() == 1) {
        return getConfig(positionalArgs[0]);
    } else if (positionalArgs.size() >= 2) {
        std::string combinedValue = positionalArgs[1];
        for (size_t i = 2; i < positionalArgs.size(); ++i) {
            combinedValue += " " + positionalArgs[i];
        }
        return setConfig(positionalArgs[0], combinedValue);
    }

    showCurrentConfig();
    return 0;
}

void Config::showHelp() {
    std::cout << BOLD_CYAN << "Config Command Usage:" << RESET << "\n";
    std::cout << "  config                        - Show current configuration\n";
    std::cout << "  config " << BOLD_GREEN << "-s" << RESET << "|" << BOLD_GREEN << "--show" << RESET << "             - Show current configuration\n";
    std::cout << "  config " << BOLD_GREEN << "-g" << RESET << "|" << BOLD_GREEN << "--get" << RESET << " " << BOLD_YELLOW << "<key>" << RESET << "        - Get configuration value\n";
    std::cout << "  config " << BOLD_GREEN << "-S" << RESET << "|" << BOLD_GREEN << "--set" << RESET << " " << BOLD_YELLOW << "<key> <value>" << RESET << " - Set configuration value\n";
    std::cout << "  config " << BOLD_GREEN << "-h" << RESET << "|" << BOLD_GREEN << "--help" << RESET << "            - Show this help\n\n";
    
    std::cout << BOLD_CYAN << "Available Configuration Keys:" << RESET << "\n";
    std::cout << "  " << BOLD_YELLOW << "prompt" << RESET << "           - Shell prompt template\n";
    std::cout << "  " << BOLD_YELLOW << "welcome_message" << RESET << "  - Message shown on shell startup\n";
    std::cout << "  " << BOLD_YELLOW << "shell_name" << RESET << "       - Name of the shell\n";
    std::cout << "  " << BOLD_YELLOW << "version" << RESET << "          - Shell version\n\n";
    
    std::cout << BOLD_CYAN << "Prompt Template Variables:" << RESET << "\n";
    std::cout << "  " << BOLD_MAGENTA << "{user}" << RESET << "     - Current username\n";
    std::cout << "  " << BOLD_MAGENTA << "{hostname}" << RESET << " - Computer hostname\n";
    std::cout << "  " << BOLD_MAGENTA << "{cwd}" << RESET << "      - Current working directory\n";
    std::cout << "  " << BOLD_MAGENTA << "\\n" << RESET << "         - New line\n";
    std::cout << "  " << BOLD_MAGENTA << "\\t" << RESET << "         - Tab character\n\n";
    
    std::cout << BOLD_CYAN << "Examples:" << RESET << "\n";
    std::cout << "  config --set prompt \"$ \"\n";
    std::cout << "  config --set prompt \"{user}@{hostname}:{cwd}$ \"\n";
    std::cout << "  config --get prompt\n";
    std::cout << "  config --set welcome_message \"Welcome to OlShell!\"\n";
}

void Config::showCurrentConfig() {
    std::cout << BOLD_CYAN << "Current OlShell Configuration:" << RESET << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    auto* config = s_shell->getConfigManager();
    
    std::cout << std::left;
    std::cout << std::setw(20) << (std::string(BOLD_YELLOW) + "Setting" + RESET) << (std::string(BOLD_YELLOW) + "Value" + RESET) << "\n";
    std::cout << "────────────────────────────────────────────────────────────────────────────────\n";
    
    std::cout << std::setw(20) << "prompt" << config->getPrompt() << "\n";
    std::cout << std::setw(20) << "welcome_message" << config->getSetting("welcome_message", "Not set") << "\n";
    std::cout << std::setw(20) << "shell_name" << config->getSetting("shell_name", "OlShell") << "\n";
    std::cout << std::setw(20) << "version" << config->getSetting("version", "2.0") << "\n";
    
    std::cout << "\n" << BOLD_CYAN << "Configuration file location:" << RESET << " ~/.olshell/config.yaml\n";
    std::cout << "Use '" << BOLD_GREEN << "config --help" << RESET << "' for more information.\n";
}

int Config::setConfig(const std::string& key, const std::string& value) {
    auto* config = s_shell->getConfigManager();
    config->setSetting(key, value);
    
    if (config->saveConfig()) {
        std::cout << BOLD_GREEN << "✓" << RESET << " Set " << BOLD_YELLOW << key << RESET << " = " << BOLD_CYAN << value << RESET << "\n";
        if (key == "prompt" || key == "welcome_message") {
            std::cout << BOLD_BLUE << "ℹ" << RESET << " Changes will take effect immediately for new prompts.\n";
        }
        return 0;
    } else {
        std::cerr << RED << "✗ Failed to save configuration." << RESET << "\n";
        return 1;
    }
}

int Config::getConfig(const std::string& key) {
    auto* config = s_shell->getConfigManager();
    std::string value = config->getSetting(key, "");
    
    if (value.empty()) {
        std::cout << BOLD_YELLOW << key << RESET << ": " << BOLD_RED << "(not set)" << RESET << "\n";
    } else {
        std::cout << BOLD_YELLOW << key << RESET << ": " << BOLD_CYAN << value << RESET << "\n";
    }
    
    return 0;
}

} // namespace olsh::Builtins
