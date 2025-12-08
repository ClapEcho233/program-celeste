//
// Created by ClapEcho233 on 2025/12/9.
//

/*
粒子分类（粒子都为小正方形）
灰尘粒子：运动较慢，朝四周发射后停留一段时间后淡出，控制在灰尘基本停止运动后淡出，为白色
冲刺粒子：
    轨迹粒子：按照冲刺方向呈现为一条线，停留一段时间后朝线两端淡出，为白色
    发散粒子：较为稀疏，方向为冲刺方向为中心的小范围扇形，速度快，颜色为冲刺后角色颜色
*/

#ifndef CELESTE_PARTICLE_H
#define CELESTE_PARTICLE_H

#include "../Common/Renderable.h"
#include "../Common/Constant.h"
#include <random>
#include <cmath>
#include <iostream>
#include <SFML/Graphics.hpp>

static std::mt19937 rarticleRng;
static std::uniform_real_distribution randomFloat(0.0f, 1.0f);

class Particle: public IRenderable {
private:
    sf::RectangleShape dot_; // 粒子对象
    sf::Vector2f position_; // 位置
    sf::Vector2f velocity_; // 速率
    sf::Color color_; // 颜色
    float dragForce_; // 阻力（单位时间下降的速度）
    float stayTimer_; // 停留时间计时器
    float fadeTimer_; // 淡出时间计时器
    float fadeTime_; // 淡出时间

public:
    Particle(const sf::Vector2f& position,
        const sf::Vector2f& size,
        const sf::Vector2f& velocity,
        const sf::Color& color,
        float dragForce,
        float stayTime,
        float fadeTime);

    void update();
    void render(sf::RenderWindow& window) override;

    bool isAlive() { return fadeTimer_ > 0;}
};

// 粒子发射&管理器
class ParticleEmitter: public IRenderable {
private:
    static constexpr float MaxStayTime = 0.9;  // 粒子最大持续时间
    static constexpr float FadeTime = 0.1;     // 粒子淡出时间

    const sf::Color Dust = sf::Color::White;                              // 灰尘颜色
    const sf::Color DashResidue = sf::Color(68, 183, 255); // 冲刺残留颜色

private:
    std::vector<Particle> particles_; // 粒子对象池

public:
    ParticleEmitter();

    void update();
    void render(sf::RenderWindow& window) override;

    // 粒子发射函数
    /**
     * 落地/起跳灰尘粒子
     * @param position 中心位置
     * @param intensity 强度 (0,1,2)
     * @param L 起始位置左边界 (相对于中心位置)
     * @param R 起始位置右边界 (相对于中心位置)
     */
    void emitLandingDust(const sf::Vector2f& position, float intensity, float L, float R);
};


#endif //CELESTE_PARTICLE_H