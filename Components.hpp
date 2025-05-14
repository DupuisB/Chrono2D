#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <SFML/Graphics.hpp>
#include "utils/math.hpp"
#include "ECS/ECS.hpp"

// WARNING: every component must have a default constructor
// otherwise the if we ever remove an entity the ECS will not be happy
// it removes entities like this: data[entity] = T();
// also the Component data is stored in an array so it uses the default constructor

struct RigidRect{
    bool isStatic;
    std::array<Vec2f, 4> positions;
    std::array<Vec2f, 4> predictedPositions;
    std::array<Vec2f, 4> velocities;
    std::array<Vec2f, 4> accelerations;
    float mass;

    std::array<Vec2f, 4> getPositions() const {
        return positions;
    }

    RigidRect(const std::array<Vec2f, 4>& positions, bool isStatic, float m = 1.0f)
        : isStatic(isStatic), positions(positions), predictedPositions(positions),
          velocities{}, accelerations{}, mass(m) {if (isStatic) mass = 0.0f;}

    RigidRect() : isStatic(false), positions{}, predictedPositions{}, velocities{}, accelerations{}, mass(1.0f) {}

    RigidRect(const Vec2f& pos, const Vec2f& size, bool isStatic, float m = 1.0f)
        : isStatic(isStatic), positions{pos, pos + Vec2f(size.x, 0), pos + size, pos + Vec2f(0, size.y)},
          predictedPositions{pos, pos + Vec2f(size.x, 0), pos + size, pos + Vec2f(0, size.y)},
          velocities{}, accelerations{}, mass(m) {if (isStatic) mass = 0.0f;}
};

struct RectConstraint{
    Vec2f size;

    RectConstraint() : size(0, 0) {}
    RectConstraint(const Vec2f& size) : size(size) {}
};

struct RenderableRect{
    sf::Color color;

    RenderableRect() : color(sf::Color::Green) {}
    RenderableRect(const sf::Color& c) : color(c) {}
};


#endif // COMPONENTS_HPP