#ifndef HELP_H
#define HELP_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Help {
public:
    int execute(const std::vector<std::string>& args);
};

}

#endif //HELP_H
