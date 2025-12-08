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
    trailNumber(0),
    jumpGraceTimer(0),
    varJumpTimer(0),
    trailCreationTimer(0){
    // 初始化状态回调函数
    setupStateCallbacks();

    // 设置角色参数
    player_.setFillColor(sf::Color::Red);
    player_.setOutlineColor(sf::Color::White);
    player_.setOutlineThickness(5);
    player_.setSize({80, 110});
    player_.setOrigin({40, 55});
    player_.setPosition(nowLevel.getPosition());
}

Player::~Player() {}

void Player::update() {
    // 更新输入
    input_.update();
    moveX = input_.lr.value;

    // 更新历史状态
    wasOnGround = onGround;

    // 更新残影
    trailControl();
    trailEffectManager_.update();

    // 执行状态更新
    stateMachine_.update();

    // 应用移动
    moveH(speed.x * deltaTime);
    moveV(speed.y * deltaTime);
    // std::cout << speed.x << "," << speed.y << std::endl;
    // std::cout << trailNumber << std::endl;
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
        [this](void* priv) { return dashCoroutine(priv); },
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
    if (input_.dash.pressed && dashes)
        return PlayerState::Dash;

    // 检查攀爬

    // 水平移动
    moveHControl();

    // 墙面滑动
    wallSlideCheck();

    // 应用重力
    if (! onGround)
        applyGravity();

    // 跳跃处理
    jumpControl();

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
    // 消费状态
    dashes -= 1;
    input_.dash.pressed = false;
    sf::sleep(sf::seconds(0.05));

    // 初始化
    bool up = input_.up.getOriginalInput();
    bool down = input_.down.getOriginalInput();
    bool left = input_.lr.left.getOriginalInput();
    bool right = input_.lr.right.getOriginalInput();
    if (left)
        lastAim.x = -1;
    else if (right)
        lastAim.x = 1;
    else
        lastAim.x = 0;
    if (up)
        lastAim.y = -1;
    else if (down)
        lastAim.y = 1;
    else
        lastAim.y = 0;
    if (lastAim.x == 0 && lastAim.y == 0) lastAim = {1, 0};
    lastAim = lastAim.normalized(); // 标准化向量
    trailNumber = TrailNumber; // 设定残影数量
    trailCreationTimer = 0; // 初始化计时器
    beforeDashSpeed = speed;
    if (lastAim.y < 0)
        onGround = false;

    return PlayerState::Dash;
}

PlayerState Player::dashUpdate() {
    if (messages_.isDone)
        return PlayerState::Normal;
    return PlayerState::Dash;
}

void Player::dashEnd() {
}

