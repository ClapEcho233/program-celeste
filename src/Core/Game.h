//
// Created by ClapEcho233 on 2025/11/17.
//

#ifndef CELESTE_GAME_H
#define CELESTE_GAME_H

#include <SFML/Graphics.hpp>
#include "../Render/ViewRender.h"
#include "../Render/Line.h"
#include "../Level/Level.h"
#include "../Player/Player.h"
#include <iostream>

class Game {
private:
    static constexpr float ShakeDeltaTime = 0.01; // 震动时间间隔
    static constexpr float ShakeNumber = 4;       // 震动次数
    static constexpr float ShakePixel = 10;       // 震动幅度（像素）

private:
    sf::RenderWindow window_;
    ViewportAspectRenderer renderer_;
    Line line_;
    sf::Clock clock_;
    LevelManager levelManager_;
    Player* player_;
    float deltaTime_;

    sf::Vector2f shakeDir_; // 震动方向
    float shakeSign_;       // 标志
    float shakeNumber_;     // 剩余震动次数
    float shakeDeltaTimer_; // 震动间隔计时器

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

    void screenShake(sf::Vector2f dir); // 屏幕震动
    void shakeUpdate();

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
