//
// Created by ClapEcho233 on 2025/12/9.
//

#include "Particle.h"

Particle::Particle(const sf::Vector2f &position, const sf::Vector2f &size, const sf::Vector2f &velocity,
    const sf::Color &color, float dragForce, float stayTime, float fadeTime) :
    dragForce_(dragForce),
    stayTimer_(stayTime),
    fadeTimer_(fadeTime),
    fadeTime_(fadeTime) {
    position_ = position;
    dot_.setSize(size);
    velocity_ = velocity;
    color_ = color;
}

void Particle::update() {
    // 初始化随机器
    std::uniform_int_distribution dir(-1, 1);

    // 更新计时器
    if (stayTimer_ > 0)
        stayTimer_ -= deltaTime;
    else if (fadeTimer_ > 0)
        fadeTimer_ -= deltaTime;
    else return ;

    // 更新颜色
    color_.a = static_cast<int>(fadeTimer_ / fadeTime_ * 255.0f);

    // 应用速度
    position_ += velocity_ * deltaTime;
    position_ += sf::Vector2f(10.0 * dir(rarticleRng), 10.0 * dir(rarticleRng)) *
                    (randomFloat(rarticleRng) * deltaTime); // 随机位移

    // 应用阻力
    float newSpeed = std::max (0.0f, std::hypot(velocity_.x, velocity_.y) - dragForce_ * deltaTime);
    if (velocity_ != sf::Vector2f(0, 0))
        velocity_ = velocity_.normalized() * newSpeed;
}

void Particle::render(sf::RenderWindow &window) {
    dot_.setPosition({static_cast<float>(std::round(position_.x / 10.0) * 10.0),
                         static_cast<float>(std::round(position_.y / 10.0) * 10.0)});
    dot_.setFillColor(color_);
    window.draw(dot_);
}

ParticleEmitter::ParticleEmitter() {
    particles_.clear();
}

void ParticleEmitter::update() {
    // 更新粒子
    for (auto& particle : particles_)
        particle.update();

    // 删除死亡粒子 (swap-and-pop)
    for (int i = 0; i < particles_.size();) {
        if (! particles_[i].isAlive()) {
            particles_[i] = particles_.back();
            particles_.pop_back();
        } else {
            ++ i;
        }
    }
}

void ParticleEmitter::render(sf::RenderWindow &window) {
    for (auto& particle : particles_)
        particle.render(window);
}

void ParticleEmitter::emitLandingDust(const sf::Vector2f &position, float intensity, float L, float R) {
    // 根据强度决定粒子数量
    int particleCount = static_cast<int>(5 + intensity * 15);

    // 初始化随机
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(-90.0, 90.0); // 角度扩散
    std::uniform_real_distribution<float> speedDist(25.0f, 75.0f * intensity); // 速度基于强度

    for (int i = 0; i < particleCount; ++i) {
        // 随机角度（转换为弧度）
        float angle = angleDist(gen) * std::numbers::pi / 180.0f;

        // 随机位置
        sf::Vector2f nowPositon = position;
        nowPositon.x = nowPositon.x + L + (R - L) * randomFloat(gen);
        nowPositon.y = nowPositon.y - 22.0 * randomFloat(gen);

        // 计算速度方向（向上扇形）
        float speed = speedDist(gen);
        sf::Vector2f velocity(
            std::sin(angle) * speed,
            -std::abs(std::cos(angle)) * speed // 主要向上
        );

        // 粒子尺寸
        float size = 10;
        sf::Vector2f particleSize(size, size);

        // 创建粒子（高阻力，短停留）
        float drag = 50.0f + randomFloat(gen) * 100.0f; // 高阻力让粒子快速停下
        // float drag = 0;
        float stayTime = 0.1f + randomFloat(gen) * 0.1f; // 短停留
        float fadeTime = 0.1f + randomFloat(gen) * 0.1f; // 较长淡出

        particles_.emplace_back(
            nowPositon,
            particleSize,
            velocity,
            Dust, // 白色灰尘
            drag,
            stayTime,
            fadeTime
        );
    }
}
