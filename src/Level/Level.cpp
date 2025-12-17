//
// Created by ClapEcho233 on 2025/11/18.
//

#include "Level.h"

#include <iostream>
#include <limits>

Entity::Entity(EType type, sf::Vector2f size, sf::Vector2f position, bool safe)
    : type_(type), safe_(safe) {
    // 设置实体的大小
    entity_.setSize(size);

    entity_.setOrigin({entity_.getSize().x / 2, entity_.getSize().y / 2});

    // 根据类型设置不同颜色
    switch(type_) {
        case EType::Platform:
            entity_.setFillColor(sf::Color::Cyan);
            break;
        case EType::JumpThru:
            entity_.setFillColor(sf::Color::Magenta);
            break;
        case EType::Hurt:
            entity_.setFillColor(sf::Color::Red);
            break;
    }

    // 设置位置
    entity_.setPosition(position);
}

sf::RectangleShape Entity::getEntity() const {
    return entity_;
}

EType Entity::getType() const {
    return type_;
}

bool Entity::getSafe() const {
    return safe_;
}

Entity Entity::setType(EType type) {
    return Entity(type, entity_.getSize(), entity_.getPosition(), safe_);
}

void Entity::render(sf::RenderWindow &window) {
    window.draw(entity_);
}

Level::Level(json configs) {
    // if (configs.is_object() == false) throw std::invalid_argument("The configs is not valid.");

    // lambda 复用代码
    auto work = [&configs](std::string name, EType type, std::vector<Entity>& target) -> void {
        for (const auto& config : configs.at(name)) {
            target.emplace_back(type,
                sf::Vector2f{config.at("width"), config.at("height")},
                sf::Vector2f{config.at("x"), config.at("y")},
                true);
        }
    };

    work("Platform", EType::Platform, platform_);
    work("JumpThru", EType::JumpThru, jumpThru_);
    work("Hurt", EType::Hurt, hurt_);

    position_ = sf::Vector2f{configs.at("position").at("x"), configs.at("position").at("y")};

    // 计算地图边界，方便镜头限制
    if (configs.contains("bounds")) {
        const auto& bounds = configs.at("bounds");
        // 显式设置世界边界（优先级最高）
        bounds_ = sf::FloatRect(
            sf::Vector2f{bounds.at("x"), bounds.at("y")},
            sf::Vector2f{bounds.at("width"), bounds.at("height")}
        );
        return;
    }

    bool hasEntity = false;
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    auto expand = [&](const Entity& entity) {
        hasEntity = true;
        // SFML3 的 FloatRect 使用 position/size 字段
        sf::FloatRect rect = entity.getEntity().getGlobalBounds();
        minX = std::min(minX, rect.position.x);
        minY = std::min(minY, rect.position.y);
        maxX = std::max(maxX, rect.position.x + rect.size.x);
        maxY = std::max(maxY, rect.position.y + rect.size.y);
    };

    auto applyExpand = [&](const std::vector<Entity>& list) {
        for (const auto& entity : list) {
            expand(entity);
        }
    };

    applyExpand(platform_);
    applyExpand(jumpThru_);
    applyExpand(hurt_);

    if (hasEntity) {
        bounds_ = sf::FloatRect({minX, minY}, {maxX - minX, maxY - minY});
    } else {
        bounds_ = sf::FloatRect();
    }
}

std::vector<Entity> Level::collision(sf::FloatRect player, sf::Vector2f speed) {
    std::vector<Entity> ret;

    // lambda 复用代码
    auto work = [&player, &ret, this, speed](std::vector<Entity> list) -> void {
        for (auto entity : list) {
            if (entity.getType() != EType::JumpThru) {
                if (entity.getEntity().getGlobalBounds().findIntersection(player))
                    ret.push_back(entity);
            } else {
                if (jumpThruCheck (player, entity) && entity.getEntity().getGlobalBounds().findIntersection(player)
                    && speed.y >= 0)
                    ret.push_back(entity.setType(EType::Platform));
            }
        }
    };

    work(hurt_);
    work(jumpThru_);
    work(platform_);

    return ret;
}

bool Level::jumpThruCheck(sf::FloatRect player, Entity entity) {
    return player.position.y + player.size.y - 1 <=
        entity.getEntity().getPosition().y - entity.getEntity().getSize().y / 2;
}

sf::Vector2f Level::getPosition() const {
    return position_;
}

sf::FloatRect Level::getBounds() const {
    return bounds_;
}

void Level::render(sf::RenderWindow &window) {
    // lambda 复用代码
    auto work = [&window](std::vector<Entity> list) -> void {
        for (auto entity : list) {
            entity.render(window);
        }
    };

    work(platform_);
    work(jumpThru_);
    work(hurt_);
}

LevelManager::LevelManager() : levelId_(0) {
    // 打开关卡配置文件
    FILE *fp = fopen("assets/LevelConfigs.json", "r");
    // 检查文件是否成功打开
    if (!fp) throw std::runtime_error("Could not open LevelConfigs.json");

    std::string file;
    while (!feof(fp)) {
        file.push_back(fgetc(fp));
    }

    // 把最后一个 EOF 去掉
    file.pop_back();
    // 关闭文件，释放资源
    fclose(fp);

    // 解析 json
    configs_=json::parse(file);
    for (auto config : configs_) {
        levels_.emplace_back(config);
    }
}

void LevelManager::setLevelId(int levelId) {
    levelId_ = levelId;
}

int LevelManager::getLevelId() {
    return levelId_;
}

const Level& LevelManager::getLevel() const {
    return levels_.at(levelId_);
}

void LevelManager::render(sf::RenderWindow &window) {
    for (auto level : levels_) {
        level.render(window);
    }
}
