#include "Game.hpp"
#include <iostream>

int main() {
    try {
        Game game;
        game.run();
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL NA EXECUÇÃO: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}