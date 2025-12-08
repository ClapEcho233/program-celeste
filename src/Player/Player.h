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
#include "StateMachine.h"
#include <iostream>
#include <cmath>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

enum class State {Normal, Climb, Dash};

class Player : public IRenderable {
private:
    static constexpr float MaxFall = 1600;               // 最大下落速度
    static constexpr float Gravity = 9000;               // 重力加速度 (像素/秒²)
    static constexpr float FastMaxFall = 2400;           // 快速下落最大速度
    static constexpr float FastMaxAccel = 3000;          // 快速下落加速度
    static constexpr float HalfGravThreshold = 400;      // 半重力阈值速度

    static constexpr float MaxRun = 900;                 // 最大跑步速度
    static constexpr float RunAccel = 10000;             // 跑步加速度
    static constexpr float RunReduce = 4000;             // 超速时减速力
    static constexpr float AirMult = 0.65;               // 空中移动系数
    static constexpr float WalkSpeed = 640;              // 行走速度

    static constexpr float JumpSpeed = -1050;            // 跳跃初始速度 (向上为负)
    static constexpr float JumpHBoost = 400;             // 跳跃水平助推
    static constexpr float VarJumpTime = 0.2;            // 可变跳跃时间窗口
    static constexpr float CeilingVarJumpGrace = 0.05;   // 天花板碰撞跳跃宽限
    static constexpr float JumpGraceTime = 0.1;          // 地面跳跃宽限时间

    static constexpr float CornerCorrection = 40;  // 墙角修正宽限像素

    static constexpr float WallSlideStartMax = 200;      // 墙面滑动起始最大下落速度
    static constexpr float WallSlideTime = 1.2;          // 墙面滑动总时间

    static constexpr float MaxDashes = 1;                // 最大冲刺次数
    static constexpr float DashSpeed = 2400;             // 冲刺速度
    static constexpr float EndDashSpeed = 1600;          // 冲刺结束速度
    static constexpr float EndDashUpMult = 0.75;         // 冲刺结束上升动量
    static constexpr float TrailCreationTime = 0.03;     // 残影创建时间间隔 (初始)
    static constexpr float TrailNumber = 4;              // 残影数量

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
    Level nowlevel_; // 当前关卡
    Input input_; // 键盘交互
    StateMachine stateMachine_;
    TrailEffectManager trailEffectManager_; // 残影控制器
    ParticleEmitter particleEmitter_; // 粒子发射器


    // 物理属性
    sf::Vector2f speed;
    int moveX; // 输入水平移动方向 (1:右 -1:左)

    // 状态相关
    bool onGround;
    bool wasOnGround;
    int dashes; // 当前冲刺次数
    float varJumpSpeed; // 可变跳跃基准速度
    float maxFall; // 最大下落速度
    int wallSlideDir; // 墙壁滑行面向方向
    sf::Vector2f lastAim; // 最后瞄准方向
    sf::Vector2f beforeDashSpeed;

    int trailNumber; // 剩余残影数量

    // 计时器
    float jumpGraceTimer; // 跳跃宽限计时器 (0.1s)
    float varJumpTimer; // 可变跳跃计时器 (0.2s)
    float wallSlideTimer; // 墙壁滑行计时器 (1.2s)
    float trailCreationTimer; // 残影创建计时器 (0.2s)

    // 回调函数
    std::function<void(sf::Vector2f)> shakeCallback;

public:

    Player(Level nowLevel, std::function<void(sf::Vector2f)> shake);
    ~Player() override;

    void update();

    void render(sf::RenderWindow &window) override;

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
    void trailControl(); // 残影控制

    // 地面检测（此时是否位于地面）
    void groundCheck();

    // 检测下一刻会不会碰撞
    std::vector<Entity> checkNextCollide(sf::Vector2f unit);

    // 跳跃
    void jump();

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

    // 应用重力
    void applyGravity();

    /**
     * 插值函数，用于使当前值靠近目标值
     * @param current 当前值
     * @param target 目标值
     * @param step 步长
     */
    float approach (const float& current, const float& target, const float& step);
};

#endif //CELESTE_PLAYER_H