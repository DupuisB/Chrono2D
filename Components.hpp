#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <SFML/Graphics.hpp>
#include "utils/math.hpp"
#include "ECS/ECS.hpp"

// WARNING: every component must have a default constructor
// otherwise the if we ever remove an entity the ECS will not be happy
// it removes entities like this: data[entity] = T();
// also the Component data is stored in an array so it uses the default constructor

enum class ShapeType{
    LINE,
    RECT
};

struct Dynamic{
    Vec2f position;
    Vec2f predictedPosition;
    Vec2f velocity;
    Vec2f acceleration;

    Dynamic() : position(0, 0), predictedPosition(0, 0), velocity(0, 0), acceleration(0, 0) {}
    Dynamic(const Vec2f& pos, const Vec2f& predPos, const Vec2f& vel, const Vec2f& acc)
        : position(pos), predictedPosition(predPos), velocity(vel), acceleration(acc) {}
};

struct Body{
    bool isStatic;
    float mass;

    Body() : isStatic(false), mass(1.0f) {}
    Body(bool isStatic, float mass) : isStatic(isStatic), mass(mass) {}
};

struct Composite{
    std::vector<Vec2f> points;
    Vec2f size;

    Composite() : size(0, 0) {}
    Composite(const std::vector<Vec2f>& points) : points(points), size(0, 0) {}
    Composite(const std::vector<Vec2f>& points, const Vec2f& size)
        : points(points), size(size) {}
};

struct Renderable{
    Vec2f pointA;
    Vec2f pointB;
    ShapeType type;

    Renderable() : pointA(0, 0), pointB(0, 0), type(ShapeType::RECT) {}
    Renderable(const Vec2f& a, const Vec2f& b, ShapeType t)
        : pointA(a), pointB(b), type(t) {}

};


#endif // COMPONENTS_HPP