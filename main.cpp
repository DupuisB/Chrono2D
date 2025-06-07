#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "include/game_object.hpp"
#include "include/player.hpp"
#include "include/constants.hpp"

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
    int playerIndex = -1; // Index of the player in gameObjects

    // --- Load Map Objects ---
    // Populates gameObjects, playerBodyId, and groundPlatformIds based on the selected map.
    // loadMap1 now returns the player's index.
    playerIndex = loadMap1(worldId, gameObjects, playerBodyId);

    // --- Initialize Player Animations ---
    if (playerIndex != -1) {
        GameObject& playerObject = gameObjects[playerIndex];
        // Base path for player sprites
        std::string basePath = "../sprite/Female/Poses/";

        // Load animations: {animation_name, {frame_paths}, frame_duration}
        playerObject.loadPlayerAnimation("idle", {basePath + "female_idle.png"}, 0.1f);
        playerObject.loadPlayerAnimation("walk", {basePath + "female_walk1.png", basePath + "female_walk2.png"}, 0.15f);
        playerObject.loadPlayerAnimation("jump", {basePath + "female_jump.png"}, 0.1f);
        playerObject.loadPlayerAnimation("fall", {basePath + "female_fall.png"}, 0.1f);

        playerObject.setPlayerAnimation("idle", false); // Initial state: idle, facing right
    } else {
        std::cerr << "Player object not found after map loading." << std::endl;
    }


    // --- End Load Map Objects ---

    // --- Game Loop Variables ---
    sf::Clock clock;        // Measures time between frames
    int32_t subSteps = 8;   // Number of physics sub-steps per frame for stability

    // --- Main Game Loop ---
    while (window.isOpen()) {
        // Use the constant UPDATE_DELTA for fixed time step
        // Calculate delta time for this frame

        float elapsed_time = clock.restart().asSeconds(); //not used atm
        float dt = UPDATE_DELTA;


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
        if (playerIndex != -1 && !B2_IS_NULL(playerBodyId)) {
            movePlayer(worldId, playerBodyId, gameObjects[playerIndex], gameObjects, jumpKeyHeld,
                      wantsToMoveLeft, wantsToMoveRight, dt);
        }

        // --- Box2D Physics Step ---
        b2World_Step(worldId, dt, subSteps);

        // --- Update Player Animation ---
        if (playerIndex != -1) {
            gameObjects[playerIndex].updatePlayerAnimation(dt);
        }

        // --- Update SFML Graphics ---
        for (auto& obj : gameObjects) {
            obj.updateShape();
        }

        // --- Camera Follow Player ---
        if (!B2_IS_NULL(playerBodyId)) {
            b2Vec2 playerPos = b2Body_GetPosition(playerBodyId);
            sf::Vector2f center = b2VecToSfVec(playerPos);
            view.setCenter(center);
        }
        window.setView(view); 

        // --- Rendering ---
        window.clear(sf::Color(135, 206, 235));

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