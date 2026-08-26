#include "Game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init()) {
        std::cerr << "Initialization failed. Exiting game..." << std::endl;
        return -1;
    }

    std::cout << "Starting Tower Defense Game loop. Press ESC to quit." << std::endl;
    game.run();

    return 0;
}