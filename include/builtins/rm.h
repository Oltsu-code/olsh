#ifndef RM_H
#define RM_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Rm {
public:
    int execute(const std::vector<std::string>& args);
};

}

#endif //RM_H
