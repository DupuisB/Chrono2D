#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "include/game_object.hpp" // Includes utils.hpp and constants.hpp

// --- Map Loading ---
#if SELECTED_MAP == 1
#include "maps/map1.hpp"
#else
// You can add other map includes here or a default
// #include "maps/default_map.hpp"
#pragma message("Warning: No map selected or SELECTED_MAP value not recognized. Defaulting to no map objects.")
#endif
// --- End Map Loading ---

#include <vector>
#include <cmath> // For M_PI / b2_pi
#include <iostream>
#include <optional>
#include <numeric>
#include <iomanip>

// --- Main Function ---
int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "SFML Box2D Platformer");
    window.setFramerateLimit(60);

    // Add camera view
    sf::View view = window.getDefaultView();

    b2Vec2 gravity = {0.0f, -10.0f};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    b2WorldId worldId = b2CreateWorld(&worldDef);
    if (B2_IS_NULL(worldId)) return -1;

    std::vector<GameObject> gameObjects;
    std::vector<b2BodyId> groundPlatformIds;
    b2BodyId playerBodyId = b2_nullBodyId; // Initialize to null

    // --- Load Map Objects ---
#if SELECTED_MAP == 1
    loadMap1(worldId, gameObjects, playerBodyId, groundPlatformIds);
#else
    // Call other map loading functions or do nothing
    // loadDefaultMap(worldId, gameObjects, playerBodyId, groundPlatformIds);
