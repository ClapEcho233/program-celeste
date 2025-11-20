//
// Created by ClapEcho233 on 2025/11/18.
//

#include "ViewRender.h"
#include <cmath>

ViewportAspectRenderer::ViewportAspectRenderer(float width, float height,
                                             const sf::Color& borderColor)
    : baseViewSize(width, height)
    , targetAspectRatio(width / height)
    , borderColor(borderColor)
{
    // 初始化游戏视图
    gameView.setSize(baseViewSize);
    gameView.setCenter({baseViewSize.x / 2.0f, baseViewSize.y / 2.0f});
}

void ViewportAspectRenderer::handleResize(const sf::RenderWindow& window) {
    handleResize(window.getSize());
}

void ViewportAspectRenderer::handleResize(const sf::Vector2u& windowSize) {
    float windowAspect = static_cast<float>(windowSize.x) / windowSize.y;

    if (windowAspect > targetAspectRatio) {
        // 窗口较宽，上下黑边
        float viewportHeight = 1.0f;
        float viewportWidth = viewportHeight * (targetAspectRatio / windowAspect);
        float viewportX = (1.0f - viewportWidth) * 0.5f;

        gameView.setViewport(sf::FloatRect({viewportX, 0.0f}, {viewportWidth, viewportHeight}));
    } else {
        // 窗口较高，左右黑边
        float viewportWidth = 1.0f;
        float viewportHeight = viewportWidth * (windowAspect / targetAspectRatio);
        float viewportY = (1.0f - viewportHeight) * 0.5f;

        gameView.setViewport(sf::FloatRect({0.0f, viewportY}, {viewportWidth, viewportHeight}));
    }
}

void ViewportAspectRenderer::applyView(sf::RenderWindow& window) const {
    window.setView(gameView);
}

sf::View& ViewportAspectRenderer::getView() {
    return gameView;
}

const sf::View& ViewportAspectRenderer::getView() const {
    return gameView;
}

void ViewportAspectRenderer::setView(const sf::View& view) {
    // 保持当前的视口设置
    sf::FloatRect currentViewport = gameView.getViewport();
    gameView = view;
    gameView.setViewport(currentViewport);
}

void ViewportAspectRenderer::resetView() {
    sf::FloatRect currentViewport = gameView.getViewport();
    gameView.setSize(baseViewSize);
    gameView.setCenter({baseViewSize.x / 2.0f, baseViewSize.y / 2.0f});
    gameView.setRotation(sf::degrees(0.0f));
    gameView.setViewport(currentViewport);
}

sf::Vector2f ViewportAspectRenderer::getBaseViewSize() const {
    return baseViewSize;
}

void ViewportAspectRenderer::setBaseViewSize(float width, float height) {
    baseViewSize.x = width;
    baseViewSize.y = height;
    targetAspectRatio = width / height;

    // 更新视图大小但保持当前中心点
    sf::Vector2f currentCenter = gameView.getCenter();
    gameView.setSize(baseViewSize);
    gameView.setCenter(currentCenter);
}

void ViewportAspectRenderer::setBaseViewSize(const sf::Vector2f& size) {
    setBaseViewSize(size.x, size.y);
}

float ViewportAspectRenderer::getAspectRatio() const {
    return targetAspectRatio;
}

void ViewportAspectRenderer::setBorderColor(const sf::Color& color) {
    borderColor = color;
}

sf::Color ViewportAspectRenderer::getBorderColor() const {
    return borderColor;
}

void ViewportAspectRenderer::setCameraCenter(float x, float y) {
    gameView.setCenter({x, y});
}

void ViewportAspectRenderer::setCameraCenter(const sf::Vector2f& center) {
    gameView.setCenter(center);
}

void ViewportAspectRenderer::moveCamera(float offsetX, float offsetY) {
    gameView.move({offsetX, offsetY});
}

void ViewportAspectRenderer::moveCamera(const sf::Vector2f& offset) {
    gameView.move(offset);
}

void ViewportAspectRenderer::zoomCamera(float factor) {
    gameView.zoom(factor);
}

void ViewportAspectRenderer::setCameraSize(float width, float height) {
    gameView.setSize({width, height});
}

void ViewportAspectRenderer::setCameraSize(const sf::Vector2f& size) {
    gameView.setSize(size);
}

void ViewportAspectRenderer::rotateCamera(float angle) {
    gameView.rotate(sf::degrees(angle));
}

bool ViewportAspectRenderer::isPointVisible(const sf::Vector2f& point) const {
    sf::FloatRect visibleArea = getVisibleArea();
    return visibleArea.contains(point);
}

sf::FloatRect ViewportAspectRenderer::getVisibleArea() const {
    sf::Vector2f center = gameView.getCenter();
    sf::Vector2f size = gameView.getSize();

    return sf::FloatRect(
        {center.x - size.x / 2.0f, center.y - size.y / 2.0f},
        {size.x, size.y}
    );
}
