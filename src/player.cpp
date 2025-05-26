#include "../include/player.hpp"
#include "../include/utils.hpp"
#include "../include/constants.hpp"
#include "../include/game_object.hpp" // Required for GameObject class properties
#include <cmath> // For std::abs, std::max, std::sqrt
#include <algorithm> // For std::min, std::max

// Helper function to get the sign of a number
inline float sign(float val) {
    if (val > 0.0f) return 1.0f;
    if (val < 0.0f) return -1.0f;
    return 0.0f;
}

void movePlayer(b2WorldId worldId, b2BodyId playerBodyId, const std::vector<GameObject>& gameObjects,
                bool jumpKeyHeld, bool leftKeyHeld, bool rightKeyHeld, float dt) {

    if (B2_IS_NULL(playerBodyId)) return;

    // --- Player Physics Parameters (Inspired by Game Maker's Toolkit article) ---
    // Horizontal Movement
    static const float PLAYER_MAX_SPEED = 20.0f;
    static const float PLAYER_GROUND_ACCELERATION = 100.0f;
    static const float PLAYER_AIR_ACCELERATION = 60.0f;
    static const float PLAYER_GROUND_DECELERATION = 100.0f;
    static const float PLAYER_TURN_SPEED_FACTOR = 1.5f;

    // Jump
    static const float PLAYER_JUMP_HEIGHT = 2.5f;
    static const float PLAYER_TIME_TO_JUMP_APEX = 0.4f;

    // Gravity Modification
    static const float PLAYER_FALL_GRAVITY_FACTOR = 5.0f;
    static const float PLAYER_JUMP_CUT_GRAVITY_FACTOR = 2.5f;

    // Derived Jump & Gravity Values
    static const float WORLD_GRAVITY_MAGNITUDE = 10.0f;
    static const float PLAYER_EFFECTIVE_GRAVITY_MAGNITUDE = (2.0f * PLAYER_JUMP_HEIGHT) / (PLAYER_TIME_TO_JUMP_APEX * PLAYER_TIME_TO_JUMP_APEX);
    static const float PLAYER_INITIAL_JUMP_VELOCITY = PLAYER_EFFECTIVE_GRAVITY_MAGNITUDE * PLAYER_TIME_TO_JUMP_APEX;
    static const float PLAYER_BASE_GRAVITY_SCALE = PLAYER_EFFECTIVE_GRAVITY_MAGNITUDE / WORLD_GRAVITY_MAGNITUDE;


    // --- Player State Variables (some are persistent across calls) ---
    static bool isGrounded = false;
    static bool wasGroundedLastFrame = false;
    static bool isJumping = false; // True if player is in the upward or controlled downward phase of a jump
    static float coyoteTimer = 0.5f;
    static float jumpBufferTimer = 0.5f;
    static bool previousJumpKeyHeld = false;

    // --- Input Processing ---
    bool jumpKeyJustPressed = jumpKeyHeld && !previousJumpKeyHeld;
    previousJumpKeyHeld = jumpKeyHeld;


    // --- Ground Check ---
    wasGroundedLastFrame = isGrounded;
    isGrounded = false;
    b2ContactData contactData[10];
    int count = b2Body_GetContactData(playerBodyId, contactData, 10);
    for (int i = 0; i < count; ++i) {
        if (contactData[i].manifold.pointCount > 0) {
            b2BodyId bodyA = b2Shape_GetBody(contactData[i].shapeIdA);
            b2BodyId bodyB = b2Shape_GetBody(contactData[i].shapeIdB);
            b2BodyId otherBodyId = b2_nullBodyId;
            float supportingNormalY = 0.0f;

            if (B2_ID_EQUALS(bodyA, playerBodyId)) {
                otherBodyId = bodyB;
                supportingNormalY = -contactData[i].manifold.normal.y;
            } else if (B2_ID_EQUALS(bodyB, playerBodyId)) {
                otherBodyId = bodyA;
                supportingNormalY = contactData[i].manifold.normal.y;
            } else {
                continue;
            }

            // Check if the other body is a GameObject that can be jumped on
            for(const auto& gameObject : gameObjects) {
                if (B2_ID_EQUALS(otherBodyId, gameObject.bodyId)) {
                    if (gameObject.canJumpOn && supportingNormalY > 0.7f) { // Check if contact normal is mostly upward
                        isGrounded = true;
                    }
                    break; // Found the GameObject
                }
            }
            if (isGrounded) break;
        }
    }

    // Update Coyote Time & Jump State
    if (isGrounded) {
        coyoteTimer = 0.1f; // coyoteTimeThreshold
        if (isJumping) { // Landed
             isJumping = false;
        }
    } else {
        coyoteTimer = std::max(0.0f, coyoteTimer - dt);
    }

    // Update Jump Buffer
    if (jumpKeyJustPressed) {
        jumpBufferTimer = 0.1f; // jumpBufferThreshold
    } else {
        jumpBufferTimer = std::max(0.0f, jumpBufferTimer - dt);
    }
    
    bool justLanded = isGrounded && !wasGroundedLastFrame;

    // --- Handle Jumping ---
    bool canJumpFromState = (isGrounded || coyoteTimer > 0.0f);
    bool tryJumpFromBuffer = justLanded && (jumpBufferTimer > 0.0f);

    if (tryJumpFromBuffer || (jumpKeyJustPressed && canJumpFromState)) {
        b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
        b2Body_SetLinearVelocity(playerBodyId, {currentVel.x, PLAYER_INITIAL_JUMP_VELOCITY});
        isJumping = true;
        jumpBufferTimer = 0.0f; // Consume buffer
        coyoteTimer = 0.0f;     // Consume coyote time
        isGrounded = false;     // No longer grounded once jump starts
    }

    // --- Gravity Modification ---
    b2Vec2 playerVel = b2Body_GetLinearVelocity(playerBodyId);
    float currentGravityScale = PLAYER_BASE_GRAVITY_SCALE;

    if (isJumping && playerVel.y > 0.01f && !jumpKeyHeld) { // Jump cut: key released while moving up
        currentGravityScale = PLAYER_BASE_GRAVITY_SCALE * PLAYER_JUMP_CUT_GRAVITY_FACTOR;
    } else if (playerVel.y < -0.01f) { // Falling
        currentGravityScale = PLAYER_BASE_GRAVITY_SCALE * PLAYER_FALL_GRAVITY_FACTOR;
    }
    // If on ground and not jumping, or at apex, PLAYER_BASE_GRAVITY_SCALE applies.
    // If isJumping is false (e.g. walked off ledge), fall gravity applies.
    if (isGrounded && !isJumping) { // Ensure normal gravity when standing on ground
         currentGravityScale = PLAYER_BASE_GRAVITY_SCALE; // Or 1.0f if PLAYER_BASE_GRAVITY_SCALE is only for jump arc
                                                          // Let's stick to PLAYER_BASE_GRAVITY_SCALE for consistency
    }


    b2Body_SetGravityScale(playerBodyId, currentGravityScale);


    // --- Horizontal Movement ---
    float forceX = 0.0f;
    float currentVelX = playerVel.x;
    float playerMass = b2Body_GetMass(playerBodyId);

    if (leftKeyHeld || rightKeyHeld) {
        float direction = leftKeyHeld ? -1.0f : 1.0f;
        float accelRate = isGrounded ? PLAYER_GROUND_ACCELERATION : PLAYER_AIR_ACCELERATION;

        // Apply turn speed factor if changing direction
        if (sign(currentVelX) != 0 && sign(currentVelX) != direction) {
            accelRate *= PLAYER_TURN_SPEED_FACTOR;
        }

        // Only apply force if not exceeding max speed in the direction of movement,
        // or if trying to move against current velocity (turning/slowing down towards opposite max speed)
        if ((direction > 0 && currentVelX < PLAYER_MAX_SPEED) || (direction < 0 && currentVelX > -PLAYER_MAX_SPEED)) {
             forceX = direction * accelRate * playerMass;
        }
        // If at max speed and still pressing in that direction, no acceleration force.
        // If trying to accelerate further but already at max speed, Box2D friction/damping will handle it.

    } else { // No horizontal input: apply deceleration if on ground
        if (isGrounded && std::abs(currentVelX) > 0.01f) {
            float decelForceMagnitude = PLAYER_GROUND_DECELERATION * playerMass;
            forceX = -sign(currentVelX) * decelForceMagnitude;

            // Prevent deceleration from overshooting and reversing direction in a single frame
            if (std::abs(forceX * dt / playerMass) > std::abs(currentVelX)) {
                forceX = -currentVelX * playerMass / dt; // Force to stop in one frame
            }
        }
    }

    if (forceX != 0.0f) {
        b2Body_ApplyForceToCenter(playerBodyId, {forceX, 0.0f}, true);
    }
    
    // Final velocity clamping (optional, forces should ideally respect max speed, but this is a hard clamp)
    // playerVel = b2Body_GetLinearVelocity(playerBodyId); // Get updated velocity if forces applied
    // float clampedVelX = std::max(-PLAYER_MAX_SPEED, std::min(playerVel.x, PLAYER_MAX_SPEED));
    // if (std::abs(playerVel.x - clampedVelX) > 0.01f) { // Only set if different to avoid waking body
    //    b2Body_SetLinearVelocity(playerBodyId, {clampedVelX, playerVel.y});
    // }
    // For now, let forces and damping manage speed, avoid direct velocity setting for horizontal if possible.
}
