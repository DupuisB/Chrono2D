#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <SFML/Graphics.hpp>
#include <vector>

#include "utils/math.hpp"
#include "ECS/ECS.hpp"

// WARNING: every component must have a default constructor
// otherwise the if we ever remove an entity the ECS will not be happy
// it removes entities like this: data[entity] = T();
// also the Component data is stored in an array so it uses the default constructor

struct Mass{
    float m;
    Mass() : m(1.0f) {}
    Mass(float m) : m(m) {}
};
struct Position{
    std::vector<Vec2f> positions;
    Vec2f center;
    Position() : positions(), center(0, 0) {}
    Position(const std::vector<Vec2f>& pos) : positions(pos) {
        center = Vec2f(0, 0);
        float size = static_cast<float>(positions.size());
        if (positions.size() == 0) return;
        for (const auto& p : positions) {
            center += p;
        }
        center /= static_cast<float>(positions.size());
    }
    Position(const std::vector<Vec2f>& pos, const Vec2f& center) : positions(pos), center(center) {}
    void updateCenter() {
        center = Vec2f(0, 0);
        float size = static_cast<float>(positions.size());
        if (positions.size() == 0) return;
        for (const auto& p : positions) {
            center += p;
        }
        center /= static_cast<float>(positions.size());
    }
};
struct PredictedPosition{
    std::vector<Vec2f> predictedPositions;
    PredictedPosition() : predictedPositions() {}
    PredictedPosition(const std::vector<Vec2f>& pos) : predictedPositions(pos) {}
};
struct Velocity{
    std::vector<Vec2f> velocities;
    Velocity() : velocities() {}
    Velocity(const std::vector<Vec2f>& vel) : velocities(vel) {}
};
struct Acceleration{
    std::vector<Vec2f> accelerations;
    Acceleration() : accelerations() {}
    Acceleration(const std::vector<Vec2f>& acc) : accelerations(acc) {}
};

struct PolygonConstraint{
    std::vector<float> lengthConstraints;
    std::vector<std::array<int, 2>> edges;

    PolygonConstraint() : lengthConstraints(), edges() {}
    PolygonConstraint(const std::vector<float>& constraints, const std::vector<std::array<int, 2>>& edges) : lengthConstraints(constraints), edges(edges) {}
};

struct RenderablePolygon{
    sf::Color color;
    bool fill = false;

    RenderablePolygon() : color(sf::Color::Green) {}
    RenderablePolygon(const sf::Color& c) : color(c) {}
};


#endif // COMPONENTS_HPP