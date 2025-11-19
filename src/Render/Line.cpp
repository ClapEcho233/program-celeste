//
// Created by ClapEcho233 on 2025/11/19.
//

#include "Line.h"

Line::Line(float x, float y) :
    gridCountX_(x), gridCountY_(y) {
    work();
}

void Line::setGridCount(const sf::Vector2f &gridSize) {
    gridCountX_ = gridSize.x;
    gridCountY_ = gridSize.y;
    work ();
}

void Line::setGridCount(const float &gridCountX, const float &gridCountY) {
    setGridCount({gridCountX, gridCountY});
}

void Line::work() {
    gridLines_.clear ();
    float inv = 320 / gridCountX_;
    for (int i = 0; i <= gridCountX_; i++) {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0] = sf::Vertex(sf::Vector2f(i * inv, 0.0f), sf::Color(80, 80, 80, 100));
        line[1] = sf::Vertex(sf::Vector2f(i * inv, 180.0f), sf::Color(80, 80, 80, 100));
        gridLines_.push_back(line);
    }
    for (int i = 0; i <= gridCountY_; i++) {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0] = sf::Vertex(sf::Vector2f(0.0f, i * inv), sf::Color(80, 80, 80, 100));
        line[1] = sf::Vertex(sf::Vector2f(320.0f, i * inv), sf::Color(80, 80, 80, 100));
        gridLines_.push_back(line);
    }
}


void Line::render(sf::RenderWindow &window) {
    for (const auto& line : gridLines_) {
        window.draw(line);
    }
}



