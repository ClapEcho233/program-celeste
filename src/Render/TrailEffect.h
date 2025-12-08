//
// Created by ClapEcho233 on 2025/12/7.
//

#ifndef CELESTE_TRAILEFFECT_H
#define CELESTE_TRAILEFFECT_H

#include "../Common/Constant.h"
#include "../Common/Renderable.h"
#include <vector>
#include <SFML/Graphics.hpp>


class TrailEffect: public IRenderable{
private:
    static constexpr float TrailLifetime = 0.2; // 残影持续时间
    static constexpr float FadeDuration = 0.3;  // 淡出时间
private:
    sf::RectangleShape trail_;
    sf::Color color_;
    float trailLifeTime_;
    float fadeTime_;

public:
    TrailEffect(sf::Vector2f position, sf::Vector2f size, sf::Color color, float lifeTime, float fadeTime);

    bool isAlive();
    void update();
    void render(sf::RenderWindow& window) override;
};

class TrailEffectManager: public IRenderable {
private:
    std::vector<TrailEffect> trailEffects_;

public:
    TrailEffectManager();

    void create(sf::Vector2f position, sf::Vector2f size, sf::Color color);
    void update();
    void render(sf::RenderWindow& window) override;
};


#endif //CELESTE_TRAILEFFECT_H