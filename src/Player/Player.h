//
// Created by ClapEcho233 on 2025/11/18.
//

#ifndef CELESTE_PLAYER_H
#define CELESTE_PLAYER_H

#include "../Common/Renderable.h"
#include "../Level/Level.h"
#include "../Input/Input.h"
#include "../Render/TrailEffect.h"
#include "../Render/Particle.h"
#include "AnimationComponent.h"
#include "StateMachine.h"
#include <iostream>
#include <cmath>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

enum class State {Normal, Climb, Dash};

class Player : public IRenderable {
private:
    static constexpr float MaxFall = 1600;                           // 最大下落速度
    static constexpr float Gravity = 9000;                           // 重力加速度 (像素/秒²)
    static constexpr float HalfGravThreshold = 400;                  // 半重力阈值速度

    static constexpr float MaxRun = 900;                             // 最大跑步速度
    static constexpr float RunAccel = 10000;                         // 跑步加速度
    static constexpr float RunReduce = 4000;                         // 超速时减速力
    static constexpr float AirMult = 0.65;                           // 空中移动系数
    static constexpr float WalkSpeed = 640;                          // 行走速度

    static constexpr float JumpSpeed = -1050;                        // 跳跃初始速度 (向上为负)
    static constexpr float JumpHBoost = 400;                         // 跳跃水平助推
    static constexpr float VarJumpTime = 0.2;                        // 可变跳跃时间窗口
    static constexpr float CeilingVarJumpGrace = 0.05;               // 天花板碰撞跳跃宽限
    static constexpr float JumpGraceTime = 0.1;                      // 地面跳跃宽限时间

    static constexpr float CornerCorrection = 40;                    // 墙角修正宽限像素

    static constexpr float WallSlideStartMax = 200;                  // 墙面滑动起始最大下落速度
    static constexpr float WallSlideTime = 1.2;                      // 墙面滑动总时间

    static constexpr float MaxDashes = 1;                            // 最大冲刺次数
    static constexpr float DashSpeed = 2400;                         // 冲刺速度
    static constexpr float EndDashSpeed = 1600;                      // 冲刺结束速度
    static constexpr float EndDashUpMult = 0.75;                     // 冲刺结束上升动量
    static constexpr float TrailCreationTime = 0.03;                 // 残影创建时间间隔 (初始)
    static constexpr float TrailNumber = 4;                          // 残影数量
    static constexpr float DashAttackTime = 0.3;                     // 冲刺攻击时间

    static constexpr float ClimbMaxStamina = 110;                    // 最大体力
    static constexpr float ClimbUpCost = 100 / 2.2;                  // 向上爬体力消耗 (=45.45/s)
    static constexpr float ClimbStillCost = 100 / 10.0;              // 静止体力消耗 (=10/s)
    static constexpr float ClimbJumpCost = 110 / 4.0;                // 攀爬跳跃体力消耗 (=27.5/s)
    static constexpr float ClimbCheckDist = 20;                      // 攀爬检测距离
    static constexpr float ClimbNoMoveTime = 0.1;                    // 抓墙后短暂不可移动时间
    static constexpr float ClimbTiredThreshold = 20;                 // 疲劳阈值
    static constexpr float ClimbUpSpeed = -450;                      // 向上爬速度
    static constexpr float ClimbDownSpeed = 800;                     // 向下爬速度
    static constexpr float ClimbAccel = 9000;                        // 攀爬加速度
    static constexpr float ClimbGrabYMult = 0.2;                     // 抓墙时垂直速度乘数
    static constexpr float ClimbHopY = -1200;                        // 攀爬跳跃垂直速度
    static constexpr float ClimbHopX = 1000;                         // 攀爬跳跃水平速度
    static constexpr float ClimbHopForceTime = 0.2;                  // 攀爬跳跃强制移动时间
    static constexpr float ClimbJumpProtectionTime = 0.2;            // 墙跳保护时间
    static constexpr float ClimbJumpBoostTime = 0.2;                 // 墙面助推时间

    static constexpr float SuperJumpSpeed = JumpSpeed;               // 超级跳垂直速度
    static constexpr float SuperJumpH = 2600;                        // 超级跳水平速度
    static constexpr float DuckSuperJumpXMult = 1.25;                // 下蹲超级跳跃水平倍数
    static constexpr float DuckSuperJumpYMult = 0.5;                 // 下蹲超级跳跃垂直倍数
    static constexpr float DashRefillCooldownTime = 0.1;             // 超级跳回复冲刺冷却时间
    static constexpr float WallJumpCheckDist = 30;                   // 墙跳检测距离
    static constexpr float WallJumpForceTime = 0.16;                 // 墙跳强制移动时间
    static constexpr float WallJumpHSpeed = MaxRun + JumpHBoost;     // 墙跳水平速度
    static constexpr float WallJumpSpeedY = -1050;                   // 墙跳垂直速度
    static constexpr float SuperWallJumpSpeedY = -1600;              // 超级墙跳垂直速度
    static constexpr float SuperWallJumpVarTime = 0.25;              // 超级墙跳可变时间
    static constexpr float SuperWallJumpForceTime = 0.2;             // 超级墙跳强制时间
    static constexpr float SuperWallJumpH = MaxRun + JumpHBoost * 2; // 超级墙跳水平速度

    //------------------------------------------------------------------------------------------------------------------

    const sf::Color Normal = sf::Color(172, 50, 50);
    const sf::Color Used = sf::Color(68, 183, 255);

    //------------------------------------------------------------------------------------------------------------------

