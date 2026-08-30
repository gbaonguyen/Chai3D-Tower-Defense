#include "Game.h"
#include <iostream>

int main() {
    Game game;

    if (!game.init()) {
        std::cerr << "Initialization failed. Exiting game..." << std::endl;
        return -1;
    }

    std::cout << "Starting the Game. Press ESC to quit." << std::endl;
    game.run();

    return 0;
}