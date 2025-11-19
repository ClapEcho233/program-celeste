#include "Core/Game.h"

int main() {
    freopen("nul", "w", stderr);

    Game game;

    while (game.isRunning()) {
        game.processEvent();
        game.update();
        game.render();
    }
}