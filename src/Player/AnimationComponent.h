//
// Created by ClapEcho233 on 2025/12/14.
//

#ifndef CELESTE_ANIMATION_H
#define CELESTE_ANIMATION_H

#include "../Common/Constant.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>

class AnimationComponent {
public:
    enum class State {
        Idle,
        Jump,
        Landing
    };
    
private:
    struct Animation {
        sf::Vector2f startScale; // 开始状态
        sf::Vector2f targetScale; // 目标状态
        float duration; // 持续时间
        float elapsed; // 当前时间
        std::function<float(float)> easing; // 插值回调函数
        bool recover; // 是否为自动恢复动画
    };

    State currentState;
    Animation currentAnimation;
    sf::Vector2f currentScale;
    sf::Vector2f baseScale;
    sf::Vector2f origin;

    // 动画参数
    float jumpSquashTime;
    float landingSquashTime;

    // 辅助函数
    void startAnimation(const Animation& anim);
    void updateAnimation();
    float easeOutBack(float t);
    float easeOutBounce(float t);
    float easeInOutSine(float t);

    // 状态处理
    void updateIdle();
    void updateJump();
    void updateLanding();

public:
    AnimationComponent();

    void update();
    void setState(State newState);
    void triggerJump();
    void triggerLanding(float impactForce = 1.0f);

    sf::Vector2f getScale() const { return currentScale; }
    sf::Vector2f getOrigin() const { return origin; }

    // 用于渲染
    sf::RectangleShape applyTransform(sf::RectangleShape player) const;
};


#endif //CELESTE_ANIMATION_H