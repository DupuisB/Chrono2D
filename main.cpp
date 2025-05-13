#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <optional>

#include "ECS/ECS.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/CollisionSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "Systems/ConstraintSystem.hpp"

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
    
    ecs->addComponent<Body>(ground, Body(true, 0.0f));
    std::vector<Vec2f> groundPoints = createRect(Vec2f(0.0f, 500.0f), Vec2f(800.0f, 20.0f));
    ecs->addComponent<Composite>(ground, Composite(groundPoints, Vec2f(800.0f, 20.0f)));
    ecs->addComponent<Renderable>(ground, Renderable(groundPoints[0], groundPoints[2], ShapeType::RECT));

    // Create A point in the square (left top)
    Entity cubeA = ecs->createEntity();
    ecs->addComponent<Dynamic>(cubeA, Dynamic(Vec2f(100.0f, 100.0f), Vec2f(100.0f, 100.0f), Vec2f(0.0f, 0.0f), Vec2f(0.0f, 0.0f)));
    ecs->addComponent<Body>(cubeA, Body(false, 1.0f));
    std::vector<Vec2f> cubeAPoints = createRect(Vec2f(100.0f, 100.0f), Vec2f(50.0f, 50.0f));
    ecs->addComponent<Composite>(cubeA, Composite(cubeAPoints, Vec2f(50.0f, 50.0f)));
    ecs->addComponent<Renderable>(cubeA, Renderable(cubeAPoints[0], cubeAPoints[2], ShapeType::RECT));
    // Create B point in the square (right Top)
    Entity cubeB = ecs->createEntity();
    ecs->addComponent<Dynamic>(cubeB, Dynamic(Vec2f(150.0f, 100.0f), Vec2f(150.0f, 100.0f), Vec2f(0.0f, 0.0f), Vec2f(0.0f, 0.0f)));
    ecs->addComponent<Body>(cubeB, Body(false, 1.0f));

    Entity cubeC = ecs->createEntity();
    ecs->addComponent<Dynamic>(cubeC, Dynamic(Vec2f(100.0f, 150.0f), Vec2f(100.0f, 150.0f), Vec2f(0.0f, 0.0f), Vec2f(0.0f, 0.0f)));
    ecs->addComponent<Body>(cubeC, Body(false, 1.0f));

    Entity cubeD = ecs->createEntity();
    ecs->addComponent<Dynamic>(cubeD, Dynamic(Vec2f(150.0f, 150.0f), Vec2f(150.0f, 150.0f), Vec2f(0.0f, 0.0f), Vec2f(0.0f, 0.0f)));
    ecs->addComponent<Body>(cubeD, Body(false, 1.0f));

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
                    ecs->getData<Dynamic>(cubeA).velocity += Vec2f(0.0f, -10.0f);
                    ecs->getData<Dynamic>(cubeB).velocity += Vec2f(0.0f, -10.0f);
                    ecs->getData<Dynamic>(cubeC).velocity += Vec2f(0.0f, -10.0f);
                    ecs->getData<Dynamic>(cubeD).velocity += Vec2f(0.0f, -10.0f);
                }
            }
        }

        // Calculate delta time
        float deltaTime = clock.restart().asSeconds();
        
        // Update systems
        physicsSystem.update(deltaTime * 10.0f);
        for (int i = 0; i < 5; i++) {
            collisionSystem.detectCollisions();
            constraintSystem.update();
        }
        physicsSystem.PBDupdate(deltaTime * 10.0f);
        renderSystem.render();
    }

    return 0;
}