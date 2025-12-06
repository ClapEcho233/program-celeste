#include "Core/Game.h"
#include "Common/Constant.h"

// 帧间隔时间
float deltaTime = 0;

int main() {
    freopen("nul", "w", stderr);

    Game game;

    while (game.isRunning()) {
        deltaTime = game.getDeltaTime();
        game.initialize();
        game.processEvent();
        game.update();
        game.render();
    }
}