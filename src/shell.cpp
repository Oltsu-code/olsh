#include "../include/shell.h"
#include <iostream>
#include <filesystem>
#include <conio.h>
#include <set>
#include <sstream>

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
            processCommand(input);
        }
    }
}

void Shell::displayPrompt() {
    std::cout << "olsh:" << std::filesystem::current_path().filename().string() << "$ ";
}

std::string Shell::readInput() {
    std::string input;
    std::getline(std::cin, input);
    return input;
}

std::string Shell::readInputWithAutocomplete() {
    std::string input;
    std::getline(std::cin, input);

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

void Shell::processCommand(const std::string& input) {
    if (input == "exit") {
        exit();
        return;
    }

    // check if is a script file
    if (scriptInterpreter->isScriptFile(input)) {
        scriptInterpreter->executeScript(input);
        return;
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
    executor->execute(std::move(command));
}

void Shell::exit() {
    std::cout << "why quit :(\n";
    running = false;
}

}