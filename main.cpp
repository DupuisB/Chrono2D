#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "include/game_object.hpp" // Includes utils.hpp and constants.hpp

// --- Map Loading ---
// Conditional compilation to include the selected map file.
#if SELECTED_MAP == 1
#include "maps/map1.hpp"
#else
// Placeholder for other maps or a default map.
// #include "maps/default_map.hpp"
#pragma message("Warning: No map selected or SELECTED_MAP value not recognized. Defaulting to no map objects.")
#endif
// --- End Map Loading ---

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
    std::vector<b2BodyId> groundPlatformIds; // IDs of bodies considered as ground for contact checks
    b2BodyId playerBodyId = b2_nullBodyId; // ID of the player's body

    // --- Load Map Objects ---
    // Populates gameObjects, playerBodyId, and groundPlatformIds based on the selected map.
#if SELECTED_MAP == 1
    loadMap1(worldId, gameObjects, playerBodyId, groundPlatformIds);
#else
    // loadDefaultMap(worldId, gameObjects, playerBodyId, groundPlatformIds); // Example for a default map
#endif
    // --- End Load Map Objects ---

    // --- Game Loop Variables ---
    sf::Clock clock;        // Measures time between frames
    int32_t subSteps = 8;   // Number of physics sub-steps per frame for stability

    // Player movement parameters
    float moveForce = 50.0f;            // Force applied for horizontal movement
    float jumpImpulseMagnitude = 25.0f; // Initial impulse for jumping
    float maxHorizontalSpeed = 5.0f;    // Maximum horizontal speed for the player
    float jumpCutMultiplier = 0.5f;     // Multiplier to reduce jump height if jump key is released early

    // Player state variables
    bool isGrounded = false;            // True if the player is currently touching the ground
    bool wasGroundedLastFrame = false;  // Grounded state in the previous frame
    bool jumpKeyHeld = false;           // True if the jump key is currently pressed
    bool jumpKeyJustPressed = false;    // True for one frame when the jump key is initially pressed
    bool previousJumpKeyHeld = false;   // Jump key state in the previous frame
    bool isJumping = false;             // True if the player is in the upward phase of a jump

    // Advanced jump mechanics timers
    const float coyoteTimeThreshold = 0.1f; // Allows jumping shortly after leaving a platform
    float coyoteTimer = 0.0f;
    const float jumpBufferThreshold = 0.1f; // Allows a jump to register if key pressed shortly before landing
    float jumpBufferTimer = 0.0f;

    // --- Main Game Loop ---
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f); // Cap delta time to prevent physics instability with large steps

        // --- SFML Event Handling ---
        jumpKeyJustPressed = false; // Reset per frame
        previousJumpKeyHeld = jumpKeyHeld;

        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
                // Key press/release events can be handled here if needed for single-trigger actions,
                // but continuous actions (like holding jump) are better handled by sf::Keyboard::isKeyPressed.
            }
        }

        // --- Input State Update ---
        // Check current state of movement and jump keys
        bool wantsToMoveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        bool wantsToMoveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        jumpKeyHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        if (jumpKeyHeld && !previousJumpKeyHeld) {
            jumpKeyJustPressed = true; // Detects the frame the jump key was pressed
        }

        // --- Ground Check ---
        // Determines if the player is currently on a "ground" surface.
        wasGroundedLastFrame = isGrounded;
        isGrounded = false;
        if (!B2_IS_NULL(playerBodyId)) {
            b2ContactData contactData[10]; // Buffer for contact data
            int count = b2Body_GetContactData(playerBodyId, contactData, 10);
            for (int i = 0; i < count; ++i) {
                if (contactData[i].manifold.pointCount > 0) {
                    b2BodyId bodyA = b2Shape_GetBody(contactData[i].shapeIdA);
                    b2BodyId bodyB = b2Shape_GetBody(contactData[i].shapeIdB);
                    b2BodyId otherBodyId = b2_nullBodyId;
                    float supportingNormalY = 0.0f; // Y-component of contact normal from other object towards player

                    if (B2_ID_EQUALS(bodyA, playerBodyId)) {
                        otherBodyId = bodyB;
                        // Normal points from player (A) to other (B). Flip for supporting normal.
                        supportingNormalY = -contactData[i].manifold.normal.y;
                    } else if (B2_ID_EQUALS(bodyB, playerBodyId)) {
                        otherBodyId = bodyA;
                        // Normal points from other (A) to player (B). This is the supporting normal.
                        supportingNormalY = contactData[i].manifold.normal.y;
                    } else {
                        continue; // Contact does not involve the player
                    }

                    // Check if the other body is in the list of ground platforms
                    bool isOtherGround = false;
                    for(const auto& groundId : groundPlatformIds) {
                        if (B2_ID_EQUALS(otherBodyId, groundId)) {
                            isOtherGround = true;
                            break;
                        }
                    }

                    // Player is grounded if contact normal is mostly upward (supporting)
                    if (isOtherGround && supportingNormalY > 0.7f) {
                        isGrounded = true;
                        break; 
                    }
                }
            }
        }

        // Update Coyote Time and Jump Buffer
        if (isGrounded) {
            coyoteTimer = coyoteTimeThreshold; // Reset coyote time when grounded
            isJumping = false;                 // Reset jumping state
        } else {
            coyoteTimer = std::max(0.0f, coyoteTimer - dt); // Decrease coyote timer when airborne
        }
        jumpBufferTimer = std::max(0.0f, jumpBufferTimer - dt); // Decrease jump buffer timer

        bool justLanded = isGrounded && !wasGroundedLastFrame;

        // --- Handle Jumping ---
        if (jumpKeyJustPressed) {
            jumpBufferTimer = jumpBufferThreshold; // Activate jump buffer if jump key pressed
        }

        // Conditions for allowing a jump:
        // 1. Player is grounded OR coyote time is active.
        // 2. Jump key was just pressed OR jump buffer is active and player just landed.
        bool canJumpFromState = (isGrounded || coyoteTimer > 0.0f);
        bool tryJumpFromBuffer = justLanded && (jumpBufferTimer > 0.0f);

        if (!B2_IS_NULL(playerBodyId) && (tryJumpFromBuffer || (jumpKeyJustPressed && canJumpFromState)))
        {
            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            b2Body_SetLinearVelocity(playerBodyId, {currentVel.x, 0.0f}); // Reset vertical velocity for consistent jump height
            b2Body_ApplyLinearImpulseToCenter(playerBodyId, {0.0f, jumpImpulseMagnitude}, true);
            isJumping = true;       // Player is now in the jumping state
            jumpBufferTimer = 0.0f; // Consume jump buffer
            coyoteTimer = 0.0f;     // Consume coyote time
            isGrounded = false;     // Player is no longer grounded after initiating jump
        }

        // --- Jump Height Control (Early Release) ---
        // If jump key is released while moving upwards, reduce upward velocity.
        if (!B2_IS_NULL(playerBodyId) && !jumpKeyHeld && isJumping) {
            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            if (currentVel.y > 0) { // Only cut jump if moving upwards
                b2Body_SetLinearVelocity(playerBodyId, {currentVel.x, currentVel.y * jumpCutMultiplier});
            }
            isJumping = false; // End the jump phase
        }

        // --- Horizontal Movement ---
        if (!B2_IS_NULL(playerBodyId)) {
            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            float forceX = 0.0f;

            // Apply force if moving left and not exceeding max speed
            if (wantsToMoveLeft && currentVel.x > -maxHorizontalSpeed) {
                forceX = -moveForce;
            }
            // Apply force if moving right and not exceeding max speed
            if (wantsToMoveRight && currentVel.x < maxHorizontalSpeed) {
                forceX = moveForce;
            }
            
            if(forceX != 0.0f) {
                b2Body_ApplyForceToCenter(playerBodyId, {forceX, 0.0f}, true);
            }
        }

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