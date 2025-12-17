//
// Created by ClapEcho233 on 2025/11/18.
//

#include "Player.h"

Player::Player(Level nowLevel, std::function<void(sf::Vector2f)> shake) :
    nowlevel_(nowLevel){
    // 初始化状态回调函数
    setupStateCallbacks();

    // 设置角色参数
    player_.setFillColor(sf::Color::Red);
    player_.setOutlineColor(sf::Color::White);
    player_.setOutlineThickness(5);
    player_.setSize({80, 110});
    player_.setOrigin({40, 55});
    player_.setPosition(nowLevel.getPosition());

    jumpGraceTimer = 0;
    varJumpTimer = 0;
    wallSlideTimer = 0;
    trailCreationTimer = 0;
    forceMoveXTimer = 0;
    climbNoMoveTimer = 0;
    wallBoostTimer = 0;
    climbJumpProtectionTimer = 0;
    dashRefillCooldownTimer = 0;
    wallSlideDir = 0;

    // 设置回调
    shakeCallback = shake;
}

Player::~Player() {}

void Player::update() {
    // 更新输入
    input_.update();
    moveX = input_.lr.value;
    if (forceMoveXTimer > 0) {
        forceMoveXTimer -= deltaTime;
        moveX = forceMoveX;
    }

    // 更新计时器
    updateDashAttackTimer();

    // 更新历史状态
    wasOnGround = onGround;

    // 更新残影
    trailControl();
    trailEffectManager_.update();
    // 更新粒子
    particleEmitter_.update();
    // 更新动画
    animationComponent_.update();

    // 执行状态更新
    stateMachine_.update();

    // 应用移动
    moveH(speed.x * deltaTime);
    moveV(speed.y * deltaTime);
    // std::cout << speed.x << "," << speed.y << std::endl;
    // std::cout << varJumpTimer << std::endl;
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
    dashRefillCooldownTimer = 0;
    jumpGraceTimer = 0;
    std::cout << "Enter Normal State" << std::endl;
    return PlayerState::Normal;
}

PlayerState Player::normalUpdate() {
    // 地面检测
    groundCheck();

    // 攀爬跳保护计时
    if (climbJumpProtectionTimer > 0) {
        climbJumpProtectionTimer -= deltaTime;
    }

    // 朝向检测
    if (moveX)
        facing = moveX;

    // 检查冲刺
    if (input_.dash.pressed && dashes)
        return PlayerState::Dash;

    // 检查攀爬
    if (input_.climb.check && climbCheck(facing)
        && climbJumpProtectionTimer <= 0)
        return PlayerState::Climb;

    moveHControl(); // 水平移动
    wallSlideCheck(); // 墙面滑动
    wallBoostCheck(); // 墙面助推

    // 跳跃处理
    jumpControl();

    // 应用重力
    applyGravity();

    return PlayerState::Normal;
}

void Player::normalEnd() {
    std::cout << "Exit Normal State" << std::endl;
}

PlayerState Player::climbBegin() {
    // 初始化
    speed.x = 0;
    speed.y *= ClimbGrabYMult;
    wallSlideTimer = WallSlideTime;
    climbNoMoveTimer = ClimbNoMoveTime;
    wallBoostTimer = 0;

    // 对齐至墙面
    for (int i = 0; i < ClimbCheckDist; i++) {
        if (checkNextCollide({facing, 0}).empty())
            player_.move({facing, 0});
        else
            break;
    }
    return PlayerState::Climb;
}

PlayerState Player::climbUpdate() {
    // 更新状态
    groundCheck();
    if (climbNoMoveTimer > 0)
        climbNoMoveTimer -= deltaTime;
    if (onGround)
        stamina = ClimbMaxStamina;

    // 跳跃检测
    if (input_.jump.pressed) {
        if (moveX == -static_cast<int>(facing))
            wallJump(-static_cast<int>(facing));
        else
            climbJump();
        return PlayerState::Normal;
    }

    // 冲刺检测
    if (input_.dash.pressed && dashes)
        return PlayerState::Dash;

    // 松开抓取键检测
    if (! input_.climb.check)
        return PlayerState::Normal;

    // 墙面丢失检测
    if (checkNextCollide({facing, 0}).empty()) {
        if (speed.y < 0)
            climbHop();
        return PlayerState::Normal;
    }

    handleNormalClimbing(); // 执行正常攀爬行为
    handleStaminaConsumption(); // 执行体力消耗和状态管理

    // 体力耗尽
    if (stamina <= 0)
        return PlayerState::Normal;

    return PlayerState::Climb;
}

