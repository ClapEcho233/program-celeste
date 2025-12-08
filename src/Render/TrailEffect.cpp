//
// Created by ClapEcho233 on 2025/12/7.
//

#include "TrailEffect.h"

#include <iostream>

TrailEffect::TrailEffect(sf::Vector2f position, sf::Vector2f size, sf::Color color,
                         float lifeTime = TrailLifetime, float fadeTime = FadeDuration):
    trailLifeTime_(lifeTime), fadeTime_(fadeTime) {
    // 初始化残影对象
    trail_.setSize(size);
    trail_.setOrigin({trail_.getSize().x / 2, trail_.getSize().y / 2});
    trail_.setPosition(position);
    trail_.setFillColor(color);

    color_ = color;
}

void TrailEffect::update() {
    if (trailLifeTime_ > 0)
        trailLifeTime_ -= deltaTime;
    else if (fadeTime_ > 0)
        fadeTime_ -= deltaTime;
    color_.a = static_cast<int>(fadeTime_ / FadeDuration * 255.0f);
    trail_.setFillColor(color_);
}

void TrailEffect::render(sf::RenderWindow &window) {
    window.draw(trail_);
}

TrailEffectManager::TrailEffectManager() {
    trailEffects_.clear();
}

void TrailEffectManager::create(sf::Vector2f position, sf::Vector2f size, sf::Color color) {
    trailEffects_.emplace_back(position, size, color);
}

void TrailEffectManager::update() {
    for (auto& trail : trailEffects_) // 更新
        trail.update();
    while (!trailEffects_.empty() && !trailEffects_.front().isAlive()) // 删除死的残影
        trailEffects_.erase(trailEffects_.begin());
}

void TrailEffectManager::render(sf::RenderWindow &window) {
    for (auto& trail : trailEffects_)
        trail.render(window);
}




