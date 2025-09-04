#include "../include/shell.h"
#include "../include/utils/fs.h"
#include "../include/builtins/config.h"
#include "../include/utils/linenoise.h"
#include "../include/executor/process.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <cctype>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

namespace olsh {

// static interrupt flag for proper ctrl+c handling
std::atomic<bool> Shell::s_interrupted{false};

#ifdef _WIN32
BOOL WINAPI Shell::signalHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        // if there's a running process, send signal to it and handle gracefully
        if (Process::isRunning()) {
            Process::interruptActive(); // this will mark process as not running
            return TRUE; // prevent shell from exiting
        } else {
            // no process running, notify shell for prompt handling
            Shell::notifyInterrupted();
            return TRUE; // we handled it
        }
    }
    return FALSE;
}
#else
void Shell::signalHandler(int signal) {
    if (signal == SIGINT) {
        // if there's a running process, try to interrupt it
        if (Process::isRunning()) {
            Process::interruptActive();
        } else {
            // no process running, notify shell
            Shell::notifyInterrupted();
        }
    }
}
#endif

void Shell::setupSignalHandlers() {
#ifdef _WIN32
    SetConsoleCtrlHandler(signalHandler, TRUE);
#else
    signal(SIGINT, signalHandler);
#endif
}

void Shell::cleanupSignalHandlers() {
#ifdef _WIN32
    SetConsoleCtrlHandler(NULL, FALSE);
#else
    signal(SIGINT, SIG_DFL);
#endif
}

Shell::Shell() : running(true) {
    // setup signal handlers first
    setupSignalHandlers();
    
    // init
    parser = std::make_unique<CommandParser>();
    executor = std::make_unique<Executor>();
    scriptInterpreter = std::make_unique<Utils::ScriptInterpreter>(this);
    aliasManager = std::make_unique<Builtins::Alias>();
    historyManager = std::make_unique<Builtins::History>();
    configManager = std::make_unique<Utils::Config>();
    autocompleteManager = std::make_unique<Utils::Autocomplete>();
    
    // give autocomplete access to aliases
    auto aliases = aliasManager->getAliases();
    std::set<std::string> aliasNames;
    for (const auto& pair : aliases) {
        aliasNames.insert(pair.first);
    }
    autocompleteManager->updateAliases(aliasNames);
    
    // initialize input manager
    inputManager = std::make_unique<Utils::InputManager>();
    Utils::InputManager::setShellInstance(this);

    // set shell instance for config builtin
    Builtins::Config::setShellInstance(this);

    // set history instance for linenoise
    linenoiseSetHistoryInstance(historyManager.get());

    // load history
    std::string historyFile = configManager->getSetting("config_dir", "") + "/.olshell/history";
    inputManager->loadHistory(historyFile);
    
    currentDirectory = std::filesystem::current_path().string();
}

Shell::~Shell() {
    // save history on exit
    std::string historyFile = configManager->getSetting("config_dir", "") + "/.olshell/history";
    inputManager->saveHistory(historyFile);
    
    // cleanup signal handlers
    cleanupSignalHandlers();
}

void Shell::run() {
    // show welcome message from config
    std::string welcomeMessage = configManager->getSetting("welcome_message",
        "OlShell - Type 'help' for available commands.");
    std::cout << welcomeMessage << "\n";

    while (running) {
        // check for pending interrupt before showing prompt
        if (s_interrupted.load(std::memory_order_acquire)) {
            s_interrupted.store(false, std::memory_order_release);
            std::cout << std::endl;
        }

        std::string promptStr = getPromptString();
        std::string input = inputManager->readLine(promptStr);

        // check for EOF (Ctrl+D)
        if (input == "\x04") {
            std::cout << "\nGoodbye!\n";
            exit();
            break;
        }

        // empty input (including from Ctrl+C)
        if (input.empty()) {
            continue;
        }

        // add to history
        historyManager->addCommand(input);

        // process the command
        (void)processCommand(input);
    }
}

void Shell::notifyInterrupted() {
    s_interrupted.store(true, std::memory_order_release);
}

