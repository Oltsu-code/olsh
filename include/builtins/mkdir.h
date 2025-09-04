#ifndef MKDIR_H
#define MKDIR_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Mkdir {
public:
    int execute(const std::vector<std::string>& args);
};

} // namespace olsh::Builtins

#endif //MKDIR_H
