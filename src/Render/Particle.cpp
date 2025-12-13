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
    if (velocity_ != sf::Vector2f(0, 0))
        position_ += sf::Vector2f(10.0 * dir(particleRng), 10.0 * dir(particleRng)) *
                    (randomFloat(particleRng) * deltaTime); // 随机位移

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

    for (int i = 0; i < particleCount; i++) {
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

        float drag = 50.0f + randomFloat(gen) * 100.0f;
        float stayTime = 0.1f + randomFloat(gen) * 0.1f;
        float fadeTime = 0.1f + randomFloat(gen) * 0.1f;

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

void ParticleEmitter::emitWallJump(const sf::Vector2f &position, float number, float U, float D, float dir) {
    // 初始化随机器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(-180.0, 180.0); // 角度扩散
    std::uniform_real_distribution<float> speedDist(25.0f, 75.0f);

    for (int i = 0; i < number; i++) {
        // 随机角度（转换为弧度）
        float angle = angleDist(gen) * std::numbers::pi / 180.0f;

        // 随机位置
        sf::Vector2f nowPositon = position;
        nowPositon.x = nowPositon.x + 22.0 * randomFloat(gen) * dir;
        nowPositon.y = nowPositon.y + U + (D - U) * randomFloat(gen);

        // 计算速度方向
        float speed = speedDist(gen);
        sf::Vector2f velocity(
            std::abs(std::sin(angle)) * dir * speed,
            std::cos(angle) * speed
        );

        // 粒子尺寸
        float size = 10;
        sf::Vector2f particleSize(size, size);

        float drag = 50.0f + randomFloat(gen) * 100.0f;
        float stayTime = 0.1f + randomFloat(gen) * 0.1f;
        float fadeTime = 0.1f + randomFloat(gen) * 0.1f;

        particles_.emplace_back(
            nowPositon,
            particleSize,
            velocity,
            Dust,
            drag,
            stayTime,
            fadeTime
        );
    }
}

void ParticleEmitter::emitDashLine(const sf::Vector2f &position, const sf::Vector2f &dir) {
    // 初始化
    int halfLength = 8; // 轨迹线长度的一半（单位为粒子）
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < halfLength; ++i) {
        for (int sign = -1; sign <= 1; sign += 2) {
            // 初始化粒子状态
            float stayTime = 0.1 + i * 0.02;
            float fadeTime = 0.05 + randomFloat(gen) * 0.05;
            sf::Vector2f nowPositon = position + dir * (i * 10.0f * sign);
            float size = 10;
            sf::Vector2f particleSize(size, size);

            particles_.emplace_back(
                nowPositon,
                particleSize,
                sf::Vector2f(0, 0),
                Dust,
                0,
                stayTime,
                fadeTime
            );
        }
    }
}

void ParticleEmitter::emitDashBurst(const sf::Vector2f &position, float intensity) {
    // 根据强度决定粒子数量
    int particleCount = static_cast<int>(5 + intensity * 15);

    // 初始化随机
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(-180.0, 180.0); // 角度扩散
    std::uniform_real_distribution<float> speedDist(20.0f, 10.0f); // 速度基于强度

    for (int i = 0; i < particleCount; ++i) {
        // 随机角度（转换为弧度）
        float angle = angleDist(gen) * std::numbers::pi / 180.0f;

        // 随机位置
        auto randonMove = sf::Vector2f(std::sin(angle) * 38 * (randomFloat(gen) / 1.7 + 1),
                                       std::cos(angle) * 38 * (randomFloat(gen) / 1.7 + 1));
        sf::Vector2f nowPositon = position + randonMove;
        // 计算速度方向（向上扇形）
        float speed = speedDist(gen);
        sf::Vector2f velocity(std::sin(angle) * speed, std::cos(angle) * speed);

        // 粒子尺寸
        float size = 10;
        sf::Vector2f particleSize(size, size);

        float drag = 50.0 + randomFloat(gen) * 100.0;
        float stayTime = 0.05 + randomFloat(gen) * 0.01;
        float fadeTime = 0.01 + randomFloat(gen) * 0.03;

        particles_.emplace_back(
            nowPositon,
            particleSize,
            velocity,
            Dust,
            drag,
            stayTime,
            fadeTime
        );
    }
}

void ParticleEmitter::emitDashLaunch(const sf::Vector2f &position, const sf::Vector2f &dir) {
    // 初始化随机器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution sign(-1, 1);
    std::uniform_real_distribution<float> angleDist(-45, 45); // 角度扩散
    std::uniform_real_distribution<float> speedDist(150.0, 200.0);

    // 初始化粒子状态
    sf::Vector2f nowPositon = position + sf::Vector2f(20.0 * sign(gen), 20.0 * sign(gen)) *
                                                     (randomFloat(gen) * deltaTime);
    float stayTime = 0.8 + randomFloat(gen) * 0.5;
    float fadeTime = 0.1f + randomFloat(gen) * 0.1f;
    float drag = 10.0 + randomFloat(gen) * 10.0;
    float speed = speedDist(gen);
    float angle = angleDist(gen) * std::numbers::pi / 180.0f + std::atan2(dir.x, dir.y);
    float size = 10;

    sf::Vector2f particleSize(size, size);
    sf::Vector2f velocity(std::sin(angle) * speed, std::cos(angle) * speed);

    particles_.emplace_back(
            nowPositon,
            particleSize,
            velocity,
            DashResidue,
            drag,
            stayTime,
            fadeTime
        );
}


