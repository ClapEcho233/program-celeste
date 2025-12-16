//
// Created by ClapEcho233 on 2025/11/17.
//

#include "Game.h"
#include <cmath>

Game::Game() :
    window_(sf::VideoMode({1920, 1080}), "Celeste"),
    renderer_(3200, 1800, sf::Color(0, 0, 0)),
    line_(64, 36),
    deltaTime_(0),
    cameraPos_(0, 0),
    cameraTarget_(0, 0),
    shakeOffset_(0, 0),
    followPlayerX_(true),
    followPlayerY_(true),
    cameraFollowSpeed_(8.0f),
    shakeDir_(0, 0),
    shakeSign_(0),
    shakeNumber_(0),
    shakeDeltaTimer_(0)
{
    window_.setFramerateLimit(120);
    renderer_.handleResize(window_);
    renderer_.zoomCamera(1);

    player_ = new Player(levelManager_.getLevel(), [this](sf::Vector2f dir) { screenShake(dir); });

    cameraPos_ = player_->getPosition();
    cameraTarget_ = cameraPos_;
    updateCamera(true);
}

bool Game::isRunning() const {
    return window_.isOpen();
}

void Game::initialize() {
    deltaTime_ = clock_.restart().asSeconds();
}

void Game::processEvent() {
    window_.handleEvents([this](const auto &event) { this->handleEvent(event); });
}

void Game::update() {
    player_->update();
    shakeUpdate();
    updateCamera();
}

void Game::render() {
    window_.clear(renderer_.getBorderColor());
    renderer_.applyView(window_);

    line_.render(window_);
    player_->render(window_);
    levelManager_.render(window_);

    window_.display();
}

float Game::getDeltaTime() const {
    return deltaTime_;
}

void Game::screenShake(sf::Vector2f dir) {
    shakeNumber_ = ShakeNumber;
    shakeDeltaTimer_ = 0;
    shakeDir_ = dir;
    shakeSign_ = -1;
    shakeOffset_ = shakeDir_ * ShakePixel;
    renderer_.setCameraCenter(cameraPos_ + shakeOffset_);
}

void Game::shakeUpdate() {
    if (!shakeNumber_) {
        shakeOffset_ = {0, 0};
        return;
    }

    if (shakeDeltaTimer_ > 0) {
        shakeDeltaTimer_ -= deltaTime_;
    } else {
        shakeNumber_ -= 1;
        shakeDeltaTimer_ = ShakeDeltaTime;
        shakeOffset_ += static_cast<float>(2.0 * shakeSign_ * ShakePixel) * shakeDir_;
        shakeSign_ = -shakeSign_;
    }
}

void Game::updateCamera(bool instant) {
    const Level& level = levelManager_.getLevel();
    sf::Vector2f desired = cameraPos_;
    sf::Vector2f playerPos = player_->getPosition();

    // 镜头目标先对齐玩家（可选择是否锁定 X/Y）
    if (followPlayerX_)
        desired.x = playerPos.x;
    if (followPlayerY_)
        desired.y = playerPos.y;

    const sf::View& view = renderer_.getView();
    sf::Vector2f halfSize = view.getSize() * 0.5f;

    auto clampAxis = [](float value, float min, float max) {
        if (min > max)
            return (min + max) * 0.5f;
        if (value < min)
            return min;
        if (value > max)
            return max;
        return value;
    };

    sf::FloatRect bounds = level.getBounds();
    float minX = bounds.position.x + halfSize.x;
    float maxX = bounds.position.x + bounds.size.x - halfSize.x;
    float minY = bounds.position.y + halfSize.y;
    float maxY = bounds.position.y + bounds.size.y - halfSize.y;

    // 在边界内夹紧，避免镜头超出地图
    desired.x = clampAxis(desired.x, minX, maxX);
    desired.y = clampAxis(desired.y, minY, maxY);
    cameraTarget_ = desired;

    if (instant) {
        cameraPos_ = desired;
    } else {
        // 指数插值做平滑跟随，避免硬切
        float factor = 1.0f - std::exp(-cameraFollowSpeed_ * deltaTime_);
        cameraPos_.x += (desired.x - cameraPos_.x) * factor;
        cameraPos_.y += (desired.y - cameraPos_.y) * factor;
    }

    // 叠加震动偏移，保持等比例视口
    renderer_.setCameraCenter(cameraPos_ + shakeOffset_);
}

void Game::setCameraFollow(bool followX, bool followY) {
    followPlayerX_ = followX;
    followPlayerY_ = followY;
    updateCamera(true);
}

void Game::handleEvent(const sf::Event::Closed &) {
    window_.close();
    std::cout << "Celeste closed" << std::endl;
}

void Game::handleEvent(const sf::Event::Resized &resized) {
    std::cout << resized.size.x << " " << resized.size.y << std::endl;
    renderer_.handleResize(window_);
    updateCamera(true);
}