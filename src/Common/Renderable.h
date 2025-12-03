//
// Created by ClapEcho233 on 2025/11/20.
//

#ifndef CELESTE_RENDERABLE_H
#define CELESTE_RENDERABLE_H

#include <SFML/Graphics.hpp>

class IRenderable {
public:
    virtual void render(sf::RenderWindow& window) = 0;
    virtual ~IRenderable() = default;
};

#endif //CELESTE_RENDERABLE_H