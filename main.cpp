#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "include/game_object.hpp"
#include "include/player.hpp"
#include "include/constants.hpp"

// --- Map Loading ---
#include "maps/map0.hpp" // Change this to load different maps
#include "maps/map1.hpp"
#include "maps/map2.hpp"
#include "maps/map3.hpp"
#include "maps/map4.hpp"


#include <vector>
#include <cmath> 
#include <iostream>
#include <optional>
#include <numeric>
#include <iomanip>
#include <filesystem> // Required for std::filesystem::current_path
#include <SFML/Audio.hpp>
#include <cstdint>



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

    // --- Time Freeze State ---
    static bool timeFreeze = false;
    static bool wasInTimeFreeze = false;
    static std::vector<std::tuple<b2BodyId, b2BodyType, b2Vec2, float>> frozenBodyData;

    // --- Transition overlay ---
    sf::RectangleShape transitionOverlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    transitionOverlay.setFillColor(sf::Color(0, 0, 0, 0)); // Start transparent
    bool isTransitioning = false;
    bool isFadingOut = false;
    bool isFadingIn = false;
    float transitionAlpha = 0.0f;
    const float TRANSITION_SPEED = 255.0f / 1.0f; // Full fade in 1 second

    // --- TimeFreeze overlay ---
    sf::RectangleShape timeFreezeOverlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    timeFreezeOverlay.setFillColor(sf::Color(0, 0, 0, 0)); // Start transparent
    bool isTimeFreezeTransitioning = false;
    bool isTimeFreezeOverlayFadingIn = false;
    bool isTimeFreezeOverlayFadingOut = false;
    float timeFreezeOverlayAlpha = 0.0f;
    const float TIMEFREEZE_FADE_SPEED = 255.0f / 1.0f; 

    // --- Load Background Music ---
    // Initialize sound system
    initializeSounds();
    sf::SoundBuffer timeFreezeSoundBuffer;
    sf::SoundBuffer timeUnfreezeSoundBuffer;
    std::unique_ptr<sf::Sound> timeUnfreezeSound;
    std::unique_ptr<sf::Sound> timeFreezeSound;
    bool soundsInitialized = false;
    if (!timeFreezeSoundBuffer.loadFromFile("../assets/audio/timefreezesound.wav")) {
        std::cerr << "Failed to load time freeze sound!" << std::endl;
        return -1;
    }
    if (!timeUnfreezeSoundBuffer.loadFromFile("../assets/audio/timeunfreezesound.wav")) {
        std::cerr << "Failed to load time unfreeze sound!" << std::endl;
        return -1;
    }

    // --- Load Background Map
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("../assets/objects/background.png")) {
        return -1; // Échec de chargement
    }
    backgroundTexture.setRepeated(true);
    sf::RectangleShape backgroundShape;
    backgroundShape.setTexture(&backgroundTexture);

    sf::Texture cloudTexture;
    if (!cloudTexture.loadFromFile("../assets/objects/cloud.png")) {
        return -1; // Échec de chargement
    }
    cloudTexture.setRepeated(true);
    sf::RectangleShape cloudShape;
    cloudShape.setTexture(&cloudTexture);
    
    // Create sound objects
    timeFreezeSound = std::make_unique<sf::Sound>(timeFreezeSoundBuffer);
    timeUnfreezeSound = std::make_unique<sf::Sound>(timeUnfreezeSoundBuffer);
    // Configure sounds
    timeFreezeSound->setVolume(25.0f);
    timeUnfreezeSound->setVolume(25.0f);
    timeFreezeSound->setPitch(2.0f);
    timeUnfreezeSound->setPitch(2.0f);

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

    
    // --- Main Game Loop ---

    for( int level=0; level <= 0; ++level ) {
        // The cleanup logic that was here has been moved to the end of the inner while loop
        // to consolidate all inter-level cleanup.

        playerIndex = loadMap4(worldId, gameObjects, playerBodyId);
        
        
        if (level > 1 || transitionAlpha > 0.0f) {
            isTransitioning = true;
            isFadingOut = false;  // Make sure we're not fading out
            isFadingIn = true;    // Set to fade in
            transitionAlpha = 255.0f; // Start fully black
            transitionOverlay.setFillColor(sf::Color(0, 0, 0, 255)); // Set to fully black
        }

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

            // --- Game Loop Variables ---
            sf::Clock clock;
            sf::Clock cloudClock; // Add clock for cloud movement
            sf::Time cloudPausedTime = sf::Time::Zero;
            bool cloudClockPaused = false; 
            int32_t subSteps = 8;   // Number of physics sub-steps per frame
            bool levelCompleted = false; // Flag to ensure "Level completed!" message prints only once   
            bool levelReset = false; // Flag to reset the current level
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


                // --- Handle Transition ---
                if (isTransitioning) {
                    if (isFadingOut) {
                        // Fade to black
                        transitionAlpha += TRANSITION_SPEED * elapsed_time;
                        if (transitionAlpha >= 255.0f) {
                            transitionAlpha = 255.0f;
                            isFadingOut = false;
                            if (levelReset) {
                                level--; // Decrement to repeat the current level
                            }
                            // Level completed or reset, break to next level
                            break;
                        }
                    } else if (isFadingIn) {
                        // Fade from black
                        transitionAlpha -= TRANSITION_SPEED * elapsed_time;
                        if (transitionAlpha <= 0.0f) {
                            transitionAlpha = 0.0f;
                            isFadingIn = false;
                            isTransitioning = false;
                        }
                    }
                    transitionOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(transitionAlpha)));
                    
                }
                if (isTimeFreezeTransitioning) {
                    if (isTimeFreezeOverlayFadingIn) {
                        timeFreezeOverlayAlpha += TIMEFREEZE_FADE_SPEED * elapsed_time;
                        if (timeFreezeOverlayAlpha >= 80.0f ) { // Max alpha for time freeze (semi-transparent)
                            timeFreezeOverlayAlpha = 80.0f;
                            isTimeFreezeOverlayFadingIn = false;
                            isTimeFreezeTransitioning = false;
                        }
                    } else if (isTimeFreezeOverlayFadingOut) {
                        timeFreezeOverlayAlpha -= TIMEFREEZE_FADE_SPEED * elapsed_time;
                        if (timeFreezeOverlayAlpha <= 0.0f) {
                            timeFreezeOverlayAlpha = 0.0f;
                            isTimeFreezeOverlayFadingOut = false;
                            isTimeFreezeTransitioning = false;
                            timeFreeze = false; // Actually disable time freeze after fade out
                        }
                    }
                    
                    // Update overlay color with current alpha
                    timeFreezeOverlay.setFillColor(sf::Color(100, 150, 255, static_cast<std::uint8_t>(timeFreezeOverlayAlpha))); // Light blue tint
                }

                // --- Input State Update ---
                // Check current state of movement and jump keys
                bool wantsToMoveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q);
                bool wantsToMoveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
                jumpKeyHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z);
                // Detect F key just pressed for time freeze toggle
                static bool prevFKeyState = false;
                bool currFKeyState = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F);
                bool wantsToTimeFreeze = false;
                if (currFKeyState && !prevFKeyState) {
                    wantsToTimeFreeze = true;
                }
                prevFKeyState = currFKeyState;

                // Detect R key press for level reset
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
                    levelReset = true;
                }

                // --- Time Freeze Logic ---
                
                if (wantsToTimeFreeze) {
                    if (!timeFreeze) {
                        // Starting time freeze - begin fade in
                        timeFreeze = true;
                        isTimeFreezeTransitioning = true;
                        isTimeFreezeOverlayFadingIn = true;
                        isTimeFreezeOverlayFadingOut = false;
                        if (timeFreezeSound && timeFreezeSound->getStatus() != sf::SoundSource::Status::Playing) {
                            timeFreezeSound->play();
                        }
                        cloudPausedTime += cloudClock.getElapsedTime();
                        cloudClockPaused = true;
                        std::cout << "Time frozen - fading in overlay." << std::endl;
                    } else if (!isTimeFreezeTransitioning) {
                        // Ending time freeze - begin fade out
                        isTimeFreezeTransitioning = true;
                        isTimeFreezeOverlayFadingOut = true;
                        isTimeFreezeOverlayFadingIn = false;
                        if (timeUnfreezeSound && timeUnfreezeSound->getStatus() != sf::SoundSource::Status::Playing) {
                            timeUnfreezeSound->play();
                        }
                        cloudClock.restart();
                        cloudClockPaused = false;
                        std::cout << "Time unfrozen - fading out overlay." << std::endl;
                    }
                }

                if(isTransitioning) {
                    // If transitioning, ignore player input
                    wantsToMoveLeft = false;
                    wantsToMoveRight = false;
                    jumpKeyHeld = false;
                    wantsToTimeFreeze = false;
                }
                

                // --- Player Movement ---
                if (playerIndex != -1 && !B2_IS_NULL(playerBodyId)) {
                    movePlayer(worldId, playerBodyId, gameObjects[playerIndex], gameObjects, jumpKeyHeld,
                            wantsToMoveLeft, wantsToMoveRight, dt);
                    
                    // Check if player has fallen off the map
                    b2Vec2 playerPos = b2Body_GetPosition(playerBodyId);
                    if (playerPos.y < -20.0f) { // Death plane at y = -20 meters
                        levelReset = true;
                    }
                }

                // Add these vectors to store original body types
                

                if(!timeFreeze){
                    // Just exited freeze mode - restore original body types AND velocities
                    if (wasInTimeFreeze) {
                        for (const auto& data : frozenBodyData) {
                            b2BodyId bodyId = std::get<0>(data);
                            b2BodyType originalType = std::get<1>(data);
                            b2Vec2 originalLinearVel = std::get<2>(data);
                            float originalAngularVel = std::get<3>(data);
                            
                            if (!B2_IS_NULL(bodyId)) {
                                // Restore body type
                                b2Body_SetType(bodyId, originalType);
                                // Restore velocities
                                b2Body_SetLinearVelocity(bodyId, originalLinearVel);
                                b2Body_SetAngularVelocity(bodyId, originalAngularVel);
                            }
                        }
                        frozenBodyData.clear();
                        wasInTimeFreeze = false;
                    }
                    
                    // Normal physics for everything
                    b2World_Step(worldId, dt, subSteps);
                } else {
                    // Just entered freeze mode - store original types AND velocities
                    if (!wasInTimeFreeze) {
                        frozenBodyData.clear();
                        for (auto& obj : gameObjects) {
                            if (!B2_IS_NULL(obj.bodyId) && !B2_ID_EQUALS(obj.bodyId, playerBodyId)) {
                                // Store original body type and velocities
                                b2BodyType originalType = b2Body_GetType(obj.bodyId);
                                b2Vec2 originalLinearVel = b2Body_GetLinearVelocity(obj.bodyId);
                                float originalAngularVel = b2Body_GetAngularVelocity(obj.bodyId);
                                
                                frozenBodyData.push_back(std::make_tuple(obj.bodyId, originalType, originalLinearVel, originalAngularVel));
                                
                                // Make completely immovable
                                b2Body_SetType(obj.bodyId, b2_staticBody);
                                b2Body_SetLinearVelocity(obj.bodyId, {0.0f, 0.0f});
                                b2Body_SetAngularVelocity(obj.bodyId, 0.0f);
                            }
                        }
                        wasInTimeFreeze = true;
                    }
                    
                    // During freeze - physics step with static bodies
                    b2World_Step(worldId, dt, subSteps);
                }

                // When exiting freeze mode, restore original body types
                static bool wasInFreeze = false;
                if (wasInFreeze && !timeFreeze) {
                    // Restore original body types
                    for (const auto& data : frozenBodyData) {
                        b2BodyId bodyId = std::get<0>(data);
                        b2BodyType originalType = std::get<1>(data);
                        b2Vec2 originalLinearVel = std::get<2>(data);
                        float originalAngularVel = std::get<3>(data);
                        
                        if (!B2_IS_NULL(bodyId)) {
                            // Restore body type
                            b2Body_SetType(bodyId, originalType);
                            // Restore velocities
                            b2Body_SetLinearVelocity(bodyId, originalLinearVel);
                            b2Body_SetAngularVelocity(bodyId, originalAngularVel);
                        }
                    }
                    frozenBodyData.clear();
                }
                wasInFreeze = timeFreeze;
                
                // --- Sensor Event Handling for Flag and Tremplin ---
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
                        if (objA && objB) {
                            if (objA->isTremplin_prop_ && objA->isSensor_prop_ && objB->isDynamic_val_) {
                                b2Vec2 impulse = {0, 10.0f};
                                objB->setPendingImpulsion(impulse);
                                std::cout << "impulseB, dynamic:"<< objB->isDynamic_val_ << " et densité :"<< objB->density_val_ << " et vitesse: "<<std::endl;
                            }
                            else if (objB->isTremplin_prop_ && objB->isSensor_prop_ && objA->isDynamic_val_) {
                                b2Vec2 impulse = {0, 10.0f};
                                objA->setPendingImpulsion(impulse);
                                std::cout << "impulseA" <<std::endl;
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

                if (level == 1) {
                    updateMap1(worldId, gameObjects, timeFreeze);
                }

                // --- Camera Follow Player ---
                if (!B2_IS_NULL(playerBodyId)) {
                    b2Vec2 playerPos = b2Body_GetPosition(playerBodyId);
                    sf::Vector2f center = b2VecToSfVec(playerPos);
                    view.setCenter(center);
                }

                // Parallax background
                const float backgroundParallaxFactor = 0.1f;
                const float cloudParallaxFactor = 0.2f;
                const float cloudDriftSpeed = 0.005f; // Pixels per millisecond drift speed
                sf::Vector2f viewTopLeft = view.getCenter() - view.getSize();

                backgroundShape.setPosition(viewTopLeft);
                backgroundShape.setSize(view.getSize() * 5.0f);
                sf::Rect<int> bgTextureRect(
                    sf::Vector2i(
                        static_cast<int>(view.getCenter().x * backgroundParallaxFactor),
                        static_cast<int>(view.getCenter().y * backgroundParallaxFactor)
                    ),
                    sf::Vector2i(
                        static_cast<int>(view.getSize().x),
                        static_cast<int>(view.getSize().y)
                    )
                );
                backgroundShape.setTextureRect(bgTextureRect);

                cloudShape.setPosition(viewTopLeft);
                cloudShape.setSize(view.getSize() * 5.0f);
                // Add time-based drift to cloud movement
                float cloudDriftOffset;
                if (cloudClockPaused) {
                    cloudDriftOffset = cloudPausedTime.asMilliseconds() * cloudDriftSpeed;
                } else {
                    cloudDriftOffset = (cloudPausedTime + cloudClock.getElapsedTime()).asMilliseconds() * cloudDriftSpeed;
                }
                sf::Rect<int> cloudTextureRect(
                    sf::Vector2i(
                        static_cast<int>(view.getCenter().x * cloudParallaxFactor + cloudDriftOffset),
                        static_cast<int>(view.getCenter().y * cloudParallaxFactor)
                    ),
                    sf::Vector2i(
                        static_cast<int>(view.getSize().x),
                        static_cast<int>(view.getSize().y)
                    )
                );
                cloudShape.setTextureRect(cloudTextureRect);

                window.setView(view); 

                // --- Rendering ---
                window.clear(sf::Color(135, 206, 235));
                window.draw(backgroundShape);
                window.draw(cloudShape);

                // Draw all game objects
                for (size_t i = 0; i < gameObjects.size(); ++i) {
                    if (i != playerIndex) {  // Don't draw player yet
                        gameObjects[i].draw(window);
                    }
                }
                window.setView(window.getDefaultView());
                
                if (timeFreezeOverlayAlpha > 0.0f) {
                    window.draw(timeFreezeOverlay);
                }

                window.setView(view); 

                if (playerIndex != -1) {
                    gameObjects[playerIndex].draw(window);
                }
                window.setView(window.getDefaultView());
                
                if (isTransitioning || transitionAlpha > 0) {
                    window.draw(transitionOverlay);
                }

                window.display();

                // --- Check for Level Completion or Reset ---
                if (levelCompleted || levelReset) {
                    isTransitioning = true;
                    isFadingOut = true;
                }
            }
            // Reset gameObjects for the next level
            gameObjects.clear();
            playerBodyId = b2_nullBodyId;
            playerIndex = -1;
            levelCompleted = false; // Reset for the next level
            
            // Reset time freeze state
            timeFreeze = false;
            wasInTimeFreeze = false;
            frozenBodyData.clear();
            
            // Reset Freeze overlay state
            isTimeFreezeTransitioning = false;
            isTimeFreezeOverlayFadingIn = false;
            isTimeFreezeOverlayFadingOut = false;
            timeFreezeOverlayAlpha = 0.0f;
            timeFreezeOverlay.setFillColor(sf::Color(100, 150, 255, 0)); // Reset to transparent




            // Reset the Box2D world for the next level
            b2DestroyWorld(worldId);
            worldId = b2CreateWorld(&worldDef);
            cloudClock.restart(); // Reset cloud clock for next level

        }
    // --- Cleanup ---
    // Destroy the Box2D world and all bodies/shapes within it.
    if (!B2_IS_NULL(worldId)) {
        b2DestroyWorld(worldId);
        worldId = b2_nullWorldId;
    }

    return 0;
}