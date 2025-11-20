//
// Created by ClapEcho233 on 2025/11/17.
//

#include "Game.h"

Game::Game() :
    window_(sf::VideoMode({1920, 1080}), "Celeste"),
    renderer_(320, 180, sf::Color(0, 0, 0)),
    line_(64, 36) {
    window_.setFramerateLimit(120);
    renderer_.handleResize(window_);
    renderer_.setCameraCenter(160, 90);
    renderer_.zoomCamera(0.9); // 优化震动观感
    // std::cout << renderer_.isPointVisible({0,0});
}

bool Game::isRunning() const {
    return window_.isOpen();
}

void Game::processEvent() {
    window_.handleEvents([this](const auto &event) { this->handleEvent(event); });
}

void Game::update() {

}

void Game::render() {

    sf::CircleShape player(10);
    player.setFillColor(sf::Color::Red);
    player.setPosition({150, 80});


    // 渲染
    window_.clear(renderer_.getBorderColor());
    // 应用游戏视图并渲染游戏世界
    renderer_.applyView(window_);

    window_.draw(player);
    line_.render(window_);

    window_.display();
}

void Game::handleEvent(const sf::Event::Closed &) {
    window_.close();
    std::cout << "Celeste closed" << std::endl;
}

void Game::handleEvent(const sf::Event::Resized &resized) {
    std::cout << resized.size.x << " " << resized.size.y << std::endl;
    renderer_.handleResize(window_);
}






