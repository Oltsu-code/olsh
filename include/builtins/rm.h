#ifndef RM_H
#define RM_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Rm {
public:
    int execute(const std::vector<std::string>& args);
};

} // namespace olsh::Builtins

#endif //RM_H
