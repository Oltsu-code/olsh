#ifndef ECHO_H
#define ECHO_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Echo {
public:
    int execute(const std::vector<std::string>& args);
};

} // namespace olsh::Builtins

#endif //ECHO_H
