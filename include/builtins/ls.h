#ifndef LS_H
#define LS_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Ls {
public:
    int execute(const std::vector<std::string>& args);
};

} // namespace olsh::Builtins

#endif //LS_H
