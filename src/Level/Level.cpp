//
// Created by ClapEcho233 on 2025/11/18.
//

#include "Level.h"

Entity::Entity(EType type, sf::Vector2f size, sf::Vector2f position, bool safe)
    : type_(type), safe_(safe) {
    // 设置实体的大小
    entity_.setSize(size);

    // 根据类型设置不同颜色
    switch(type_) {
        case EType::Platform:
            entity_.setFillColor(sf::Color::Green);
            break;
        case EType::JumpThru:
            entity_.setFillColor(sf::Color::Blue);
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

EType Entity::getType() {
    return type_;
}

bool Entity::getSafe() {
    return safe_;
}

void Entity::render(sf::RenderWindow &window) {
    window.draw(entity_);
}
