#ifndef CAT_H
#define CAT_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Cat {
public:
    int execute(const std::vector<std::string>& args);
};

}

#endif //CAT_H
