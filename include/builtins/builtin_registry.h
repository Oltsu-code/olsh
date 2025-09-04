#ifndef BUILTIN_REGISTRY_H
#define BUILTIN_REGISTRY_H

#pragma once
#include <unordered_map>
#include <functional>
#include <vector>
#include <string>

namespace olsh {

    class BuiltinRegistry {
    private:
        std::unordered_map<std::string, std::function<int(const std::vector<std::string>&)>> commands;

        void registerCommands();

    public:
        BuiltinRegistry();
        bool isBuiltin(const std::string& command) const;
        int execute(const std::string& command, const std::vector<std::string>& args) const;
        std::vector<std::string> getCommandNames() const;
    };

    extern BuiltinRegistry& getBuiltinRegistry();

} // namespace olsh



#endif //BUILTIN_REGISTRY_H

