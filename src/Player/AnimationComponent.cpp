//
// Created by ClapEcho233 on 2025/12/14.
//

#include "AnimationComponent.h"

AnimationComponent::AnimationComponent():
    currentState(State::Idle),
    currentScale(1.0f, 1.0f),
    baseScale(1.0f, 1.0f),
    origin(0.5f, 0.5f),  // 中心点
    jumpSquashTime(0.0f),
    landingSquashTime(0.0f) {

    currentAnimation = { baseScale, baseScale, 0.0f, 0.0f, nullptr, false };
}

void AnimationComponent::update() {
    // 更新当前动画
    updateAnimation();

    // 根据状态更新
    switch (currentState) {
        case State::Idle:
            updateIdle();
            break;
        case State::Jump:
            updateJump();
            break;
        case State::Landing:
            updateLanding();
            break;
        default:
            break;
    }

    // 限制缩放范围
    currentScale.x = std::max(0.1f, std::min(2.0f, currentScale.x));
    currentScale.y = std::max(0.1f, std::min(2.0f, currentScale.y));
}

void AnimationComponent::setState(State newState) {
    if (currentState == newState) return;
    currentState = newState;
    std::cout << "Animation State: " << static_cast<int>(newState) << std::endl;
}

void AnimationComponent::triggerJump() {
    setState(State::Jump);

    // 跳跃时的纵向拉伸动画
    Animation jumpAnim = {
        sf::Vector2f(1.0f, 1.0f),    // 开始：正常
        sf::Vector2f(0.8f, 1.2f),    // 目标：变瘦变高
        0.1f,                       // 持续时间
        0.0f,
        [this](float t) { return easeOutBack(t); },  // 弹性效果
        false
    };

    startAnimation(jumpAnim);
}

void AnimationComponent::triggerLanding(float impactForce) {
    setState(State::Landing);

    // 根据冲击力计算压缩程度
    float squashX = 1.0f + impactForce * 0.6f;      // 最大1.6倍宽
    float squashY = 1.0f - impactForce * 0.6f;      // 最小0.4倍高
    squashY = std::max(0.4f, squashY);

    // 落地压缩动画
    Animation landAnim = {
        sf::Vector2f(1.0f, 1.0f),     // 开始：正常
        sf::Vector2f(squashX, squashY), // 目标：压扁
        0.1f,                        // 压缩持续时间
        0.0f,
        [this](float t) { return easeOutBounce(t); }, // 弹跳效果
        false
    };

    startAnimation(landAnim);

    // 设置恢复动画
    landingSquashTime = 0.15f + 0.1f; // 压缩 + 恢复时间
}

sf::RectangleShape AnimationComponent::applyTransform(sf::RectangleShape player) const {
    player.setScale(currentScale);
    if (currentState == State::Landing) { // 落地动画需保证玩家下表面在地上
        player.move({0, player.getSize().y * (1 - currentScale.y) / 2});
    }
    return player;
}

void AnimationComponent::updateAnimation() {
    if (currentAnimation.duration <= 0.0f) return;

    currentAnimation.elapsed += deltaTime;
    float progress = currentAnimation.elapsed / currentAnimation.duration;

    if (progress >= 1.0f) {
        // 动画完成
        currentScale = currentAnimation.targetScale;
        currentAnimation.duration = 0.0f;
        if (currentAnimation.recover) {
            setState(State::Idle);
        } else {
            // 自动恢复
            Animation recoverAnim = {
                currentScale,
                baseScale,
                0.1f,
                0.0f,
                [this](float t) { return easeInOutSine(t); },
                true
            };
            startAnimation(recoverAnim);
        }
    } else {
        // 插值计算当前缩放
        float t = currentAnimation.easing ?
                  currentAnimation.easing(progress) : progress;

        currentScale.x = currentAnimation.startScale.x +
                        (currentAnimation.targetScale.x - currentAnimation.startScale.x) * t;
        currentScale.y = currentAnimation.startScale.y +
                        (currentAnimation.targetScale.y - currentAnimation.startScale.y) * t;
    }
}

void AnimationComponent::startAnimation(const Animation& anim) {
    currentAnimation = anim;
    currentAnimation.startScale = currentScale;
    currentAnimation.elapsed = 0.0f;
}

void AnimationComponent::updateIdle() {
    // 空闲状态的微小呼吸动画
    static float idleTimer = 0.0f;
    idleTimer += deltaTime;

    float breath = std::sin(idleTimer * 2.0f) * 0.04f; // 轻微波动
    currentScale.x = baseScale.x + breath;
    currentScale.y = baseScale.y - breath * 0.5f;
}

void AnimationComponent::updateJump() {
    // 没事干
}

void AnimationComponent::updateLanding() {
    // 没事干
}

float AnimationComponent::easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

float AnimationComponent::easeOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

float AnimationComponent::easeInOutSine(float t) {
    return -(std::cos(M_PI * t) - 1.0f) / 2.0f;
}