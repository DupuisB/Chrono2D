#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <optional>

#include "ECS/ECS.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/CollisionSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "Systems/ConstraintSystem.hpp"
#include "Systems/KeyEventSystem.hpp"

const float PI = 3.14159265358979323846f;

bool paused = false;

const float TIME_STEP = 10.0f;
const float TICK = 0.01f;

std::vector<Vec2f> makePolygon(const Vec2f center, const float size, const int num_sides, float angle=0.0f);
std::vector<Vec2f> makeRect(const Vec2f topLeft, float width, float height, float angle = 0.0f);
void setupRigidBody(Entity& entity, std::shared_ptr<ECS> ecs, const std::vector<Vec2f>& points, float mass, const sf::Color& color, bool isStatic);

// Main function to demonstrate our ECS system with a cube falling onto a ground line
int main() {
    // Create window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "ECS Physics Demo");
    window.setFramerateLimit(60);

    // Initialize ECS
    std::shared_ptr<ECS> ecs = std::make_shared<ECS>();
    ecs->init();

    // Initialize systems
    PhysicsSystem physicsSystem(ecs);
    CollisionSystem collisionSystem(ecs);
    ConstraintSystem constraintSystem(ecs);
    RenderSystem renderSystem(ecs, &window);
    KeyEventSystem keyEventSystem(ecs, &window, paused);

    // Create ground entity
    Entity ground = ecs->createEntity();
    // Create a static ground rectangle
    std::vector<Vec2f> groundPoints = makeRect(Vec2f(0.0f, 500.0f), 800.0f, 50.0f);
    setupRigidBody(ground, ecs, groundPoints, 0.0f, sf::Color::Red, true);

    // Create A point in the square (left top)
    Entity player = ecs->createEntity();
    // Create a dynamic cube entity
    std::vector<Vec2f> cubePoints = makePolygon(Vec2f(400.0f, 100.0f), 50.0f, 4, 0.0f);
    ecs->addComponent<ControlledEntity>(player, ControlledEntity());
    keyEventSystem.updateControlledEntity();
    setupRigidBody(player, ecs, cubePoints, 1.0f, sf::Color::Blue, false);

    Entity dynRect = ecs->createEntity();
    std::vector<Vec2f> rectPoints = makePolygon(Vec2f(150.0f, 100.0f), 100.0f, 4, 0.0f);
    setupRigidBody(dynRect, ecs, rectPoints, 1.0f, sf::Color::Blue, false);

    // Game loop
    sf::Clock clock;
    while (window.isOpen()) {
        // Handle events using SFML 3.0 style event polling
        std::optional<sf::Event> eventOpt;
        while ((eventOpt = window.pollEvent())) {
            const sf::Event& event = *eventOpt;
            if (event.is<sf::Event::Closed>()) {
                window.close();
            }
            if (event.is<sf::Event::KeyPressed>()) {
                const sf::Event::KeyPressed* keyEventOpt = event.getIf<sf::Event::KeyPressed>();
                if (keyEventOpt) {
                    keyEventSystem.handleKeyPressedEvent(keyEventOpt);
                }
            }
        }

        if (!paused) {
            // Update systems
            physicsSystem.update(TICK * TIME_STEP);
            for (int i = 0; i < 20; i++) {
                collisionSystem.detectCollisions();
                constraintSystem.update();
            }
            physicsSystem.PBDupdate(TICK * TIME_STEP);
            renderSystem.render();
        }
    }

    return 0;
}

// Function to create a polygon with a given center, size, number of sides, and angle
// where size is the distance from the center to any vertex
std::vector<Vec2f> makePolygon(const Vec2f center, const float size, const int num_sides, float angle) {
    std::vector<Vec2f> points;
    float rotation = angle * (PI / 180.0f);

    for (int i = 0; i < num_sides; i++) {
        float theta = -2.0f * PI * i / num_sides + rotation;
        float x = center.x + size * std::cos(theta);
        float y = center.y + size * std::sin(theta);
        points.emplace_back(x, y);
    }

    return points;
}

std::vector<Vec2f> makeRect(const Vec2f topLeft, float width, float height, float angle) {
    std::vector<Vec2f> points;
    points.emplace_back(topLeft);
    points.emplace_back(topLeft.x + width, topLeft.y);
    points.emplace_back(topLeft.x + width, topLeft.y + height);
    points.emplace_back(topLeft.x, topLeft.y + height);
    // rotate by angle degrees around the center of the rectangle
    if (angle != 0.0f) {
        float rotation = angle * (PI / 180.0f);
        Vec2f center = Vec2f(topLeft.x + width / 2.0f, topLeft.y + height / 2.0f);
        for (auto& point : points) {
            float x = center.x + (point.x - center.x) * std::cos(rotation) - (point.y - center.y) * std::sin(rotation);
            float y = center.y + (point.x - center.x) * std::sin(rotation) + (point.y - center.y) * std::cos(rotation);
            point = Vec2f(x, y);
        }
    }
    return points;
}

PolygonConstraint generateConstraints(const std::vector<Vec2f>& points) {
    std::vector<float> constraints;
    std::vector<std::array<int, 2>> edges;
    Vec2f center = Vec2f(0, 0);
    for (const auto& p : points) {
        center += p;
    }
    center /= static_cast<float>(points.size());
    float edgeToEdge = (points[0] - points[1]).length();
    float centerToEdge = (points[0] - center).length();
    for (int i = 0; i < points.size(); i++) {
        for (int j = i + 1; j < points.size(); j++) {
            constraints.push_back((points[i] - points[j]).length());
            std::array<int, 2> edge;
            edge = {i, j};
            edges.push_back(edge);
        }
    }
    return PolygonConstraint(constraints, edges);
}

void setupRigidBody(Entity& entity, std::shared_ptr<ECS> ecs, const std::vector<Vec2f>& points, float mass, const sf::Color& color, bool isStatic) {
    ecs->addComponent<Mass>(entity, Mass(mass));
    ecs->addComponent<Position>(entity, Position(points));
    ecs->addComponent<PredictedPosition>(entity, PredictedPosition(points));
    if (!isStatic) {
        ecs->addComponent<InitialPosition>(entity, InitialPosition(points));
        std::vector<Vec2f> zeros(points.size(), Vec2f(0, 0));
        ecs->addComponent<Velocity>(entity, Velocity(zeros));
        ecs->addComponent<Acceleration>(entity, Acceleration(zeros));
        PolygonConstraint constraints = generateConstraints(points);
        ecs->addComponent<PolygonConstraint>(entity, constraints);
    }
    ecs->addComponent<RenderablePolygon>(entity, RenderablePolygon(color));
}
    
