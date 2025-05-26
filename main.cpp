#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "include/game_object.hpp"
#include "include/player.hpp"

// --- Map Loading ---
#include "maps/map1.hpp" // Change this to load different maps

#include <vector>
#include <cmath> 
#include <iostream>
#include <optional>
#include <numeric>
#include <iomanip>

/**
 * @brief Main entry point for the SFML Box2D Platformer game.
 * Initializes the game window, physics world, game objects, and runs the main game loop.
 * @return 0 if the game exits successfully, -1 on critical initialization failure.
 */
int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "SFML Box2D Platformer");
    window.setFramerateLimit(60);

    // Camera view for scrolling
    sf::View view = window.getDefaultView();

    // Initialize Box2D world
    b2Vec2 gravity = {0.0f, -10.0f}; // Standard gravity pointing downwards
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    b2WorldId worldId = b2CreateWorld(&worldDef);
    if (B2_IS_NULL(worldId)) {
        std::cerr << "Failed to create Box2D world." << std::endl;
        return -1;
    }

    std::vector<GameObject> gameObjects; // Stores all game objects
    b2BodyId playerBodyId = b2_nullBodyId; // ID of the player's body

    // --- Load Map Objects ---
    // Populates gameObjects, playerBodyId, and groundPlatformIds based on the selected map.
    loadMap1(worldId, gameObjects, playerBodyId);

    // --- End Load Map Objects ---

    // --- Game Loop Variables ---
    sf::Clock clock;        // Measures time between frames
    int32_t subSteps = 8;   // Number of physics sub-steps per frame for stability

    // --- Main Game Loop ---
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f); // Cap delta time to prevent physics instability with large steps

        // --- SFML Event Handling ---
        bool jumpKeyHeld = false;
        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }
        }

        // --- Input State Update ---
        // Check current state of movement and jump keys
        bool wantsToMoveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        bool wantsToMoveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        jumpKeyHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        // --- Player Movement ---
        // Utilise la fonction movePlayer de player.cpp au lieu de duplicer la logique
        movePlayer(worldId, playerBodyId, gameObjects, jumpKeyHeld,
                  wantsToMoveLeft, wantsToMoveRight, dt);

        // --- Box2D Physics Step ---
        b2World_Step(worldId, dt, subSteps);

        // --- Update SFML Graphics ---
        // Synchronize SFML shapes with Box2D body positions and rotations.
        for (auto& obj : gameObjects) {
            obj.updateShape();
        }

        // --- Camera Follow Player ---
        if (!B2_IS_NULL(playerBodyId)) {
            b2Vec2 playerPos = b2Body_GetPosition(playerBodyId);
            sf::Vector2f center = b2VecToSfVec(playerPos); // Convert Box2D position to SFML view center
            view.setCenter(center);
        }
        window.setView(view); // Apply the updated view

        // --- Rendering ---
        window.clear(sf::Color(135, 206, 235)); // Light blue background

        // Draw all game objects
        for (const auto& obj : gameObjects) {
            obj.draw(window);
        }

        window.display();
    }

    // --- Cleanup ---
    // Destroy the Box2D world and all bodies/shapes within it.
    if (!B2_IS_NULL(worldId)) {
        b2DestroyWorld(worldId);
        worldId = b2_nullWorldId;
    }

    return 0;
}