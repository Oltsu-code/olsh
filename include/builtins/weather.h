#ifndef WEATHER_H
#define WEATHER_H

#include <string>
#include <vector>

namespace olsh::Builtins {

class Weather {
public:
    int execute(const std::vector<std::string>& args);

private:
    std::string getScriptPath();
    bool checkRubyInstalled();
};

}

#endif // WEATHER_H