std::string Shell::getPromptString() {
    std::string promptTemplate = configManager->getPrompt();

    // check if using the default fancy prompt
    if (promptTemplate.find("┌─({user}@{hostname})-[{cwd}]") != std::string::npos) {
        // use the original colored fancy prompt
        std::string user = "user";
        std::string hostname = "host";

#ifdef _WIN32
        char username[256];
        DWORD username_len = 256;
        if (GetUserNameA(username, &username_len)) {
            user = username;
        }

        char computerName[256];
        DWORD size = 256;
        if (GetComputerNameA(computerName, &size)) {
            hostname = computerName;
        }
#else
        const char* login = getlogin();
        if (login) user = login;

        char hostbuf[256];
        if (gethostname(hostbuf, sizeof(hostbuf)) == 0) {
            hostname = hostbuf;
        }
#endif

        std::string cwd = Utils::Fs::normalizePath(std::filesystem::current_path().string());

        // build the fancy colored prompt
        std::ostringstream prompt;
        prompt << BOLD_CYAN  << "┌─("
               << MAGENTA    << user << "@" << hostname
               << BOLD_CYAN  << ")-["
               << MAGENTA    << cwd
               << BOLD_CYAN  << "]\n"
               << BOLD_CYAN  << "└─$ "
               << RESET;
        
        return prompt.str();
    } else {
        // use custom prompt template
        return expandPromptVariables(promptTemplate) + RESET;
    }
}

std::string Shell::expandPromptVariables(const std::string& promptTemplate) {
    std::string result = promptTemplate;
    std::string user = "user";
    std::string hostname = "host";

#ifdef _WIN32
    char username[256];
    DWORD username_len = 256;
    if (GetUserNameA(username, &username_len)) {
        user = username;
    }

    char computerName[256];
    DWORD size = 256;
    if (GetComputerNameA(computerName, &size)) {
        hostname = computerName;
    }
#else
    const char* login = getlogin();
    if (login) user = login;

    char hostbuf[256];
    if (gethostname(hostbuf, sizeof(hostbuf)) == 0) {
        hostname = hostbuf;
    }
#endif

    std::string cwd = Utils::Fs::normalizePath(std::filesystem::current_path().string());

    // replace variables in the prompt template
    size_t pos = 0;
    while ((pos = result.find("{user}", pos)) != std::string::npos) {
        result.replace(pos, 6, user);
        pos += user.length();
    }
    
    pos = 0;
    while ((pos = result.find("{hostname}", pos)) != std::string::npos) {
        result.replace(pos, 10, hostname);
        pos += hostname.length();
    }
    
    pos = 0;
    while ((pos = result.find("{cwd}", pos)) != std::string::npos) {
        result.replace(pos, 5, cwd);
        pos += cwd.length();
    }

    return result;
}

int Shell::processCommand(const std::string& input) {
    // exit command (quick check before parsing)
    if (input == "exit") {
        exit();
        return 0;
    }

    // check for script execution (extract first word for script check)
    std::istringstream iss(input);
    std::string firstWord;
    if (!(iss >> firstWord)) {
        return 0; // empty input
    }

    if (scriptInterpreter->isScriptFile(firstWord)) {
        std::vector<std::string> args;
        std::string arg;
        while (iss >> arg) {
            args.push_back(arg);
        }
        return scriptInterpreter->executeScript(firstWord, args);
    }

    // expand aliases
    std::string expandedInput = input;
    std::string aliasExpansion = aliasManager->expandAlias(firstWord);
    if (aliasExpansion != firstWord) {
        std::string rest = input.substr(firstWord.length());
        expandedInput = aliasExpansion + rest;
    }

    // let the proper parser handle everything
    auto command = parser->parse(expandedInput);

    // validate command
    if (!command) {
        std::cerr << "Failed to parse command: " << expandedInput << std::endl;
        return -1;
    }

    // execute command
    return executor->execute(std::move(command));
}
void Shell::exit() {
    std::cout << BLUE << "Goodbye!\n" << RESET;
    running = false;
}

// autocomplete interface for input manager
std::vector<std::string> Shell::autocomplete(const std::string& input, size_t cursorPos) {
    if (autocompleteManager) {
        return autocompleteManager->complete(input, cursorPos);
    }
    return {};
}

} // namespace olsh
