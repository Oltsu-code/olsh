#include "../../include/builtins/joke.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

namespace olsh::Builtins {

int Joke::execute(const std::vector<std::string>& args) { // asked gpt to make this file. pls dont judge me
    std::vector<std::string> jokes = {
        "Why do programmers prefer dark mode? Because light attracts bugs!",
        "How many programmers does it take to change a light bulb? None, that's a hardware problem!",
        "Why don't shells ever get tired? Because they always have their energy!",
        "What's a terminal's favorite type of music? Heavy metal!",
        "Why did the developer go broke? Because he used up all his cache!",
        "What do you call a programmer from Finland? Nerdic!",
        "Why do Java developers wear glasses? Because they can't C#!",
        "How do you comfort a JavaScript bug? You console it!",
        "Why did the programmer quit his job? He didn't get arrays!",
        "What's the object-oriented way to become wealthy? Inheritance!",
        "Why don't programmers like nature? It has too many bugs!",
        "What did the terminal say to the shell? You're my main process!",
        "Why do programmers hate the outdoors? Too much sunlight, not enough RGB!",
        "What's a computer's favorite beat? An algo-rhythm!",
        "Why did the shell script break up with the batch file? It wasn't cross-platform compatible!"
    };

    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<> distribution(0, jokes.size() - 1);

    int jokeIndex = distribution(generator);
    std::cout << jokes[jokeIndex] << std::endl;

    return 0;
}

}
