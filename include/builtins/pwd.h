#ifndef PWD_H
#define PWD_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Pwd {
public:
    int execute(const std::vector<std::string>& args);
};

} // namespace olsh::Builtins

#endif //PWD_H