void Player::climbEnd() {
}

PlayerState Player::dashBegin() {
    std::cout << "Enter Dash State" << std::endl;
    // 消费状态
    dashes -= 1;
    input_.dash.consumePress();
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
    if (lastAim.x == 0 && lastAim.y == 0) lastAim = {facing, 0};
    lastAim = lastAim.normalized(); // 标准化向量
    trailNumber = TrailNumber; // 设定残影数量
    trailCreationTimer = 0; // 初始化计时器
    dashAttackTimer = DashAttackTime;
    dashRefillCooldownTimer = DashRefillCooldownTime;
    beforeDashSpeed = speed;
    if (lastAim.y < 0)
        onGround = false;

    shakeCallback(lastAim); // 初始化震动
    particleEmitter_.emitDashLine(player_.getPosition(), lastAim); // 发射冲刺轨迹线
    particleEmitter_.emitDashBurst(player_.getPosition(), 10); // 发射圆形冲击波

    return PlayerState::Dash;
}

PlayerState Player::dashUpdate() {
    // 更新计时器
    if (dashRefillCooldownTimer > 0)
        dashRefillCooldownTimer -= deltaTime;

    // 更新是否落地
    groundCheck(false);

    // 处理超级跳
    if (input_.jump.pressed && onGround && lastAim.x != 0) {
        stateMachine_.stopCoroutine();
        superJump();
        return PlayerState::Normal;
    }

    // 处理协程状态
    if (messages_.isDone)
        return PlayerState::Normal;

    // 发射粒子
    particleEmitter_.emitDashLaunch(player_.getPosition(), lastAim);

    return PlayerState::Dash;
}

void Player::dashEnd() {
    std::cout << "Exit Dash State" << std::endl;
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

    dashDir = dir;
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
            if (dashAttackTimer > 0 && lastAim.x != 0 && onGround)
                superJump();
            else
                jump();
        } else { // 墙跳检测
            if (wallJumpCheck(1)) {
                if (dashAttackTimer > 0 && dashDir.x == 0 && dashDir.y == -1)
                    superWallJump(-1);
                else
                    wallJump(-1);
            } else if (wallJumpCheck(-1)) {
                if (dashAttackTimer > 0 && dashDir.x == 0 && dashDir.y == -1)
                    superWallJump(1);
                else
                    wallJump(1);
            }
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
    if (input_.down.check == false && speed.y > 0 && wallSlideTimer > 0 &&
        ! checkNextCollide(sf::Vector2f(1, 0) * static_cast<float>(moveX)).empty()) {
        wallSlideDir = moveX;
    }
    if (wallSlideDir != 0) {
        maxFall = std::lerp(MaxFall, WallSlideStartMax, wallSlideTimer / WallSlideTime); // 插值，使最大下落速度缓慢提升
        // 发射粒子
        particleEmitter_.emitWallJump(player_.getPosition() + sf::Vector2f(facing * 40, 0),
                              1, -25, 55, -facing);
    }
    if (wallSlideTimer > 0 && wallSlideDir) {
        wallSlideTimer = approach(wallSlideTimer, 0, deltaTime);
        wallSlideDir = 0;
    }
}

