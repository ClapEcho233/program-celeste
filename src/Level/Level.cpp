//
// Created by ClapEcho233 on 2025/11/18.
//

#include "Level.h"

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

sf::RectangleShape Entity::getEntity() {
    return entity_;
}

EType Entity::getType() const{
    return type_;
}

bool Entity::getSafe() {
    return safe_;
}

void Entity::render(sf::RenderWindow &window) {
    window.draw(entity_);
}

Level::Level(json configs) {
    // if (configs.is_object() == false) throw std::invalid_argument("The configs is not valid.");

    // lambda 复用代码
    auto work = [this, &configs](std::string name, EType type) -> void {
        for (const auto& config : configs.at(name)) {
            platform_.emplace_back(type,
                sf::Vector2f{config.at("width"), config.at("height")},
                sf::Vector2f{config.at("x"), config.at("y")},
                true);
        }
    };

    work("Platform", EType::Platform);
    work("JumpThru", EType::JumpThru);
    work("Hurt", EType::Hurt);

    position_ = sf::Vector2f{configs.at("position").at("x"), configs.at("position").at("y")};
}

std::vector<Entity> Level::collision(sf::FloatRect player) {
    std::vector<Entity> ret;

    // lambda 复用代码
    auto work = [&player, &ret](std::vector<Entity> list) -> void {
        for (auto entity : list) {
            if (entity.getEntity().getGlobalBounds().findIntersection(player)) {
                ret.push_back(entity);
            }
        }
    };

    work(hurt_);
    work(jumpThru_);
    work(platform_);

    return ret;
}

sf::Vector2f Level::getPosition() const {
    return position_;
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

Level LevelManager::getLevel() {
    return levels_.at(levelId_);
}

void LevelManager::render(sf::RenderWindow &window) {
    for (auto level : levels_) {
        level.render(window);
    }
}