    // 碰撞数据结构
    struct CollisionData {
        sf::Vector2f Direction;       // 碰撞方向 (单位向量)
        sf::Vector2f Moved;           // 碰撞前已移动的距离
        Entity Hit;                   // 碰撞到的实体
        sf::Vector2f Remaining;       // 剩余移动距离
    };

private:
    // 对象
    sf::RectangleShape player_;
    LevelManager* levelManager_; // 当前关卡管理器（保持指针，便于切换关卡）
    Input input_; // 键盘交互
    StateMachine stateMachine_;
    TrailEffectManager trailEffectManager_; // 残影控制器
    ParticleEmitter particleEmitter_; // 粒子发射器
    AnimationComponent animationComponent_; // 动画控制器

    // 状态

    float facing; // 朝向
    int moveX; // 输入水平移动方向 (1:右 -1:左)
    float stamina; // 体力
    bool onGround; // 是否站在地上
    bool wasOnGround; // 上一帧是否站在地上
    int dashes; // 当前冲刺次数
    float varJumpSpeed; // 可变跳跃基准速度
    float maxFall; // 最大下落速度
    int wallSlideDir; // 墙壁滑行面向方向
    int forceMoveX; // 强制移动方向
    int lastClimbMove; // 上次攀爬移动方向
    float wallBoostDir; // 墙面助推方向
    bool alive; // 是否活着
    sf::Vector2f speed; // 速度
    sf::Vector2f lastAim; // 最后瞄准方向
    sf::Vector2f beforeDashSpeed; // 冲刺前速度
    sf::Vector2f dashDir; // 冲刺速度

    int trailNumber = 0; // 剩余残影数量

    // 计时器
    float jumpGraceTimer; // 跳跃宽限计时器 (0.1s)
    float varJumpTimer; // 可变跳跃计时器 (0.2s)
    float wallSlideTimer; // 墙壁滑行计时器 (1.2s)
    float trailCreationTimer; // 残影创建计时器 (0.2s)
    float forceMoveXTimer; // 强制移动计时器
    float climbNoMoveTimer; // 攀爬不可移动计时器 (0.1s)
    float wallBoostTimer; // 墙面助推计时器 (0.2s)
    float climbJumpProtectionTimer; // 墙跳保护计时器 (0.2s)
    float dashAttackTimer; // 冲刺攻击计时器 (0.3s)
    float dashRefillCooldownTimer; // 超级跳回复冲刺冷却计时器 (0.1s)

    // 回调函数
    std::function<void(sf::Vector2f)> shakeCallback;

public:
    Player(LevelManager& levelManager, std::function<void(sf::Vector2f)> shake);
    ~Player() override;

    void update();

    void render(sf::RenderWindow &window) override;

    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;
    void setPosition(sf::Vector2f pos);
    sf::Vector2f getSpeed() const;
    void setSpeed(sf::Vector2f s);
    void stopMovement();
    void resumeDash();
    bool isAlive() const;
    int getDashes() const;
    ParticleEmitter* getParticleEmitter();

private:
    // 状态回调函数
    void setupStateCallbacks();

    // Normal 状态
    PlayerState normalBegin();
    PlayerState normalUpdate();
    void normalEnd();

    // Climb 状态
    PlayerState climbBegin();
    PlayerState climbUpdate();
    void climbEnd();

    // Dash 状态
    PlayerState dashBegin();
    PlayerState dashUpdate();
    void dashEnd();
    int dashCoroutine(void *priv);

    void moveHControl(); // 水平移动控制
    void jumpControl(); // 跳跃控制
    void wallSlideCheck(); // 墙壁滑行检测
    void wallBoostCheck(); // 墙面助推检测
    void trailControl(); // 残影控制

    void groundCheck(bool replyDash = true); // 地面检测（此时是否位于地面）
    bool climbCheck(int dir, float yAdd = 0); // 攀爬检测
    bool wallJumpCheck(float dir); // 墙跳条件检测

    void handleNormalClimbing(); // 处理正常攀爬
    void handleStaminaConsumption(); // 处理体力消耗和状态管理

    // 检测下一刻会不会碰撞
    std::vector<Entity> checkNextCollide(sf::Vector2f unit);

    // 跳跃
    void jump(bool particle = true); // 普通跳跃
    void superJump(); // 超级跳跃
    void wallJump(int dir); // 离墙跳跃
    void superWallJump(int dir); // 超级离墙跳跃
    void climbJump(); // 攀爬跳跃 (沿墙面向上)
    void climbHop(); // 自动跳上平台

    // 应用移动，返回是否移动了任何距离
    bool moveH(float h); // 应用水平移动
    bool moveV(float v); // 应用竖直移动
    bool moveHExact(int h); // 精确应用水平移动
    bool moveVExact(int v); // 精确应用竖直移动

    // 碰撞回调
    void onCollideH(const CollisionData& data); // 水平碰撞回调
    void onCollideV(const CollisionData& data); // 竖直碰撞回调

    // 不同碰撞方向处理
    void handleLeftWallCollision(const CollisionData& data); // 碰到左侧墙面
    void handleRightWallCollision(const CollisionData& data); // 碰到右侧墙面
    void handleGroundCollision(const CollisionData& data); // 碰到天花板
    void handleCeilingCollision(const CollisionData& data); // 碰到地板

    // 墙角修正
    bool upwardCornerCorrection(); // 向上墙角修正
    bool dashDownCornerCorrection(); // 冲刺向下墙角修正
    bool horizontalCornerCorrection(); // 水平墙角修正

    void applyGravity(); // 应用重力

    void updateDashAttackTimer(); // 更新冲刺攻击计时器

    /**
     * 插值函数，用于使当前值靠近目标值
     * @param current 当前值
     * @param target 目标值
     * @param step 步长
     */
    float approach (const float& current, const float& target, const float& step);
};

#endif //CELESTE_PLAYER_H
