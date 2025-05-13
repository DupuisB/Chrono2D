#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "include/game_object.hpp" // Includes utils.hpp and constants.hpp

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

    b2Vec2 gravity = {0.0f, -10.0f};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    b2WorldId worldId = b2CreateWorld(&worldDef);
    if (B2_IS_NULL(worldId)) return -1;

    std::vector<GameObject> gameObjects;
    std::vector<b2BodyId> groundPlatformIds;

    // Ground
    float groundWidthM = pixelsToMeters(WINDOW_WIDTH);
    float groundHeightM = pixelsToMeters(50);
    float groundXM = groundWidthM / 2.0f;
    float groundYM = groundHeightM / 2.0f;
    GameObject ground;
    if (ground.init(worldId, groundXM, groundYM, groundWidthM, groundHeightM, false, sf::Color::Green)) {
        gameObjects.push_back(ground);
        groundPlatformIds.push_back(ground.bodyId);
    }

    // Player
    float playerWidthM = pixelsToMeters(30);
    float playerHeightM = pixelsToMeters(50);
    float playerXM = pixelsToMeters(100);
    float playerYM = pixelsToMeters(300); // Adjusted for typical platformer start
    GameObject playerObject;
    b2BodyId playerBodyId = b2_nullBodyId; // Initialize to null
    // For player: fixedRotation = true, linearDamping = 0.1f
    if (playerObject.init(worldId, playerXM, playerYM, playerWidthM, playerHeightM, true, sf::Color::Blue, true, 0.1f)) {
        gameObjects.push_back(playerObject);
        playerBodyId = playerObject.bodyId; // Assign here after successful init
    }

    // Pushable Box
    float boxSizeM = pixelsToMeters(40);
    float boxXM = pixelsToMeters(400);
    float boxYM = groundYM + groundHeightM / 2.0f + boxSizeM / 2.0f + pixelsToMeters(1); // Ensure it's slightly above ground
    GameObject pushableBox;
    // For pushable box: default fixedRotation=false, linearDamping=0.0f (can be adjusted if needed)
    // Let's give it some linear damping too
    if (pushableBox.init(worldId, boxXM, boxYM, boxSizeM, boxSizeM, true, sf::Color::Red, false, 0.2f)) {
        gameObjects.push_back(pushableBox);
        groundPlatformIds.push_back(pushableBox.bodyId);
    }

    // --- Hanging Platform ---
    float anchorX_m = pixelsToMeters(WINDOW_WIDTH * 0.7f);
    float anchorY_m = pixelsToMeters(WINDOW_HEIGHT * 0.8f);
    b2BodyId anchorBodyId = createAnchorBody(worldId, anchorX_m, anchorY_m);

    float platWidthM = pixelsToMeters(120);
    float platHeightM = pixelsToMeters(20);
    float platX_m = anchorX_m;
    float platY_m = anchorY_m - pixelsToMeters(150);
    GameObject hangingPlatform;
    // For hanging platform: dynamic, but might want fixedRotation or specific damping
    if (hangingPlatform.init(worldId, platX_m, platY_m, platWidthM, platHeightM, true, sf::Color(160, 82, 45), false, 0.5f)) {
        gameObjects.push_back(hangingPlatform);
        groundPlatformIds.push_back(hangingPlatform.bodyId);
    }
    b2BodyId platformBodyId = gameObjects.back().bodyId; // Assuming platform is last added
    // Show default mass data
    std::cout << "Default mass data: " << b2Body_GetMass(platformBodyId) << std::endl;
    std::cout << "Default inertia: " << b2Body_GetRotationalInertia(platformBodyId) << std::endl;

    b2MassData myMassData;
    myMassData.mass = 2.34375f;
    myMassData.center = (b2Vec2){0.0f, 0.0f};
    myMassData.rotationalInertia = 5.0f;
    b2Body_SetMassData(platformBodyId, myMassData);

    // --- Create Segmented Rope ---
    const int numRopeSegments = 10;
    std::vector<b2BodyId> ropeSegmentBodyIds;
    ropeSegmentBodyIds.reserve(numRopeSegments);

    if (!B2_IS_NULL(anchorBodyId) && !B2_IS_NULL(platformBodyId)) {
        b2Vec2 anchorPos = b2Body_GetPosition(anchorBodyId); // World position of the static anchor
        b2Vec2 platformAttachPointLocal = {0.0f, platHeightM / 2.0f};
        b2Vec2 platformAttachPointWorld = b2Body_GetWorldPoint(platformBodyId, platformAttachPointLocal);

        float desiredTotalRopeLength = b2Distance(anchorPos, platformAttachPointWorld);
        float ropeSegmentLengthM = desiredTotalRopeLength / numRopeSegments;
        float segmentThicknessM = pixelsToMeters(8); // Thickness of each rope segment

        b2BodyId prevBodyId = anchorBodyId;
        b2Vec2 prevBodyAnchorLocal = {0.0f, 0.0f}; // Anchor point on the static anchor body

        for (int i = 0; i < numRopeSegments; ++i) {
            GameObject segmentObj;
            // Calculate initial position for the segment (hanging straight down)
            float segmentCenterX = anchorPos.x;
            float segmentCenterY = anchorPos.y - (i * ropeSegmentLengthM) - (ropeSegmentLengthM / 2.0f);

            // Create the rope segment GameObject
            // Properties: light, flexible, some damping
            if (segmentObj.init(worldId, segmentCenterX, segmentCenterY,
                                segmentThicknessM, ropeSegmentLengthM, // width (thickness), height (length)
                                true, sf::Color(139, 69, 19),      // Dynamic, Brown color
                                false, 0.2f, 0.05f, 0.5f, 0.1f)) { // fixedRotation=false, linearDamping, density, friction, restitution
                gameObjects.push_back(segmentObj);
                ropeSegmentBodyIds.push_back(segmentObj.bodyId);

                // Define the revolute joint
                b2RevoluteJointDef revoluteDef = b2DefaultRevoluteJointDef();
                revoluteDef.bodyIdA = prevBodyId;
                revoluteDef.bodyIdB = segmentObj.bodyId;
                revoluteDef.localAnchorA = prevBodyAnchorLocal;
                revoluteDef.localAnchorB = {0.0f, ropeSegmentLengthM / 2.0f}; // Top of the current segment
                revoluteDef.collideConnected = false;

                b2CreateRevoluteJoint(worldId, &revoluteDef);

                prevBodyId = segmentObj.bodyId;
                prevBodyAnchorLocal = {0.0f, -ropeSegmentLengthM / 2.0f}; // Bottom of the current segment for the next joint
            } else {
                std::cerr << "Failed to create rope segment " << i << std::endl;
                // Handle error, maybe break or skip this segment
            }
        }

        // Connect the last rope segment to the platform
        if (!ropeSegmentBodyIds.empty()) {
            b2RevoluteJointDef revoluteDef = b2DefaultRevoluteJointDef();
            revoluteDef.bodyIdA = ropeSegmentBodyIds.back(); // Last segment of the rope
            revoluteDef.bodyIdB = platformBodyId;
            revoluteDef.localAnchorA = {0.0f, -ropeSegmentLengthM / 2.0f}; // Bottom of the last rope segment
            revoluteDef.localAnchorB = platformAttachPointLocal;          // Attachment point on the platform
            revoluteDef.collideConnected = false;
            b2CreateRevoluteJoint(worldId, &revoluteDef);
        }
    }
    // --- End Segmented Rope Creation ---
    
    // Create a horizontal rope between two new anchors
    const int numHSegments = 20;
    b2Vec2 leftAnchorPos  = { pixelsToMeters(100), pixelsToMeters(200) };
    b2Vec2 rightAnchorPos = { pixelsToMeters(800), pixelsToMeters(200) };

    // Create two static anchor bodies
    b2BodyId leftAnchor  = createAnchorBody(worldId, leftAnchorPos.x,  leftAnchorPos.y);
    b2BodyId rightAnchor = createAnchorBody(worldId, rightAnchorPos.x, rightAnchorPos.y);

    std::vector<b2BodyId> hRopeSegIds;
    hRopeSegIds.reserve(numHSegments);

    float totalLength   = b2Distance(leftAnchorPos, rightAnchorPos);
    float segmentLength = totalLength / numHSegments;
    float thickness     = pixelsToMeters(3);

    b2BodyId prevBody    = leftAnchor;
    b2Vec2   prevLocalA  = {0.0f, 0.0f};

    for (int i = 0; i < numHSegments; ++i) {
        // Interpolate segment center along the line
        float t = (i + 0.5f) / numHSegments;
        b2Vec2 center = {
            leftAnchorPos.x + t * (rightAnchorPos.x - leftAnchorPos.x),
            leftAnchorPos.y + t * (rightAnchorPos.y - leftAnchorPos.y)
        };

        GameObject segObj;
        if (segObj.init(
                worldId,
                center.x, center.y,
                segmentLength, thickness,
                true,               // dynamic
                sf::Color::Yellow,
                false,              // fixedRotation
                1.0f,               // linearDamping
                1.0f,               // density
                0.0f,               // friction
                1.0f                // restitution
            ))
        {
            gameObjects.push_back(segObj);
            b2BodyId curBody = segObj.bodyId;
            hRopeSegIds.push_back(curBody);

            // Create revolute joint between prevBody and this segment
            b2RevoluteJointDef jd = b2DefaultRevoluteJointDef();
            jd.bodyIdA        = prevBody;
            jd.bodyIdB        = curBody;
            jd.localAnchorA   = prevLocalA;
            jd.localAnchorB   = { -segmentLength * 0.5f, 0.0f };
            jd.collideConnected = false;
            b2CreateRevoluteJoint(worldId, &jd);

            // Prepare for next iteration
            prevBody   = curBody;
            prevLocalA = { segmentLength * 0.5f, 0.0f };
        }
    }

    // Connect the last segment to the right anchor
    if (!hRopeSegIds.empty()) {
        b2RevoluteJointDef jd = b2DefaultRevoluteJointDef();
        jd.bodyIdA        = hRopeSegIds.back();
        jd.bodyIdB        = rightAnchor;
        jd.localAnchorA   = { segmentLength * 0.5f, 0.0f };
        jd.localAnchorB   = { 0.0f, 0.0f };
        jd.collideConnected = false;
        b2CreateRevoluteJoint(worldId, &jd);
    }
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