int Player::dashCoroutine(void *priv) {
    // 初始化协程相关
    auto *co = copp::this_coroutine::get_coroutine();
    messages_.isDone = false;
    messages_.waitTime = 0;
    co -> yield(); // 等待一帧

    // 应用冲刺速度
    sf::Vector2f dir = lastAim;
    sf::Vector2f newSpeed = dir * DashSpeed;
    if (std::copysign(1.0, beforeDashSpeed.x) == std::copysign(1.0, newSpeed.x) &&
        std::abs(beforeDashSpeed.x) > std::abs(newSpeed.x))
        newSpeed.x = beforeDashSpeed.x;

    speed = newSpeed;

    // 0.15s 冲刺阶段
    messages_.waitTime = 0.15;
    co -> yield();

    // 冲刺结束处理
    if (dir.y <= 0)
        speed = dir * EndDashSpeed;
    if (dir.y < 0)
        speed.y *= EndDashUpMult;

    messages_.waitTime = 0;
    messages_.isDone = true;
    co -> yield();

    return 0;
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

void Player::jumpControl() {
    if (onGround) { // 跳跃缓冲
       jumpGraceTimer = JumpGraceTime;
    } else if (jumpGraceTimer > 0) {
        jumpGraceTimer -= deltaTime;
    }
    if (input_.jump.buffered) { // 跳跃瞬间
        if (jumpGraceTimer > 0) { // 在宽限时间内
            jump();
        }
        return ;
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
}

void Player::wallSlideCheck() {
    maxFall = MaxFall;
    if (input_.down.check == false && speed.y >= 0 && wallSlideTimer > 0 &&
        checkNextCollide(sf::Vector2f(1, 0) * static_cast<float>(moveX)).empty() == false) {
        wallSlideDir = moveX;
    }
    if (wallSlideDir != 0) {
        maxFall = std::lerp(MaxFall, WallSlideStartMax, wallSlideTimer / WallSlideTime); // 插值，使最大下落速度缓慢提升
    }
    if (wallSlideTimer > 0) {
        wallSlideTimer = approach(wallSlideTimer, 0, deltaTime);
        wallSlideDir = 0;
    }
    // std::cout << wallSlideTimer << std::endl;
}

void Player::trailControl() {
    if (! trailNumber) return ;

    if (trailCreationTimer > 0)
        trailCreationTimer -= deltaTime;
    else {
        trailCreationTimer = TrailCreationTime + (TrailNumber - trailNumber) * 0.02;
        trailNumber -= 1;
        trailEffectManager_.create(player_.getPosition(), player_.getSize(), Used);
    }
}

void Player::groundCheck() {
    if (speed.y >= 0) { // 只有下落或静止时检测
        if (checkNextCollide({0, 1}).empty()) {
            onGround = false;
        } else {
            onGround = true;
            dashes = MaxDashes;
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
    if (! onGround)
        return ;
    // 消费跳跃缓冲
    input_.jump.consumeBuffer();

    // 设置角色状态
    onGround = false;

    // 重置计时器
    varJumpTimer = VarJumpTime;
    wallSlideTimer = WallSlideTime;
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
            // 调用碰撞回调
            onCollideH(data);
            break;
        }
    }
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
            // 调用碰撞回调
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
            handleRightWallCollision(data);
        } else {
            handleLeftWallCollision(data);
        }

        speed.x = 0;
    } else {
        moveH(data.Remaining.x);
    }
}

void Player::onCollideV(const CollisionData& data) {
    if (data.Hit.getType() == EType::Platform) { // 如果为平台（不可穿过），就停止
        int hitDirection = std::copysign(1.0, data.Direction.y);

        // 向上墙角修正
        if (upwardCornerCorrection()) {
            moveV(data.Remaining.y);
            return ;
        }

        // 处理不同方向碰撞
        if (hitDirection > 0) {
            handleGroundCollision(data);
        } else {
            handleCeilingCollision(data);
        }

        speed.y = 0;
    } else {
        moveV(data.Remaining.y);
    }
}

void Player::handleLeftWallCollision(const CollisionData &data) {
}

void Player::handleRightWallCollision(const CollisionData &data) {
}

void Player::handleGroundCollision(const CollisionData &data) {
    // 设置状态
    // onGround = true;
    // dashes = MaxDashes;
    wallSlideTimer = WallSlideTime;
}

void Player::handleCeilingCollision(const CollisionData &data) {
    // 取消可变跳跃
    if (varJumpTimer) varJumpTimer = 0;
}

bool Player::upwardCornerCorrection() {
    if (speed.y >= 0) return false; // 处理向上移动

    auto check = [this](float sign) -> bool {
        for (int i = 1; i <= CornerCorrection; i ++) {
            auto collision = checkNextCollide({static_cast<float>(i) * sign, -1});
            if (collision.empty()) {
                moveH(i * sign);
                return true;
            }
        }
        return false;
    };

    // 根据水平速度方向分类
    if (speed.x <= 0) {
        if (check(-1)) return true;
    }
    if (speed.x >= 0) {
        if (check(1)) return true;
    }
    return false;
}


void Player::applyGravity() {
    if (! onGround) {
        // 调整重力系数，使长按跳跃时滞空时间更久
        float mult = (std::abs(speed.y) < HalfGravThreshold && input_.jump.check) ? 0.5 : 1.0;
        speed.y = approach(speed.y, maxFall, Gravity * mult * deltaTime);
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
    if (dashes == 1)
        player_.setFillColor(Normal);
    else
        player_.setFillColor(Used);
    trailEffectManager_.render(window);
    window.draw(player_);
}