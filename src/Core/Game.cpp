//
// Created by ClapEcho233 on 2025/11/17.
//

#include "Game.h"
#include <algorithm>
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
    shakeDeltaTimer_(0),
    transitioning_(false),
    transitionHoldTimer_(0),
    transitionBlendTimer_(0),
    pendingLevelId_(0),
    levelSwapped_(false),
    frozenCameraPos_(0, 0),
    previousLevelId_(0),
    transitionDir_(0, 0),
    currentTransitionArea_({0, 0}, {0, 0}),
    preTransitionPlayerPos_(0, 0),
    preTransitionSpeed_(0, 0)
{
    window_.setFramerateLimit(120);
    renderer_.handleResize(window_);
    renderer_.zoomCamera(1);

    player_ = new Player(levelManager_, [this](sf::Vector2f dir) { screenShake(dir); });

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
    if (transitioning_) {
        updateLevelTransition();
        return;
    }

    player_->update();
    shakeUpdate();
    updateCamera();
    checkLevelTransition();
}

void Game::render() {
    window_.clear(renderer_.getBorderColor());
    renderer_.applyView(window_);

    line_.render(window_);
    if (transitioning_) {
        int extra = levelSwapped_ ? previousLevelId_ : pendingLevelId_;
        levelManager_.render(window_, extra);
    } else {
        levelManager_.render(window_);
    }
    player_->render(window_);

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

void Game::checkLevelTransition() {
    const Level& level = levelManager_.getLevel();
    sf::FloatRect playerBounds = player_->getBounds();

    for (const auto& transition : level.getTransitions()) {
        if (transition.area.findIntersection(playerBounds)) {
            startLevelTransition(transition);
            break;
        }
    }
}

void Game::startLevelTransition(const Level::Transition& transition) {
    int nextLevelId = transition.nextLevelId;
    if (transitioning_ || nextLevelId == levelManager_.getLevelId())
        return;
    if (nextLevelId < 0 || nextLevelId >= levelManager_.getLevelCount())
        return;

    transitioning_ = true;
    previousLevelId_ = levelManager_.getLevelId();
    pendingLevelId_ = nextLevelId;
    transitionHoldTimer_ = TransitionHoldTime;
    transitionBlendTimer_ = TransitionBlendTime;
    levelSwapped_ = false;
    frozenCameraPos_ = cameraPos_;
    shakeNumber_ = 0;
    shakeOffset_ = {0, 0};
    preTransitionSpeed_ = player_->getSpeed();
    player_->stopMovement();
    currentTransitionArea_ = transition.area;
    transitionDir_ = determineTransitionDir(levelManager_.getLevel(), transition);
    preTransitionPlayerPos_ = player_->getPosition();
}

void Game::updateLevelTransition() {
    if (transitionHoldTimer_ > 0) {
        transitionHoldTimer_ -= deltaTime_;
        renderer_.setCameraCenter(frozenCameraPos_);
        return;
    }

    if (!levelSwapped_) {
        levelManager_.setLevelId(pendingLevelId_);
        levelSwapped_ = true;
        placePlayerInNextLevel();
    }

    updateCamera(false);
    transitionBlendTimer_ -= deltaTime_;
    if (transitionBlendTimer_ <= 0) {
        transitioning_ = false;
    }
}

sf::Vector2f Game::determineTransitionDir(const Level& level, const Level::Transition& t) const {
    sf::FloatRect bounds = level.getBounds();
    float leftDist = std::abs(t.area.position.x - bounds.position.x);
    float rightDist = std::abs(bounds.position.x + bounds.size.x - (t.area.position.x + t.area.size.x));
    float topDist = std::abs(t.area.position.y - bounds.position.y);
    float bottomDist = std::abs(bounds.position.y + bounds.size.y - (t.area.position.y + t.area.size.y));

    float minDist = std::min(std::min(leftDist, rightDist), std::min(topDist, bottomDist));
    if (minDist == leftDist) return {-1.f, 0.f};
    if (minDist == rightDist) return {1.f, 0.f};
    if (minDist == topDist) return {0.f, -1.f};
    return {0.f, 1.f};
}

void Game::placePlayerInNextLevel() {
    const Level& nextLevel = levelManager_.getLevel();
    sf::FloatRect nextBounds = nextLevel.getBounds();
    sf::Vector2f playerSize = player_->getBounds().size;
    float halfW = playerSize.x * 0.5f;
    float halfH = playerSize.y * 0.5f;
    const float margin = 12.0f; // 保证出生点离开触发区域且位移更小

    // 找到下一关中指向上一关的过渡区域，方便贴合边界
    const auto& transitions = nextLevel.getTransitions();
    const Level::Transition* backTransition = nullptr;
    for (const auto& t : transitions) {
        if (t.nextLevelId == previousLevelId_) {
            backTransition = &t;
            break;
        }
    }

    // 基于方向选择一个合适的落点（在边界内侧），保持另一轴的感知不变
    auto clampAxis = [](float v, float min, float max) {
        return std::max(min, std::min(max, v));
    };

    sf::Vector2f target = preTransitionPlayerPos_;
    auto anchor = backTransition ? backTransition->area : currentTransitionArea_;

    if (transitionDir_.x > 0) { // 从左往右出关，落点放在下一关左侧边界内侧
        float x = (backTransition ? anchor.position.x + anchor.size.x : nextBounds.position.x) + halfW + margin;
        float y = clampAxis(preTransitionPlayerPos_.y, nextBounds.position.y + halfH, nextBounds.position.y + nextBounds.size.y - halfH);
        target = {x, y};
    } else if (transitionDir_.x < 0) { // 从右往左
        float x = (backTransition ? anchor.position.x : nextBounds.position.x + nextBounds.size.x) - halfW - margin;
        float y = clampAxis(preTransitionPlayerPos_.y, nextBounds.position.y + halfH, nextBounds.position.y + nextBounds.size.y - halfH);
        target = {x, y};
    } else if (transitionDir_.y > 0) { // 从上往下
        float y = (backTransition ? anchor.position.y + anchor.size.y : nextBounds.position.y) + halfH + margin;
        float x = clampAxis(preTransitionPlayerPos_.x, nextBounds.position.x + halfW, nextBounds.position.x + nextBounds.size.x - halfW);
        target = {x, y};
    } else if (transitionDir_.y < 0) { // 从下往上
        float y = (backTransition ? anchor.position.y : nextBounds.position.y + nextBounds.size.y) - halfH - margin;
        float x = clampAxis(preTransitionPlayerPos_.x, nextBounds.position.x + halfW, nextBounds.position.x + nextBounds.size.x - halfW);
        target = {x, y};
    }

    player_->setPosition(target);
    player_->setSpeed(preTransitionSpeed_);
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
