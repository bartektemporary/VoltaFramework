#include "VoltaFramework.hpp"
#include <filesystem>
namespace fs = std::filesystem;
#ifdef SCRIPTS_MAIN_EXISTS
#include "Game.hpp"
#endif
int main() {
    VoltaFramework game;
    #ifdef SCRIPTS_MAIN_EXISTS
    game.setGame(new Game());
    #endif
    game.run();
    return 0;
}