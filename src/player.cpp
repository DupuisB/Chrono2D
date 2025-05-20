#include "../include/player.hpp"
#include "../include/utils.hpp"
#include "../include/constants.hpp"

void movePlayer(b2WorldId worldId, b2BodyId playerBodyId, std::vector<b2BodyId>& groundPlatformIds,
                bool jumpKeyHeld, bool leftKeyHeld, bool rightKeyHeld, float dt) {
    // --- Player Movement Variables ---
    static const float moveForce = 70.0f; // Force applied for horizontal movement
    static const float maxHorizontalSpeed = 5.0f; // Maximum horizontal speed
    static const float jumpImpulseMagnitude = 25.0f; // Impulse magnitude for jumping
    static const float jumpCutMultiplier = 0.5f; // Multiplier to reduce upward velocity when jump key is released
    static const float coyoteTimeThreshold = 0.1f; // Time allowed to jump after leaving a platform
    static const float jumpBufferThreshold = 0.1f; // Time allowed to jump after pressing the jump key

    // --- Player State Variables ---
    static bool isGrounded = false; // Indicates if the player is on the ground
    static bool wasGroundedLastFrame = false; // Indicates if the player was grounded in the last frame
    static bool isJumping = false; // Indicates if the player is currently jumping
    static float coyoteTimer = 0.0f; // Timer for coyote time
    static float jumpBufferTimer = 0.0f; // Timer for jump buffer

    // --- Input Handling ---
    bool wantsToMoveLeft = leftKeyHeld;
    bool wantsToMoveRight = rightKeyHeld;
    
    // Detect if the jump key was just pressed
    static bool previousJumpKeyHeld = false;
    bool jumpKeyJustPressed = false;

    if (jumpKeyHeld && !previousJumpKeyHeld) {
        jumpKeyJustPressed = true; // Detects the frame the jump key was pressed
    }
    
    if (jumpKeyHeld) {
        previousJumpKeyHeld = true;
    } else {
        previousJumpKeyHeld = false;
        jumpKeyJustPressed = false; // Reset on release
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
                if (supportingNormalY > 0.7f) {
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
}
