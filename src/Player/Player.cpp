//
// Created by ClapEcho233 on 2025/11/18.
//

#include "Player.h"

Player::Player(Level nowLevel) :
    nowlevel_(nowLevel),
    moveX(0),
    onGround(false),
    wasOnGround(false),
    dashes(1),
    jumpGraceTimer(0),
    varJumpTimer(0) {
    // 初始化状态回调函数
    setupStateCallbacks();

    // 设置角色参数
    player_.setFillColor(sf::Color::Yellow);
    player_.setOutlineColor(sf::Color::White);
    player_.setOutlineThickness(1);
    player_.setSize({80, 110});
    player_.setPosition(nowLevel.getPosition());
}

Player::~Player() {}

void Player::update() {
    // 更新输入
    input_.update();
    moveX = input_.lr.value;

    // 更新历史状态
    wasOnGround = onGround;

    // 执行状态更新
    stateMachine_.update();

    // std::cout << speed.x << "," << speed.y << std::endl;
}

void Player::setupStateCallbacks() {
    // Normal 状态
    stateMachine_.setCallbacks(
        PlayerState::Normal,
        [this] { return normalUpdate(); },
        nullptr,  // 没有协程
        [this] { normalBegin(); },
        [this] { normalEnd(); }
    );

    // Climb 状态
    stateMachine_.setCallbacks(
        PlayerState::Climb,
        [this] { return climbUpdate(); },
        nullptr,
        [this] { climbBegin(); },
        [this] { climbEnd(); }
    );

    // Dash 状态
    stateMachine_.setCallbacks(
        PlayerState::Dash,
        [this] { return dashUpdate(); },
        [this] { return dashCoroutine(); },
        [this] { dashBegin(); },
        [this] { dashEnd(); }
    );
}

PlayerState Player::normalBegin() {
    std::cout << "Enter Normal State" << std::endl;
    return PlayerState::Normal;
}

PlayerState Player::normalUpdate() {
    // 地面检测
    if (speed.y >= 0)
        groundCheck();

    // 检查冲刺

    // 检查攀爬

    // 水平移动
    moveHControl();

    // 应用重力
    if (! onGround)
        applyGravity();

    // 跳跃处理
    if (onGround) { // 跳跃缓冲
        jumpGraceTimer = JumpGraceTime;
    } else if (jumpGraceTimer > 0) {
        jumpGraceTimer -= deltaTime;
    }
    if (input_.jump.buffered && onGround) { // 跳跃瞬间
        if (jumpGraceTimer > 0) { // 在宽限时间内
            jump();
        }
        // return PlayerState::Normal;
    }
    if (varJumpTimer > 0) // 可变跳跃处理
        varJumpTimer -= deltaTime;
    if (varJumpTimer > 0) {
        if (input_.jump.check) {
            speed.y = std::min (speed.y, varJumpSpeed);
        } else {
            varJumpTimer = 0;
        }
    }

    // 应用移动
    moveH(speed.x * deltaTime);
    moveV(speed.y * deltaTime);

    return PlayerState::Normal;
}

void Player::normalEnd() {
    std::cout << "Exit Normal State" << std::endl;
}

PlayerState Player::climbBegin() {
    return PlayerState::Climb;
}

PlayerState Player::climbUpdate() {
    return PlayerState::Climb;
}

void Player::climbEnd() {
}

PlayerState Player::dashBegin() {
    return PlayerState::Dash;
}

PlayerState Player::dashUpdate() {
    return PlayerState::Dash;
}

void Player::dashEnd() {
}

CoroutineResult Player::dashCoroutine() {
    return {false, 0.0};
}

void Player::moveHControl() {
    // std::cout << "Enter Move H Control" << std::endl;
    float mult = onGround ? 1.0 : AirMult;
    if (std::abs(speed.x) > MaxRun && std::copysign(1.0, speed.x) == moveX) {
        // 超速情况：减速到最大速度
        speed.x = approach(speed.x, MaxRun * moveX, RunReduce * mult * deltaTime);
    } else {
        // 正常情况：加速到目标速度
        speed.x = approach(speed.x, MaxRun * moveX, RunAccel * mult * deltaTime);
        // std::cout << speed.x << std::endl;
    }
}

void Player::groundCheck() {
    if (speed.y >= 0) { // 只有下落或静止时检测
        if (checkNextCollide({0, 1}).empty()) {
            onGround = false;
        }
    }
}

