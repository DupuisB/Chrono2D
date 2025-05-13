#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <optional>

#include "ECS/ECS.hpp"
#include "Systems.cpp"

// Main function to demonstrate our ECS system with a cube falling onto a ground line
int main() {
    // Create window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "ECS Physics Demo");
    window.setFramerateLimit(60);

    // Initialize ECS
    ECS ecs;
    ecs.init();

    // Initialize systems
    PhysicsSystem physicsSystem(&ecs);
    CollisionSystem collisionSystem(&ecs);
    RenderSystem renderSystem(&ecs, &window);

    // Create ground entity
    Entity ground = ecs.createEntity();
    
    // Create ground shape
    std::shared_ptr<sf::RectangleShape> groundShape = std::make_shared<sf::RectangleShape>();
    groundShape->setSize(sf::Vector2f(800.0f, 0.0f));
    groundShape->setFillColor(sf::Color(100, 200, 100));
    
    // Add components to ground
    ecs.addComponent<Wall>(Vec2f(0.0f, 580.0f), Vec2f(0.0f, 1.0f));
    ecs.addComponent<Shape>(ground, Shape(groundShape));
    ecs.addComponent<RigidBody>(ground, RigidBody(1000.0f, true)); // Static object

    // Create falling cube entity
    Entity cube = ecs.createEntity();
    
    // Create cube shape
    std::shared_ptr<sf::RectangleShape> cubeShape = std::make_shared<sf::RectangleShape>();
    cubeShape->setSize(sf::Vector2f(50.0f, 50.0f));
    cubeShape->setFillColor(sf::Color(200, 100, 100));
    
    // Add components to cube
    ecs.addComponent<Transform>(cube, Transform(Vec2f(375.0f, 100.0f), Vec2f(0.0f, 0.0f), Vec2f(0.0f, 0.0f)));
    ecs.addComponent<Shape>(cube, Shape(cubeShape));
    ecs.addComponent<RigidBody>(cube, RigidBody(1.0f, false)); // Dynamic object

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
                    Transform& transform = ecs.getData<Transform>(cube);
                    transform.velocity += Vec2f(0.0f, -50.0f);
                    transform.acceleration = Vec2f(0.0f, 0.0f);
                }
            }
        }

        // Calculate delta time
        float deltaTime = clock.restart().asSeconds();
        
        // Update systems
        physicsSystem.update(deltaTime * 10.0f);
        collisionSystem.update();
        renderSystem.update();
    }

    return 0;
}