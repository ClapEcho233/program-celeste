//
// Created by ClapEcho233 on 2025/11/17.
//

#include "Game.h"

Game::Game() :
    window_(sf::VideoMode({1920, 1080}), "Celeste"),
    renderer_(3200, 1800, sf::Color(0, 0, 0)),
    line_(64, 36),
    deltaTime_(0),
    shakeSign_(0),
    shakeNumber_(0),
    shakeDeltaTimer_(0){
    window_.setFramerateLimit(120);
    renderer_.handleResize(window_);
    renderer_.setCameraCenter(1600, 900);
    renderer_.zoomCamera(0.96); // 优化震动观感
    // std::cout << renderer_.isPointVisible({0,0});

    player_ = new Player(levelManager_.getLevel(), [this](sf::Vector2f dir) {screenShake(dir);});
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
    player_ -> update(); // 更新角色
    shakeUpdate(); // 更新震动事件
}

void Game::render() {
    // 渲染
    window_.clear(renderer_.getBorderColor());
    // 应用游戏视图并渲染游戏世界
    renderer_.applyView(window_);

    line_.render(window_);

    // 渲染对象
    player_ -> render(window_);
    levelManager_.render(window_);

    window_.display();
}

float Game::getDeltaTime() const {
    return deltaTime_;
}

void Game::screenShake(sf::Vector2f dir) {
    // 初始化
    shakeNumber_ = ShakeNumber;
    shakeDeltaTimer_ = 0;
    shakeDir_ = dir;
    shakeSign_ = -1;
    renderer_.moveCamera(shakeDir_ * ShakePixel);
}

void Game::shakeUpdate() {
    if (!shakeNumber_) {
        renderer_.setCameraCenter(1600, 900);
        return ;
    }
    if (shakeDeltaTimer_ > 0)
        shakeDeltaTimer_ -= deltaTime_;
    else {
        shakeNumber_ -= 1;
        shakeDeltaTimer_ = ShakeDeltaTime;
        renderer_.moveCamera(static_cast<float>(2.0 * shakeSign_ * ShakePixel) * shakeDir_);
        shakeSign_ = -shakeSign_;
    }
}


void Game::handleEvent(const sf::Event::Closed &) {
    window_.close();
    std::cout << "Celeste closed" << std::endl;
}

void Game::handleEvent(const sf::Event::Resized &resized) {
    std::cout << resized.size.x << " " << resized.size.y << std::endl;
    renderer_.handleResize(window_);
}






