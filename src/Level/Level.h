//
// Created by ClapEcho233 on 2025/11/18.
//

#ifndef CELESTE_LEVEL_H
#define CELESTE_LEVEL_H

#include "../Common/Renderable.h"

enum class EType {Platform, JumpThru, Hurt};

class Entity : public Renderable {
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

class Level: public Renderable {
private:

};

class LevelManager: public Renderable {

};

#endif //CELESTE_LEVEL_H