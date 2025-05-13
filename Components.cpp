#include <SFML/Graphics.hpp>
#include <memory>
#include "utils/math.hpp"
#include "ECS/ECS.hpp"

enum shapeType {
    CIRCLE,
    RECTANGLE,
    POLYGON
};

struct Transform {
    Vec2f position;
    Vec2f velocity;
    Vec2f acceleration;
    float rotation;    // Rotation angle in radians
    float angularVelocity;    // Angular velocity in radians/second
    float angularAcceleration;    // Angular acceleration in radians/second^2

    Transform() 
        : position(0, 0), velocity(0, 0), acceleration(0, 0),
          rotation(0), angularVelocity(0), angularAcceleration(0) {}
          
    Transform(const Vec2f& position, const Vec2f& velocity, const Vec2f& acceleration,
              float rotation = 0, float angularVelocity = 0, float angularAcceleration = 0)
        : position(position), velocity(velocity), acceleration(acceleration),
          rotation(rotation), angularVelocity(angularVelocity), 
          angularAcceleration(angularAcceleration) {}
};

struct Wall {
    Vec2f position;
    Vec2f normal;
    Vec2f size;  // Used for rendering and collision calculations

    Wall() : position(0, 0), normal(0, 1), size(100, 10) {}
    Wall(const Vec2f& position, const Vec2f& normal, const Vec2f& size)
        : position(position), normal(normal), size(size) {}
};

struct Shape {
    std::shared_ptr<sf::Shape> shape;
    shapeType type;
    
    // For calculating rotational inertia and collision responses
    Vec2f size;        // Width/height for rectangles, or radius for circles
    float density;     // Used to calculate mass based on shape size

    Shape() : type(CIRCLE), size(1, 1), density(1.0f) {}
    
    Shape(const std::shared_ptr<sf::Shape>& shape, shapeType type, const Vec2f& size, float density = 1.0f) 
        : shape(shape), type(type), size(size), density(density) {}
};

struct RigidBody {
    float mass;
    float inertia;    // Moment of inertia for rotational physics
    bool isStatic;
    float restitution; // Coefficient of restitution (bounciness)
    float friction;    // Friction coefficient

    RigidBody() : mass(1.0f), inertia(1.0f), isStatic(false), restitution(0.8f), friction(0.3f) {}
    
    RigidBody(float mass, float inertia, bool isStatic, float restitution = 0.8f, float friction = 0.3f) 
        : mass(mass), inertia(inertia), isStatic(isStatic), restitution(restitution), friction(friction) {}
    
    // Calculate inertia automatically based on shape
    void calculateInertia(const Shape& shape) {
        if (shape.type == CIRCLE) {
            // For a circle: I = 0.5 * m * r^2
            inertia = 0.5f * mass * shape.size.x * shape.size.x;
        } else if (shape.type == RECTANGLE) {
            // For a rectangle: I = (1/12) * m * (width^2 + height^2)
            inertia = (mass / 12.0f) * (shape.size.x * shape.size.x + shape.size.y * shape.size.y);
        }
    }
};