#ifndef CP_H
#define CP_H

#include <string>
#include <vector>

namespace olsh::Builtins {
class Cp {
public:
    int execute(const std::vector<std::string>& args);
};

} // namespace olsh::Builtins

#endif //CP_H