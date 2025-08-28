#include "../include/shell.h"
#include "../include/utils/fs.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>
#include <conio.h>
#include <set>
#include <sstream>
#include <vector>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace olsh {

Shell::Shell() : running(true) {
    parser = std::make_unique<CommandParser>();
    executor = std::make_unique<Executor>();
    autocomplete = std::make_unique<Utils::Autocomplete>();
    scriptInterpreter = std::make_unique<Utils::ScriptInterpreter>(this);
    aliasManager = std::make_unique<Builtins::Alias>();
    historyManager = std::make_unique<Builtins::History>();
    currentDirectory = std::filesystem::current_path().string();

    auto aliases = aliasManager->getAliases();
    std::set<std::string> aliasNames;
    for (const auto& pair : aliases) {
        aliasNames.insert(pair.first);
    }
    autocomplete->updateAliases(aliasNames);
}

Shell::~Shell() = default;

void Shell::run() {
    // print da cool message
    std::cout << "OlShell v2.0 - Best shell ever made yk. Pls delete bash, zsh and every other shell u have on ur computer to use this.\n";
    std::cout << "run 'help' for commands and 'exit' to quit.\n\n";

    while (running) {
        displayPrompt();
        std::string input = readInputWithAutocomplete();
        if (!input.empty()) {
            historyManager->addCommand(input);
            (void)processCommand(input);
        }
    }
}

void Shell::displayPrompt() {
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

    std::cout
        << BOLD_CYAN  << "┌─("
        << MAGENTA    << user << "@" << hostname
        << BOLD_CYAN  << ")-["
        << MAGENTA    << cwd
        << BOLD_CYAN  << "]\n"
        << BOLD_CYAN  << "└─$ "
        << RESET;
}


std::string Shell::readInput() {
    std::string input;
    if (!std::getline(std::cin, input)) {
        running = false;
        return std::string();
    }
    return input;
}

std::string Shell::readInputWithAutocomplete() {
    std::string input;
    if (!std::getline(std::cin, input)) {
        running = false;
        return std::string();
    }

    return input;
}

void Shell::handleTabCompletion(std::string& input, size_t& cursorPos) {
    auto completions = autocomplete->complete(input, cursorPos);

    if (completions.empty()) {
        return;
    }

    if (completions.size() == 1) {
        size_t wordStart = cursorPos;
        while (wordStart > 0 && input[wordStart - 1] != ' ') {
            wordStart--;
        }

        std::string prefix = input.substr(wordStart, cursorPos - wordStart);
        std::string completion = completions[0];

        input.replace(wordStart, cursorPos - wordStart, completion);
        cursorPos = wordStart + completion.length();
    } else {
        std::cout << std::endl;
        for (const auto& completion : completions) {
            std::cout << completion << "  ";
        }
        std::cout << std::endl;
    }
}

static std::vector<std::string> split_args_quoted(const std::string& s) {
    std::vector<std::string> out; std::string cur; bool in_s=false, in_d=false;
    for (size_t i=0;i<s.size();++i){ char c=s[i];
        if (c=='\\' && i+1<s.size()) { cur.push_back(s[++i]); continue; }
        if (c=='"' && !in_s) { in_d=!in_d; continue; }
        if (c=='\'' && !in_d) { in_s=!in_s; continue; }
        if (!in_s && !in_d && std::isspace((unsigned char)c)) { if(!cur.empty()){ out.push_back(cur); cur.clear(); } }
        else cur.push_back(c);
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

int Shell::processCommand(const std::string& input) {
    if (input == "exit") {
        exit();
        return 0;
    }

    // detect script invocation with args
    std::vector<std::string> parts = split_args_quoted(input);
    if (!parts.empty() && scriptInterpreter->isScriptFile(parts[0])) {
        std::vector<std::string> args;
        if (parts.size() > 1) args.assign(parts.begin()+1, parts.end());
        return scriptInterpreter->executeScript(parts[0], args);
    }

    // expand aliases
    std::string expandedInput = input;
    std::istringstream iss(input);
    std::string firstWord;
    iss >> firstWord;

    if (!firstWord.empty()) {
        std::string aliasExpansion = aliasManager->expandAlias(firstWord);
        if (aliasExpansion != firstWord) {
            std::string rest = input.substr(firstWord.length());
            expandedInput = aliasExpansion + rest;
        }
    }

    // parse the command
    auto command = parser->parse(expandedInput);

    // execute the command
    return executor->execute(std::move(command));
}

void Shell::exit() {
    std::cout << "why quit :(\n";
    std::cout << RESET;
    running = false;
}

}