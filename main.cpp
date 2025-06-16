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
#include <filesystem> // Required for std::filesystem::current_path
#include <SFML/Audio.hpp>

// Helper function to find GameObject by shapeId
GameObject* findGameObjectByShapeId(b2ShapeId shapeId, std::vector<GameObject>& gameObjects) {
    if (B2_IS_NULL(shapeId)) return nullptr;
    for (auto& obj : gameObjects) {
        if (B2_ID_EQUALS(obj.shapeId, shapeId)) {
            return &obj;
        }
    }
    return nullptr;
}


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
    b2Vec2 gravity = {0.0f, -10.0f};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    b2WorldId worldId = b2CreateWorld(&worldDef);
    if (B2_IS_NULL(worldId)) {
        std::cerr << "Failed to create Box2D world." << std::endl;
        return -1;
    }

    std::vector<GameObject> gameObjects;
    b2BodyId playerBodyId = b2_nullBodyId;
    int playerIndex = -1;

    // --- Load Map Objects ---
    // Populates gameObjects, playerBodyId, and playerIndex based on the selected map.
    playerIndex = loadMap1(worldId, gameObjects, playerBodyId);

    // --- Initialize Player Animations ---
    if (playerIndex != -1) {
        GameObject& playerObject = gameObjects[playerIndex];
        std::string basePath = "../assets/sprite/character/Poses/";

        playerObject.loadPlayerAnimation("idle", {basePath + "female_idle.png"}, 0.1f);
        playerObject.loadPlayerAnimation("walk", {basePath + "female_walk1.png", basePath + "female_walk2.png"}, 0.15f);
        playerObject.loadPlayerAnimation("jump", {basePath + "female_jump.png"}, 0.1f);
        playerObject.loadPlayerAnimation("fall", {basePath + "female_fall.png"}, 0.1f);

        playerObject.setPlayerAnimation("idle", false); // Initial state: idle, facing right
    } else {
        std::cerr << "Player object not found after map loading." << std::endl;
    }

    // --- Load Background Music ---
    // Initialize sound system
    initializeSounds();
    // Create a music object
    sf::Music backgroundMusic;
    // Load the music file
    if (!backgroundMusic.openFromFile("../assets/audio/backgroundmusic.ogg")) {
        std::cerr << "Failed to load background music!" << std::endl;
        return -1;
    }
    // Set music properties
    backgroundMusic.setLooping(true); // Enable looping
    backgroundMusic.setVolume(10); // Set volume (0 to 100)
    // Play the music
    backgroundMusic.play();


    // --- Game Loop Variables ---
    sf::Clock clock;
    int32_t subSteps = 8;   // Number of physics sub-steps per frame
    bool levelCompleted = false; // Flag to ensure "Level completed!" message prints only once


    // --- Main Game Loop ---
    while (window.isOpen()) {
        float elapsed_time = clock.restart().asSeconds();
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

        // --- Sensor Event Handling for Flag ---
        if (!levelCompleted) {
            b2SensorEvents sensorEvents = b2World_GetSensorEvents(worldId);
            
            for (int i = 0; i < sensorEvents.beginCount; ++i) {
                b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];
                GameObject* objA = findGameObjectByShapeId(event.sensorShapeId, gameObjects); // Shape that is the sensor
                GameObject* objB = findGameObjectByShapeId(event.visitorShapeId, gameObjects); // Shape that entered the sensor

                if (objA && objB) {
                    bool playerHitFlag = false;
                    // Check if one is player and the other is the flag sensor
                    // Case 1: objA is the flag sensor, objB is the player
                    if (objA->isFlag_prop_ && objA->isSensor_prop_ && objB->isPlayer) {
                        playerHitFlag = true;
                    } 
                    // Case 2: objB is the flag sensor, objA is the player
                    else if (objB->isFlag_prop_ && objB->isSensor_prop_ && objA->isPlayer) {
                        playerHitFlag = true;
                    }

                    if (playerHitFlag) {
                        std::cout << "Level completed !" << std::endl;
                        levelCompleted = true;
                        // Optional: Add further game end logic here (e.g., stop player, show message on screen)
                    }
                }
            }
            // Note: You might also want to handle sensorEvents.endEvents if needed
        }


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