//
// Created by ClapEcho233 on 2025/11/17.
//

#include "Game.h"

Game::Game() :
    window_(sf::VideoMode({1920, 1080}), "Celeste"),
    renderer_(3200, 1800, sf::Color(0, 0, 0)),
    line_(64, 36),
    deltaTime_(0){
    window_.setFramerateLimit(120);
    renderer_.handleResize(window_);
    renderer_.setCameraCenter(1600, 900);
    renderer_.zoomCamera(0.96); // 优化震动观感
    // std::cout << renderer_.isPointVisible({0,0});

    player_ = new Player(levelManager_.getLevel());
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
    player_ -> update();
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


void Game::handleEvent(const sf::Event::Closed &) {
    window_.close();
    std::cout << "Celeste closed" << std::endl;
}

void Game::handleEvent(const sf::Event::Resized &resized) {
    std::cout << resized.size.x << " " << resized.size.y << std::endl;
    renderer_.handleResize(window_);
}






