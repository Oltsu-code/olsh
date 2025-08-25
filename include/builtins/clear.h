#ifndef CLEAR_H
#define CLEAR_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Clear {
public:
    int execute(const std::vector<std::string>& args);
};

}

#endif //CLEAR_H
