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

const float TIME_STEP = 5.0f;
const float TICK = 0.02f;

// helper function returns vector of points after creating a rect
std::vector<Vec2f> createRect(const Vec2f& pos, const Vec2f& size) {
    std::vector<Vec2f> points;
    points.push_back(pos);
    points.push_back(pos + Vec2f(size.x, 0));
    points.push_back(pos + size);
    points.push_back(pos + Vec2f(0, size.y));
    return points;
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
    
    ecs->addComponent<RigidRect>(ground, RigidRect(Vec2f(0.0f, 500.0f), Vec2f(600.0f, 20.0f), true));
    ecs->addComponent<RenderableRect>(ground, RenderableRect());

    // static cube rotated 45 degrees
    Vec2f pointA = Vec2f(100.0f, 300.0f);
    Vec2f pointB = Vec2f(150.0f, 350.0f);
    Vec2f pointC = Vec2f(100.0f, 400.0f);
    Vec2f pointD = Vec2f(50.0f, 350.0f);

    // Create a static cube entity
    Entity staticCube = ecs->createEntity();
    ecs->addComponent<RigidRect>(staticCube, RigidRect({pointA, pointB, pointC, pointD}, true));
    ecs->addComponent<RenderableRect>(staticCube, RenderableRect(sf::Color::Green));

    // Create A point in the square (left top)
    Entity player = ecs->createEntity();
    ecs->addComponent<RigidRect>(player, RigidRect(Vec2f(50.0f, 100.0f), Vec2f(50.0f, 50.0f), false));
    ecs->addComponent<RectConstraint>(player, RectConstraint(Vec2f(50.0f, 50.0f)));
    ecs->addComponent<RenderableRect>(player, RenderableRect(sf::Color::Red));

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
                    RigidRect& playerData = ecs->getData<RigidRect>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i] += Vec2f(0.0f, -100.0f);
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::Right) {
                    RigidRect& playerData = ecs->getData<RigidRect>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i].x = 10.0f;
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::Left) {
                    RigidRect& playerData = ecs->getData<RigidRect>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i].x = -10.0f;
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::S) {
                    RigidRect& playerData = ecs->getData<RigidRect>(player);
                    for (int i=0; i<4; i++) {
                        playerData.velocities[i].x = 0.0f;
                    }
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::R) {
                    // reset cube position
                    RigidRect& playerData = ecs->getData<RigidRect>(player);
                    playerData.positions[0] = Vec2f(50.0f, 100.0f);
                    playerData.positions[1] = Vec2f(100.0f, 100.0f);
                    playerData.positions[2] = Vec2f(100.0f, 150.0f);
                    playerData.positions[3] = Vec2f(50.0f, 150.0f);
                    playerData.predictedPositions[0] = Vec2f(50.0f, 100.0f);
                    playerData.predictedPositions[1] = Vec2f(100.0f, 100.0f);
                    playerData.predictedPositions[2] = Vec2f(100.0f, 150.0f);
                    playerData.predictedPositions[3] = Vec2f(50.0f, 150.0f);
                    playerData.velocities[0] = Vec2f(0.0f, 0.0f);
                    playerData.velocities[1] = Vec2f(0.0f, 0.0f);
                    playerData.velocities[2] = Vec2f(0.0f, 0.0f);
                    playerData.velocities[3] = Vec2f(0.0f, 0.0f);
                } else if (keyEvent && keyEvent->code == sf::Keyboard::Key::P) {
                    // Pause the simulation
                    paused = !paused;
                }
            }
        }


        if (!paused) {
            // Update systems
            physicsSystem.update(TICK * TIME_STEP);
            for (int i = 0; i < 10; i++) {
                collisionSystem.detectCollisions();
                constraintSystem.update();
            }
            physicsSystem.PBDupdate(TICK * TIME_STEP);
            renderSystem.render();
        }
    }

    return 0;
}