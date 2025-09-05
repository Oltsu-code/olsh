#ifndef OLSHELL_TOUCH_H
#define OLSHELL_TOUCH_H
#include <string>
#include <vector>

namespace olsh::Builtins {

class Touch {
public:
    int execute(const std::vector<std::string>& args);
};

} // namespace olsh::Builtins

#endif //OLSHELL_TOUCH_H