void Player::wallBoostCheck() {
    if (wallBoostTimer <= 0) return ;
    wallBoostTimer -= deltaTime;
    if (moveX == wallBoostDir) {
        // 切换至墙跳
        speed.x = WallJumpHSpeed * moveX;
        stamina += ClimbJumpCost; // 返还体力
        wallBoostTimer = 0;
    }
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

void Player::groundCheck(bool replyDash) {
    if (checkNextCollide({0, 1}).empty()) {
        onGround = false;
    } else {
        onGround = true;
        if (replyDash) dashes = MaxDashes;
    }
}

bool Player::climbCheck(int dir, float yAdd) {
    return ! checkNextCollide({dir * ClimbCheckDist, yAdd}).empty() && stamina > 0;
}

bool Player::wallJumpCheck(float dir) {
    return ! checkNextCollide({dir * WallJumpCheckDist, 0}).empty();
}

void Player::handleNormalClimbing() {
    if (climbNoMoveTimer > 0) return ;
    float target = 0;

    if (input_.up.check)
        target = ClimbUpSpeed;
    else if (input_.down.check) {
        target = ClimbDownSpeed;
        // 发射粒子
        particleEmitter_.emitWallJump(player_.getPosition() + sf::Vector2f(facing * 40, 0),
                              1, -25, 55, -facing);
    }

    lastClimbMove = target != 0 ? std::copysign(1.0, target) : 0;
    speed.y = approach(speed.y, target, ClimbAccel * deltaTime);
}

void Player::handleStaminaConsumption() {
    if (climbNoMoveTimer > 0) return ;
    if (lastClimbMove == -1) {
        stamina -= ClimbUpCost * deltaTime;
    } else {
        if (lastClimbMove == 0)
            stamina -= ClimbStillCost * deltaTime;
    }
}

std::vector<Entity> Player::checkNextCollide(sf::Vector2f unit) {
    // 获取角色位置和大小
    auto position = player_.getGlobalBounds().position;
    auto size = player_.getGlobalBounds().size;

    // 检测碰撞
    return nowlevel_.collision(sf::FloatRect(position + unit, size), speed);
}

void Player::jump(bool particle) {
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

    // 发射粒子效果
    if (particle)
        particleEmitter_.emitLandingDust(player_.getPosition() + sf::Vector2f(0, 55),
                                1, -40, 40);
    // 启动动画
    animationComponent_.triggerJump();
}

void Player::superJump() {
    // 初始化
    input_.jump.consumeBuffer();
    jumpGraceTimer = 0;
    varJumpTimer = VarJumpTime;
    wallSlideTimer = WallSlideTime;
    wallBoostTimer = 0;
    dashAttackTimer = 0;

    // 应用速度
    speed.x = SuperJumpH * (moveX == 0 ? facing : moveX);
    speed.y = SuperJumpSpeed;
    if (lastAim.y > 0) { // 斜下冲刺触发
        speed.x *= DuckSuperJumpXMult;
        speed.y *= DuckSuperJumpYMult;
    }
    varJumpSpeed = speed.y;

    // 回复冲刺次数
    if (dashRefillCooldownTimer <= 0)
        dashes = MaxDashes;

    // 发射粒子
    particleEmitter_.emitLandingDust(player_.getPosition() + sf::Vector2f(0, 55),
                                1.5, -40, 40);
    // 启动动画
    animationComponent_.triggerJump();
}

void Player::wallJump(int dir) {
    // 初始化
    input_.jump.consumeBuffer();
    jumpGraceTimer = 0;
    varJumpTimer = VarJumpTime;
    wallSlideTimer = WallSlideTime;
    wallBoostTimer = 0;

    // 强制移动
    if (moveX != 0) {
        forceMoveX = dir;
        forceMoveXTimer = WallJumpForceTime;
    }

    // 设置速度
    speed.x = WallJumpHSpeed * dir;
    speed.y = WallJumpSpeedY;
    varJumpSpeed = speed.y;

    // 发射粒子
    particleEmitter_.emitWallJump(player_.getPosition() + sf::Vector2f(-dir * 40, 0),
                          15, -25, 55, dir);
    // 启动动画
    animationComponent_.triggerJump();
}

void Player::superWallJump(int dir) {
    // 初始化
    input_.jump.consumeBuffer();
    jumpGraceTimer = 0;
    varJumpTimer = SuperWallJumpVarTime;
    wallSlideTimer = WallSlideTime;
    wallBoostTimer = 0;

    // 设置速度
    speed.x = SuperWallJumpH * dir;
    speed.y = SuperWallJumpSpeedY;
    varJumpSpeed = speed.y;

    // 发射粒子
    particleEmitter_.emitWallJump(player_.getPosition() + sf::Vector2f(-dir * 40, 0),
                          25, -25, 55, dir);
    // 启动动画
    animationComponent_.triggerJump();
}

void Player::climbJump() {
    climbJumpProtectionTimer = ClimbJumpProtectionTime;
    if (!onGround) {
        stamina -= ClimbJumpCost;
    }
    jump(false);
    // 墙面助推
    if (moveX == 0)  // 没有水平输入时
    {
        wallBoostDir = -facing;           // 助推方向与面朝方向相反
        wallBoostTimer = ClimbJumpBoostTime;   // 0.2秒助推窗口
    }
    // 发射粒子
    particleEmitter_.emitWallJump(player_.getPosition() + sf::Vector2f(facing * 40, 0),
                          20, -15, 55, -facing);
    // 启动动画
    animationComponent_.triggerJump();
}

void Player::climbHop() {
    speed.x = facing * ClimbHopX;
    speed.y = std::min(speed.y, ClimbHopY);
    forceMoveX = 0;
    forceMoveXTimer = ClimbHopForceTime;
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
    int hitDirection = std::copysign(1.0, data.Direction.x);

    // 高速水平墙角修正
    if (horizontalCornerCorrection()) {
        moveH(data.Remaining.x);
        return ;
    }

    // 处理不同方向碰撞
    if (hitDirection > 0) {
        handleRightWallCollision(data);
    } else {
        handleLeftWallCollision(data);
    }

    speed.x = 0;
    dashAttackTimer = 0;
}

void Player::onCollideV(const CollisionData& data) {
    int hitDirection = std::copysign(1.0, data.Direction.y);

    // 向上墙角修正
    if (upwardCornerCorrection()) {
        moveV(data.Remaining.y);
        return ;
    }
    // 冲刺向下墙角修正
    if (dashDownCornerCorrection()) {
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
    dashAttackTimer = 0;
}

void Player::handleLeftWallCollision(const CollisionData &data) {
}

void Player::handleRightWallCollision(const CollisionData &data) {
}

void Player::handleGroundCollision(const CollisionData &data) {
    // 更新状态
    stamina = ClimbMaxStamina;
    wallSlideTimer = WallSlideTime;
    // 发射粒子效果
    particleEmitter_.emitLandingDust(player_.getPosition() + sf::Vector2f(0, 55),
                            (data.Moved.y + data.Remaining.y) / deltaTime / (DashSpeed / 1.5), -40, 40);
    // 启动动画
    animationComponent_.triggerLanding(0.5 * (data.Moved.y + data.Remaining.y) / deltaTime / MaxFall);
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

bool Player::dashDownCornerCorrection() {
    // 冲刺状态且向下移动
    if (stateMachine_.getCurrentState() != PlayerState::Dash) return false;
    if (speed.y <= 0) return false;

    auto check = [this](float sign) -> bool {
        for (int i = 1; i <= CornerCorrection; i ++) {
            auto collision = checkNextCollide({static_cast<float>(i) * sign, 1});
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

bool Player::horizontalCornerCorrection() {
    if (std::abs(speed.x) < 1000) return false;

    float dirX = std::copysign(1.0, speed.x);

    for (int i = 1; i <= CornerCorrection; i ++) {
        // 尝试两个方向
        for (int yDir = -1; yDir <= 1; yDir += 2) {
            float testY = i * yDir;
            auto collision = checkNextCollide({dirX, testY});
            if (collision.empty()) {
                moveV(testY);
                return true;
            }
        }
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

void Player::updateDashAttackTimer() {
    if (dashAttackTimer > 0)
        dashAttackTimer -= deltaTime;
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
    window.draw(animationComponent_.applyTransform(player_));
    particleEmitter_.render(window);
}

sf::Vector2f Player::getPosition() const {
    return player_.getPosition();
}
