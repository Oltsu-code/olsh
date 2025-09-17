#ifndef OLSHELL_MV_H
#define OLSHELL_MV_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Mv {
public:
    int execute(const std::vector<std::string>& args);
};

}

#endif //OLSHELL_MV_H