#endif
    // --- End Load Map Objects ---

    // --- Game Loop Variables ---
    sf::Clock clock;
    int32_t subSteps = 8;

    float moveForce = 50.0f;
    float jumpImpulseMagnitude = 25.0f;
    float maxHorizontalSpeed = 5.0f;
    float jumpCutMultiplier = 0.5f;

    bool isGrounded = false;
    bool wasGroundedLastFrame = false;
    bool jumpKeyHeld = false;
    bool jumpKeyJustPressed = false;
    bool previousJumpKeyHeld = false;
    bool isJumping = false;

    const float coyoteTimeThreshold = 0.1f;
    float coyoteTimer = 0.0f;
    const float jumpBufferThreshold = 0.1f;
    float jumpBufferTimer = 0.0f;

    // --- Game Loop ---
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f); // Cap dt to prevent large steps

        // --- SFML Event Handling ---
        jumpKeyJustPressed = false;
        previousJumpKeyHeld = jumpKeyHeld;

        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
                if (event->is<sf::Event::KeyPressed>()) {
                    auto keyEvent = event->getIf<sf::Event::KeyPressed>();
                    if(keyEvent && keyEvent->code == sf::Keyboard::Key::Space) {
                    }
                } else if (event->is<sf::Event::KeyReleased>()) {
                    auto keyEvent = event->getIf<sf::Event::KeyReleased>();
                    if(keyEvent && keyEvent->code == sf::Keyboard::Key::Space) {
                    }
                }
            }
        }

        // --- Input State Update ---
        bool wantsToMoveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        bool wantsToMoveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        jumpKeyHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        if (jumpKeyHeld && !previousJumpKeyHeld) {
            jumpKeyJustPressed = true;
        }

        // --- Ground Check ---
        wasGroundedLastFrame = isGrounded;
        isGrounded = false;
        if (!B2_IS_NULL(playerBodyId)) { // Ensure player body is valid
            b2ContactData contactData[10];
            int count = b2Body_GetContactData(playerBodyId, contactData, 10);
            for (int i = 0; i < count; ++i) {
                if (contactData[i].manifold.pointCount > 0) {
                    b2ShapeId shapeA = contactData[i].shapeIdA;
                    b2ShapeId shapeB = contactData[i].shapeIdB;
                    b2BodyId bodyA = b2Shape_GetBody(shapeA);
                    b2BodyId bodyB = b2Shape_GetBody(shapeB);
                    b2BodyId otherBodyId = b2_nullBodyId;
                    float supportingNormalY = 0.0f; // Y-component of normal from other object towards player

                    if (B2_ID_EQUALS(bodyA, playerBodyId)) {
                        otherBodyId = bodyB;
                        // Normal points from player (A) to other (B).
                        // We want normal from other to player, so flip it.
                        supportingNormalY = -contactData[i].manifold.normal.y;
                    } else if (B2_ID_EQUALS(bodyB, playerBodyId)) {
                        otherBodyId = bodyA;
                        // Normal points from other (A) to player (B). This is what we need.
                        supportingNormalY = contactData[i].manifold.normal.y;
                    } else {
                        continue; // Contact doesn't involve the player
                    }

                    bool isOtherGround = false;
                    for(const auto& groundId : groundPlatformIds) {
                        if (B2_ID_EQUALS(otherBodyId, groundId)) {
                            isOtherGround = true;
                            break;
                        }
                    }

                    // If the other body is ground and the normal from it to the player is mostly upward
                    if (supportingNormalY > 0.7f) {
                        isGrounded = true;
                        break; // Found ground contact
                    }
                }
            }
        }

        if (isGrounded) {
            coyoteTimer = coyoteTimeThreshold;
            isJumping = false; // Ensure jumping flag is reset when grounded
        } else {
            coyoteTimer = std::max(0.0f, coyoteTimer - dt);
        }
        jumpBufferTimer = std::max(0.0f, jumpBufferTimer - dt);
        bool justLanded = isGrounded && !wasGroundedLastFrame;

        // --- Handle Jumping ---
        if (jumpKeyJustPressed) {
            jumpBufferTimer = jumpBufferThreshold;
        }

        bool canJump = (isGrounded || coyoteTimer > 0.0f); // Corrected logic for when jump is allowed
        bool tryJumpFromBuffer = justLanded && (jumpBufferTimer > 0.0f);

        if (!B2_IS_NULL(playerBodyId) && (tryJumpFromBuffer || (jumpKeyJustPressed && canJump)))
        {
            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            b2Body_SetLinearVelocity(playerBodyId, {currentVel.x, 0.0f}); // Reset vertical velocity before jump
            b2Body_ApplyLinearImpulseToCenter(playerBodyId, {0.0f, jumpImpulseMagnitude}, true);
            isJumping = true;
            jumpBufferTimer = 0.0f; // Consume buffer
            coyoteTimer = 0.0f;     // Consume coyote time
            isGrounded = false;     // No longer grounded after initiating jump
        }

        // --- Jump Height Control ---
        if (!B2_IS_NULL(playerBodyId) && !jumpKeyHeld && isJumping) {
            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            if (currentVel.y > 0) { // Only cut jump if moving upwards
                b2Body_SetLinearVelocity(playerBodyId, {currentVel.x, currentVel.y * jumpCutMultiplier});
            }
            isJumping = false; // Stop jump phase even if velocity wasn't positive, to prevent re-application
        }

        // --- Horizontal Movement ---
        if (!B2_IS_NULL(playerBodyId)) {
            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            float forceX = 0.0f;
            if (wantsToMoveLeft && currentVel.x > -maxHorizontalSpeed) {
                forceX = -moveForce;
            }
            if (wantsToMoveRight && currentVel.x < maxHorizontalSpeed) {
                forceX = moveForce;
            }
            // Apply force only if there's a desired movement and not exceeding max speed in that direction
            if(forceX != 0.0f) {
                b2Body_ApplyForceToCenter(playerBodyId, {forceX, 0.0f}, true);
            }
        }

        // --- Box2D Step ---
        b2World_Step(worldId, dt, subSteps);

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
        window.clear(sf::Color(135, 206, 235)); // Cornflower blue

        // Draw Game Objects
        for (const auto& obj : gameObjects) {
            obj.draw(window);
        }

        window.display();
    }

    // --- Cleanup ---
    if (!B2_IS_NULL(worldId)) {
        b2DestroyWorld(worldId);
        worldId = b2_nullWorldId;
    }

    return 0;
}