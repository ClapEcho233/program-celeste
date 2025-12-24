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
    static constexpr float ShakeDeltaTime = 0.01; // 震动间隔
    static constexpr float ShakeNumber = 4;       // 震动次数
    static constexpr float ShakePixel = 10;       // 震动幅度（像素）
    static constexpr float TransitionHoldTime = 0.2;   // 转场暂停时间
    static constexpr float TransitionBlendTime = 0.65;  // 转场镜头平滑时间
    static constexpr float DeadWaitTime = 1;

    const sf::Color Normal = sf::Color(172, 50, 50);
    const sf::Color Used = sf::Color(68, 183, 255);

private:
    sf::RenderWindow window_;
    ViewportAspectRenderer renderer_;
    Line line_;
    sf::Clock clock_;
    LevelManager levelManager_;
    Player* player_;
    float deltaTime_;

    sf::Vector2f cameraPos_;       // 当前镜头中心（平滑后）
    sf::Vector2f cameraTarget_;    // 目标镜头中心（未平滑）
    sf::Vector2f shakeOffset_;     // 镜头震动偏移
    bool followPlayerX_;           // 是否锚定玩家 X
    bool followPlayerY_;           // 是否锚定玩家 Y
    float cameraFollowSpeed_;      // 镜头跟随平滑速度

    sf::Vector2f shakeDir_; // 震动方向
    float shakeSign_;       // 标志
    float shakeNumber_;     // 剩余震动次数
    float shakeDeltaTimer_; // 震动间隔计时器

    float deadWaitTimer_; // 死亡等待时间计时器

    bool transitioning_;          // 是否处于关卡切换
    float transitionHoldTimer_;   // 暂停阶段计时
    float transitionBlendTimer_;  // 镜头平滑阶段计时
    int pendingLevelId_;          // 目标关卡 id
    bool levelSwapped_;           // 是否已切换关卡
    sf::Vector2f frozenCameraPos_; // 暂停阶段固定镜头位置
    int previousLevelId_;         // 用于在转场时同时渲染上一关
    sf::Vector2f transitionDir_;  // 当前转场方向（指向离开本关的方向）
    sf::FloatRect currentTransitionArea_; // 触发转场的区域
    sf::Vector2f preTransitionPlayerPos_; // 记录触发瞬间的玩家位置
    sf::Vector2f preTransitionSpeed_;     // 记录触发瞬间的玩家速度

public:
    Game();

    /**
     * @return 是否仍在运行
     */
    bool isRunning() const;

    /**
     * 初始化游戏引擎，更新 deltaTime
     */
    void initialize();
    void processEvent();
    void update();
    void render();
    float getDeltaTime() const;

    void screenShake(sf::Vector2f dir); // 屏幕震动
    void shakeUpdate();
    void updateCamera(bool instant = false);
    void setCameraFollow(bool followX, bool followY);
    void checkLevelTransition();
    void startLevelTransition(const Level::Transition& transition);
    void updateLevelTransition();
    sf::Vector2f determineTransitionDir(const Level& level, const Level::Transition& t) const;
    void placePlayerInNextLevel();
    void deadReset(); // 玩家死亡重置

private:
    void handleEvent(const sf::Event::Closed&);
    void handleEvent(const sf::Event::Resized &resized);
    void handleEvent(const auto &) {}
};

#endif //CELESTE_GAME_H
