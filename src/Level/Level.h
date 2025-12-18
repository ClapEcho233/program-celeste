//
// Created by ClapEcho233 on 2025/11/18.
//

#ifndef CELESTE_LEVEL_H
#define CELESTE_LEVEL_H

#include "../Common/Renderable.h"
#include "../Common/json.hpp"
#include <vector>
#include <optional>
#include <stdio.h>

enum class EType {Platform, JumpThru, Hurt};

using json = nlohmann::json;

/**
 * 单个关卡 json 结构示例
 * {
 *   "Platform": [
 *     {"width": 10, "height": 10, "x": 10, "y": 10},
 *     ...
 *   ],
 *   "JumpThru": [...],
 *   "Hurt": [...],
 *   "position": {"x": 10, "y": 10},     // 玩家初始位置
 *   "bounds": {                         // 可选：显式设置地图世界边界
 *     "x": 0, "y": 0, "width": 3200, "height": 1800
 *   }
 * }
 */
class Entity : public IRenderable {
private:
    sf::RectangleShape entity_;
    EType type_;
    bool safe_;

public:
    Entity(EType type, sf::Vector2f size, sf::Vector2f position, bool safe);
    sf::RectangleShape getEntity() const; // 获取平台对象
    EType getType() const; // 获取平台类型
    bool getSafe() const; // 平台是否安全
    Entity setType(EType type); // 改变平台类型
    void render(sf::RenderWindow &window) override;
};

class Level: public IRenderable {
public:
    struct Transition {
        int nextLevelId;
        sf::FloatRect area;
    };

private:
    std::vector<Entity> platform_;
    std::vector<Entity> hurt_;
    std::vector<Entity> jumpThru_;
    std::vector<Transition> transitions_;
    sf::Vector2f position_;
    sf::Vector2f offset_;
    sf::FloatRect bounds_;

public:
    /**
     * 构造函数，内部解析关卡配置
     * @param configs 关卡 json
     */
    Level(json configs);

    /**
     * 碰撞检测
     * @param player 玩家碰撞箱
     * @param speed 碰撞时玩家速度
     * @return 碰撞到的实体，unsafe 在前
     */
    std::vector<Entity> collision(sf::FloatRect player, sf::Vector2f speed) const;
    bool jumpThruCheck(sf::FloatRect player, Entity entity) const; // 单向板穿越检测

    sf::Vector2f getPosition() const;

    /**
     * 获取关卡边界，用于限制镜头移动范围。
     * 优先使用配置中的 bounds；缺省则回退到实体包围盒。
     * @return 世界坐标系下的边界矩形
     */
    sf::FloatRect getBounds() const;

    const std::vector<Transition>& getTransitions() const;
    sf::Vector2f getOffset() const;

    void render(sf::RenderWindow &window) override;
};

class LevelManager: public IRenderable {
private:
    std::vector<Level> levels_;
    json configs_;
    int levelId_;

public:
    LevelManager();
    int getLevelId();
    void setLevelId(int levelId);
    const Level& getLevel() const;
    const Level& getLevelById(int id) const;
    int getLevelCount() const;
    void render(sf::RenderWindow &window) override;
    void render(sf::RenderWindow &window, std::optional<int> extraLevelId);
};

#endif //CELESTE_LEVEL_H
