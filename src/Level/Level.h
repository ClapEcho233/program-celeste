//
// Created by ClapEcho233 on 2025/11/18.
//

#ifndef CELESTE_LEVEL_H
#define CELESTE_LEVEL_H

#include "../Common/Renderable.h"
#include "../Common/json.hpp"
#include <vector>
#include <stdio.h>

enum class EType {Platform, JumpThru, Hurt};

using json = nlohmann::json;

/* 单个关卡 json 结构例子：
{
    "Platform": [
        {
            "width": 10,
            "height": 10,
            "x": 10,
            "y": 10
        },
        ...
    ],
    "JumpThru": [
        ...
    ],
    "Hurt":[
        ...
    ]
}
 */

class Entity : public IRenderable {
private:
    sf::RectangleShape entity_;
    EType type_;
    bool safe_;


public:
    Entity(EType type, sf::Vector2f size, sf::Vector2f position, bool safe);

    sf::RectangleShape getEntity();

    EType getType();

    bool getSafe();

    void render(sf::RenderWindow &window) override;
};

class Level: public IRenderable {
private:
    std::vector<Entity> platform_;
    std::vector<Entity> hurt_;
    std::vector<Entity> jumpThru_;

public:
    /**
     * 构造函数，内有解析游戏关卡配置
     * @param configs 游戏关卡的 json
     */
    Level(json configs);

    /**
     * 检测碰撞
     * @param player 待检测对象的碰撞箱
     * @return 碰撞到的实体，unsafe 在前
     */
    std::vector<Entity> collision(sf::FloatRect player);

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

    Level getLevel();

    void render(sf::RenderWindow &window) override;
};

#endif //CELESTE_LEVEL_H