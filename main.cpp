#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <optional>

#include "ECS/ECS.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/CollisionSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "Systems/ConstraintSystem.hpp"

bool paused = false;

const float TIME_STEP = 10.0f;
const float TICK = 0.01f;

// helper function returns vector of points after creating a rect
void createRect(const Vec2f& pos, const Vec2f& size, sf::Color color, std::shared_ptr<ECS> ecs, Entity& entity, bool isStatic) {
    std::vector<Vec2f> points;
    std::vector<Vec2f> zeros = {Vec2f(0, 0), Vec2f(0, 0), Vec2f(0, 0), Vec2f(0, 0)};
    points.push_back(pos);
    points.push_back(pos + Vec2f(size.x, 0));
    points.push_back(pos + size);
    points.push_back(pos + Vec2f(0, size.y));
    float mass = isStatic ? 0.0f : 1.0f;
    ecs->addComponent<Mass>(entity, Mass(mass));
    ecs->addComponent<Position>(entity, Position(points));
    ecs->addComponent<PredictedPosition>(entity, PredictedPosition(points));
    if (!isStatic) {
        ecs->addComponent<Velocity>(entity, Velocity(zeros));
        ecs->addComponent<Acceleration>(entity, Acceleration(zeros));
        std::vector<float> constraints;
        std::vector<std::array<int, 2>> edges;
        for (int i = 0; i < points.size(); i++) {
            constraints.push_back((points[(i+1) % points.size()] - points[i]).length());
            std::array<int, 2> edge;
            if (i == points.size() - 1) {
                edge = {i, 0};
            } else {
                edge = {i, i+1};
            }
            edges.push_back(edge);
        }
        constraints.push_back((points[0] - points[2]).length());
        constraints.push_back((points[1] - points[3]).length());
        edges.push_back({0, 2});
        edges.push_back({1, 3});
        ecs->addComponent<PolygonConstraint>(entity, PolygonConstraint(constraints, edges));
    }
    ecs->addComponent<RenderablePolygon>(entity, RenderablePolygon(color));
}

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

    // Create ground entity
    Entity ground = ecs->createEntity();
    // Create a static ground rectangle
    createRect(Vec2f(0.0f, 500.0f), Vec2f(800.0f, 100.0f), sf::Color::Red, ecs, ground, true);

    // static cube rotated 45 degrees
    Vec2f pointA = Vec2f(100.0f, 300.0f);
    Vec2f pointB = Vec2f(150.0f, 350.0f);
    Vec2f pointC = Vec2f(100.0f, 400.0f);
    Vec2f pointD = Vec2f(50.0f, 350.0f);

    // Create a static cube entity
    Entity staticCube = ecs->createEntity();
    ecs->addComponent<Mass>(staticCube, Mass(0.0f));
    ecs->addComponent<Position>(staticCube, Position({pointA, pointB, pointC, pointD}));
    ecs->addComponent<PredictedPosition>(staticCube, PredictedPosition({pointA, pointB, pointC, pointD}));
    ecs->addComponent<RenderablePolygon>(staticCube, RenderablePolygon(sf::Color::Green));
    

    // Create A point in the square (left top)
    Entity player = ecs->createEntity();
    // Create a dynamic cube entity
    createRect(Vec2f(50.0f, 100.0f), Vec2f(50.0f, 50.0f), sf::Color::Blue, ecs, player, false);

    Entity dynRect = ecs->createEntity();
    createRect(Vec2f(150.0f, 100.0f), Vec2f(100.0f, 100.0f), sf::Color::Blue, ecs, dynRect, false);
    // change the mass of the dynamic rectangle
    Mass& mass = ecs->getData<Mass>(dynRect);
    mass.m = 1.0f;

    // Game loop
    sf::Clock clock;
    while (window.isOpen()) {
        // Handle events using SFML 3.0 style event polling
        std::optional<sf::Event> eventOpt;
        while ((eventOpt = window.pollEvent())) {
            const sf::Event& event = *eventOpt;
            
            // Check event type
            if (event.is<sf::Event::Closed>()) {
                window.close();
            }
            
            // Reset cube position on spacebar press
            if (event.is<sf::Event::KeyPressed>()) {
                const auto& keyEvent = event.getIf<sf::Event::KeyPressed>();
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Space) {
                    Velocity& playerData = ecs->getData<Velocity>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i] += Vec2f(0.0f, -30.0f);
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::Right) {
                    Velocity& playerData = ecs->getData<Velocity>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i].x = 30.0f;
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::Left) {
                    Velocity& playerData = ecs->getData<Velocity>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i].x = -30.0f;
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::S) {
                    Velocity& playerData = ecs->getData<Velocity>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i].x = 0.0f;
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::Down) {
                    Velocity& playerData = ecs->getData<Velocity>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i].y = 30.0f;
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::R) {
                    // reset cube position
                    Position& playerPosition = ecs->getData<Position>(player);
                    PredictedPosition& playerPredictedPosition = ecs->getData<PredictedPosition>(player);
                    Velocity& playerVelocity = ecs->getData<Velocity>(player);
                    playerPosition.positions[0] = Vec2f(50.0f, 100.0f);
                    playerPosition.positions[1] = Vec2f(100.0f, 100.0f);
                    playerPosition.positions[2] = Vec2f(100.0f, 150.0f);
                    playerPosition.positions[3] = Vec2f(50.0f, 150.0f);
                    playerPredictedPosition.predictedPositions[0] = Vec2f(50.0f, 100.0f);
                    playerPredictedPosition.predictedPositions[1] = Vec2f(100.0f, 100.0f);
                    playerPredictedPosition.predictedPositions[2] = Vec2f(100.0f, 150.0f);
                    playerPredictedPosition.predictedPositions[3] = Vec2f(50.0f, 150.0f);
                    playerVelocity.velocities[0] = Vec2f(0.0f, 0.0f);
                    playerVelocity.velocities[1] = Vec2f(0.0f, 0.0f);
                    playerVelocity.velocities[2] = Vec2f(0.0f, 0.0f);
                    playerVelocity.velocities[3] = Vec2f(0.0f, 0.0f);
                    // reset the dynamic cube too
                    Position& dynRectPosition = ecs->getData<Position>(dynRect);
                    PredictedPosition& dynRectPredictedPosition = ecs->getData<PredictedPosition>(dynRect);
                    Velocity& dynRectVelocity = ecs->getData<Velocity>(dynRect);
                    dynRectPosition.positions[0] = Vec2f(150.0f, 100.0f);
                    dynRectPosition.positions[1] = Vec2f(250.0f, 100.0f);
                    dynRectPosition.positions[2] = Vec2f(250.0f, 200.0f);
                    dynRectPosition.positions[3] = Vec2f(150.0f, 200.0f);
                    dynRectPredictedPosition.predictedPositions[0] = Vec2f(150.0f, 100.0f);
                    dynRectPredictedPosition.predictedPositions[1] = Vec2f(250.0f, 100.0f);
                    dynRectPredictedPosition.predictedPositions[2] = Vec2f(250.0f, 200.0f);
                    dynRectPredictedPosition.predictedPositions[3] = Vec2f(150.0f, 200.0f);
                    dynRectVelocity.velocities[0] = Vec2f(0.0f, 0.0f);
                    dynRectVelocity.velocities[1] = Vec2f(0.0f, 0.0f);
                    dynRectVelocity.velocities[2] = Vec2f(0.0f, 0.0f);
                    dynRectVelocity.velocities[3] = Vec2f(0.0f, 0.0f);
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::P) {
                    // Pause the simulation
                    paused = !paused;
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::E) {
                    Velocity& playerData = ecs->getData<Velocity>(player);
                    playerData.velocities[1] = Vec2f(0.0f, -15.0f);
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::A) {
                    Velocity& playerData = ecs->getData<Velocity>(player);
                    playerData.velocities[0] = Vec2f(0.0f, -15.0f);
                }
            }
        }


        if (!paused) {
            // Update systems
            physicsSystem.update(TICK * TIME_STEP);
            collisionSystem.detectCollisions();
            for (int i = 0; i < 1; i++) {
                constraintSystem.update();
            }
            physicsSystem.PBDupdate(TICK * TIME_STEP);
            renderSystem.render();
        }
    }

    return 0;
}