#ifndef CD_H
#define CD_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Cd {
public:
    int execute(const std::vector<std::string>& args);
};

}

#endif //CD_H
