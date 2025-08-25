#ifndef JOKE_H
#define JOKE_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Joke {
public:
    int execute(const std::vector<std::string>& args);
};

}

#endif //JOKE_H
