#include "Core/Game.h"
#include "Common/Constant.h"

float deltaTime = 0;

 // 帧间隔时间
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