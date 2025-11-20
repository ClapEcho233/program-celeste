//
// Created by ClapEcho233 on 2025/11/19.
//

#ifndef CELESTE_LINE_H
#define CELESTE_LINE_H

#include <SFML/Graphics.hpp>

#include "Renderable.h"

class Line: public Renderable {
private:
    std::vector<sf::VertexArray> gridLines_;
    float gridCountX_;
    float gridCountY_;

public:
    Line(float x = 32, float y = 18);

    void setGridCount (const sf::Vector2f &gridSize);

    void setGridCount (const float &gridCountX, const float &gridCountY);

    void work ();

    void render(sf::RenderWindow &window) override;
};

#endif //CELESTE_LINE_H