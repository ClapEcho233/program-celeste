//
// Created by ClapEcho233 on 2025/11/17.
//

#ifndef CELESTE_GAME_H
#define CELESTE_GAME_H

#include <SFML/Graphics.hpp>
#include "../Render/ViewRender.h"
#include "../Render/Line.h"
#include <iostream>

class Game {
private:
    sf::RenderWindow window_;
    ViewportAspectRenderer renderer_;
    Line line_;
    sf::Clock clock_;
    float deltaTime_;

public:
    Game();

    /**
     * @return 是否正在运行
     */
    bool isRunning() const;

    /**
     * 初始化游戏引擎
     * 更新 deltaTime
     */
    void initialize();

    void processEvent();

    void update();

    void render();

    float getDeltaTime() const;

private:
    /**
     * 关闭窗口事件处理
     */
    void handleEvent(const sf::Event::Closed&);

    /**
     * 改变窗口尺寸事件处理
     * @param resized 改变后窗口对象
     */
    void handleEvent(const sf::Event::Resized &resized);

    void handleEvent(const auto &) {}
};

#endif //CELESTE_GAME_H
