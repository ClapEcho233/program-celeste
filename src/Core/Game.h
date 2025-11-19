//
// Created by ClapEcho233 on 2025/11/17.
//

#ifndef CELESTE_GAME_H
#define CELESTE_GAME_H

#include <SFML/Graphics.hpp>
#include "../Render/Render.h"
#include "../Render/Line.h"
#include <iostream>

class Game {
private:
    sf::RenderWindow window_;
    ViewportAspectRenderer renderer_;
    Line line_;

public:
    Game();

    /**
     * @return 是否正在运行
     */
    bool isRunning() const;

    /**
     * 事件处理
     */
    void processEvent();

    /**
     * 游戏状态更新
     */
    void update();

    /**
     * 渲染
     */
    void render();


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