std::vector<Entity> Player::checkNextCollide(sf::Vector2f unit) {
    // 获取角色位置和大小
    auto position = player_.getGlobalBounds().position;
    auto size = player_.getGlobalBounds().size;

    // 检测碰撞
    return nowlevel_.collision(sf::FloatRect(position + unit, size));
}

void Player::jump() {
    // 消费跳跃缓冲
    input_.jump.consumeBuffer();

    // 设置角色状态
    onGround = false;

    // 重置计时器
    varJumpTimer = VarJumpTime;
    jumpGraceTimer = 0;

    // 应用跳跃速度
    speed.x += JumpHBoost * moveX;
    speed.y = JumpSpeed;

    // 记录可变跳跃基准速度
    varJumpSpeed = speed.y;
}

bool Player::moveH(float h) {
    int move = static_cast<int>(std::round(h));
    if (move == 0)
        return false;
    return moveHExact(move);
}

bool Player::moveV(float v) {
    int move = static_cast<int>(std::round(v));
    if (move == 0)
        return false;
    return moveVExact(move);
}

bool Player::moveHExact(int h) {
    if (h == 0)
        return false;

    float sign = std::copysign(1.0, h);
    int moved = 0;

    // 逐像素移动检测碰撞
    while (moved < std::abs(h)) {
        auto collision = checkNextCollide({sign, 0});
        if (collision.empty()) {
            // 没有碰撞
            player_.move({sign, 0});
            moved ++;
        } else {
            CollisionData data{
                .Direction = sf::Vector2f(1, 0) * sign,
                .Moved = sf::Vector2f(1, 0) * static_cast<float>(moved),
                .Hit = collision.front(),
                .Remaining = sf::Vector2f(1, 0) * (h - sign * moved)
            };
            onCollideH(data);
            std::cout << "emmm" << std::endl;
            break;
        }
    }
    std::cout << moved << std::endl;
    return moved > 0;
}

bool Player::moveVExact(int v) {
    if (v == 0)
        return false;

    float sign = std::copysign(1.0, v);
    int moved = 0;

    // 逐像素移动检测碰撞
    while (moved < std::abs(v)) {
        auto collision = checkNextCollide({0, sign});
        if (collision.empty()) {
            // 没有碰撞
            player_.move({0, sign});
            moved ++;
        } else {
            CollisionData data{
                .Direction = sf::Vector2f(0, 1) * sign,
                .Moved = sf::Vector2f(0, 1) * static_cast<float>(moved),
                .Hit = collision.front(),
                .Remaining = sf::Vector2f(0, 1) * (v - sign * moved)
            };
            onCollideV(data);
            break;
        }
    }

    return moved > 0;
}

void Player::onCollideH(const CollisionData& data) {
    if (data.Hit.getType() == EType::Platform) { // 如果为平台（不可穿过），就停止
        int hitDirection = std::copysign(1.0, data.Direction.x);

        // 处理不同方向碰撞
        if (hitDirection > 0) {
            HandleRightWallCollision(data);
        } else {
            HandleLeftWallCollision(data);
        }

        speed.x = 0;
    } else {
        moveH(data.Remaining.x);
    }
}

void Player::onCollideV(const CollisionData& data) {
    if (data.Hit.getType() == EType::Platform) { // 如果为平台（不可穿过），就停止
        int hitDirection = std::copysign(1.0, data.Direction.y);

        // 处理不同方向碰撞
        if (hitDirection > 0) {
            HandleGroundCollision(data);
        } else {
            HandleCeilingCollision(data);
        }

        speed.y = 0;
    } else {
        moveV(data.Remaining.y);
    }
}

void Player::HandleLeftWallCollision(const CollisionData &data) {
}

void Player::HandleRightWallCollision(const CollisionData &data) {
}

void Player::HandleGroundCollision(const CollisionData &data) {
    onGround = true;
}

void Player::HandleCeilingCollision(const CollisionData &data) {
    if (varJumpTimer) varJumpTimer = 0;
}


void Player::applyGravity() {
    if (! onGround) {
        // 调整重力系数，使长按跳跃时滞空时间更久
        float mult = (std::abs(speed.y) < HalfGravThreshold && input_.jump.check) ? 0.5 : 1.0;
        speed.y = approach(speed.y, MaxFall, Gravity * mult * deltaTime);
    }
}


float Player::approach(const float &current, const float &target, const float &step) {
    if (current < target) {
        return std::min(current + step, target);
    } else {
        return std::max(current - step, target);
    }
}

void Player::render(sf::RenderWindow &window) {
    window.draw(player_);
}