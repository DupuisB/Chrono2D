#include <SFML/Graphics.hpp>
#include <box2d/box2d.h> // Main header, includes necessary definitions like B2_ID_EQUALS

#include <vector>
#include <cmath>
#include <iostream>
#include <optional>
#include <numeric>
#include <iomanip>

// --- Constants ---
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;
const float PIXELS_PER_METER = 32.0f;

// --- Conversion Functions ---
// ... (Keep conversion functions) ...
float pixelsToMeters(float pixels) {
    return pixels / PIXELS_PER_METER;
}

float metersToPixels(float meters) {
    return meters * PIXELS_PER_METER;
}

sf::Vector2f b2VecToSfVec(b2Vec2 vec, bool scale = true, bool flipY = true) {
    float sfX = scale ? metersToPixels(vec.x) : vec.x;
    float sfY = scale ? metersToPixels(vec.y) : vec.y;
    if (flipY) {
        return sf::Vector2f(sfX, WINDOW_HEIGHT - sfY);
    } else {
         return sf::Vector2f(sfX, sfY);
    }
}

b2Vec2 sfVecToB2Vec(sf::Vector2f vec, bool scale = true, bool flipY = true) {
    float b2X = scale ? pixelsToMeters(vec.x) : vec.x;
    float b2Y = flipY ? WINDOW_HEIGHT - vec.y : vec.y;
    b2Y = scale ? pixelsToMeters(b2Y) : b2Y;
    return {b2X, b2Y};
}

// --- Helper Function to Create Box Bodies ---
// ... (Keep GameObject struct) ...
struct GameObject {
    b2BodyId bodyId = b2_nullBodyId;
    b2ShapeId shapeId = b2_nullShapeId;
    sf::RectangleShape sfShape;
    bool hasVisual = true;
};

// ... (Keep createAnchorBody) ...
b2BodyId createAnchorBody(b2WorldId worldId, float x, float y) {
     b2BodyDef bodyDef = b2DefaultBodyDef();
     bodyDef.type = b2_staticBody;
     bodyDef.position = {x, y};
     b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
     if (B2_IS_NULL(bodyId)) { std::cerr << "Error creating anchor body!" << std::endl; }
     return bodyId;
}

// ... (Keep createBox) ...
GameObject createBox(b2WorldId worldId, float x, float y, float width, float height, bool isDynamic, sf::Color color) {
     GameObject obj;
    obj.sfShape.setSize({metersToPixels(width), metersToPixels(height)});
    obj.sfShape.setFillColor(color);
    obj.sfShape.setOrigin({metersToPixels(width) / 2.0f, metersToPixels(height) / 2.0f});
    obj.sfShape.setPosition(b2VecToSfVec({x, y}));

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = isDynamic ? b2_dynamicBody : b2_staticBody;
    bodyDef.position = {x, y};
    if (isDynamic) {
         bodyDef.fixedRotation = true;
         bodyDef.linearDamping = 0.1f;
    }
    obj.bodyId = b2CreateBody(worldId, &bodyDef);
    if (B2_IS_NULL(obj.bodyId)) { std::cerr << "Error creating body!" << std::endl; obj.hasVisual = false; return obj; }

    b2Polygon box = b2MakeBox(width / 2.0f, height / 2.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = isDynamic ? 1.0f : 0.0f;
    shapeDef.material.friction = 0.7f;
    shapeDef.material.restitution = 0.1f;

    obj.shapeId = b2CreatePolygonShape(obj.bodyId, &shapeDef, &box);
     if (B2_IS_NULL(obj.shapeId)) { std::cerr << "Error creating shape!" << std::endl; b2DestroyBody(obj.bodyId); obj.bodyId = b2_nullBodyId; obj.hasVisual = false; return obj; }
    return obj;
}

// --- Main Function ---
int main() {
    // ... (Setup is the same) ...
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
    // ... (Ground creation) ...
    float groundWidthM = pixelsToMeters(WINDOW_WIDTH);
    float groundHeightM = pixelsToMeters(50);
    float groundXM = groundWidthM / 2.0f;
    float groundYM = groundHeightM / 2.0f;
    gameObjects.push_back(createBox(worldId, groundXM, groundYM, groundWidthM, groundHeightM, false, sf::Color::Green));
    groundPlatformIds.push_back(gameObjects.back().bodyId);


    // Player
    // ... (Player creation) ...
    float playerWidthM = pixelsToMeters(30);
    float playerHeightM = pixelsToMeters(50);
    float playerXM = pixelsToMeters(100);
    float playerYM = pixelsToMeters(300);
    gameObjects.push_back(createBox(worldId, playerXM, playerYM, playerWidthM, playerHeightM, true, sf::Color::Blue));
    b2BodyId playerBodyId = gameObjects.back().bodyId;


    // Pushable Box
    // ... (Box creation) ...
    float boxSizeM = pixelsToMeters(40);
    float boxXM = pixelsToMeters(400);
    float boxYM = groundYM + groundHeightM / 2.0f + boxSizeM / 2.0f + 0.1f;
    gameObjects.push_back(createBox(worldId, boxXM, boxYM, boxSizeM, boxSizeM, true, sf::Color::Red));
    groundPlatformIds.push_back(gameObjects.back().bodyId);


    // --- Hanging Platform ---
    // ... (Platform and joint creation) ...
     float anchorX = pixelsToMeters(WINDOW_WIDTH * 0.7f);
    float anchorY = pixelsToMeters(WINDOW_HEIGHT * 0.8f);
    b2BodyId anchorBodyId = createAnchorBody(worldId, anchorX, anchorY);

    float platWidthM = pixelsToMeters(120);
    float platHeightM = pixelsToMeters(20);
    float platX = anchorX;
    float platY = anchorY - pixelsToMeters(150);
    gameObjects.push_back(createBox(worldId, platX, platY, platWidthM, platHeightM, true, sf::Color(160, 82, 45)));
    b2BodyId platformBodyId = gameObjects.back().bodyId;
    groundPlatformIds.push_back(platformBodyId);

    b2JointId distanceJointId = b2_nullJointId;
    b2DistanceJointDef distanceDef;

    if (!B2_IS_NULL(anchorBodyId) && !B2_IS_NULL(platformBodyId)) {
        distanceDef = b2DefaultDistanceJointDef();
        distanceDef.bodyIdA = anchorBodyId;
        distanceDef.bodyIdB = platformBodyId;
        distanceDef.localAnchorA = {0.0f, 0.0f};
        distanceDef.localAnchorB = {0.0f, platHeightM / 2.0f};

        b2Vec2 worldAnchorA = b2Body_GetWorldPoint(anchorBodyId, distanceDef.localAnchorA);
        b2Vec2 worldAnchorB = b2Body_GetWorldPoint(platformBodyId, distanceDef.localAnchorB);
        distanceDef.length = b2Distance(worldAnchorA, worldAnchorB);
        distanceDef.minLength = distanceDef.length;
        distanceDef.maxLength = distanceDef.length;

        distanceJointId = b2CreateDistanceJoint(worldId, &distanceDef);
        if(B2_IS_NULL(distanceJointId)) { std::cerr << "Failed to create distance joint!" << std::endl; }
    }


    // --- Game Loop Variables ---
    // ... (Variables are the same) ...
     sf::Clock clock;
    // float timeStep = 1.0f / 60.0f; // dt is now used for world step
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
        dt = std::min(dt, 0.05f);

        // --- SFML 3 Event Handling ---
        jumpKeyJustPressed = false;
        previousJumpKeyHeld = jumpKeyHeld;

        while (std::optional<sf::Event> event = window.pollEvent()) {
             if (event) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
                 // Check specifically for Space key press/release events if needed for other logic
                 if (event->is<sf::Event::KeyPressed>()) {
                     auto keyEvent = event->getIf<sf::Event::KeyPressed>();
                     // *** CHANGE: Check for Space key ***
                     if(keyEvent && keyEvent->code == sf::Keyboard::Key::Space) {
                         // Input state is handled by isKeyPressed below,
                         // but you could add specific single-press logic here if needed.
                     }
                 } else if (event->is<sf::Event::KeyReleased>()) {
                      auto keyEvent = event->getIf<sf::Event::KeyReleased>();
                     // *** CHANGE: Check for Space key ***
                     if(keyEvent && keyEvent->code == sf::Keyboard::Key::Space) {
                         // Jump cut logic uses the isKeyPressed state below.
                     }
                 }
            }
        }
        // -----------------------------

        // --- Input State Update ---
        bool wantsToMoveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        bool wantsToMoveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        // *** CHANGE: Check Space key for jump ***
        jumpKeyHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        // Detect "just pressed" state for Space key
        if (jumpKeyHeld && !previousJumpKeyHeld) {
            jumpKeyJustPressed = true;
        }

        // --- Ground Check ---
        // ... (Ground check logic is the same) ...
         wasGroundedLastFrame = isGrounded;
        isGrounded = false;
        b2ContactData contactData[10];
        int count = b2Body_GetContactData(playerBodyId, contactData, 10);
        for (int i = 0; i < count; ++i) {
             if (contactData[i].manifold.pointCount > 0) {
                 b2ShapeId shapeA = contactData[i].shapeIdA;
                 b2ShapeId shapeB = contactData[i].shapeIdB;
                 b2BodyId bodyA = b2Shape_GetBody(shapeA);
                 b2BodyId bodyB = b2Shape_GetBody(shapeB);
                 b2BodyId otherBodyId = b2_nullBodyId;
                 float normalYDirection = 0.0f;

                 // Determine which body is the player and which is the other
                 if (B2_ID_EQUALS(bodyA, playerBodyId)) {
                     otherBodyId = bodyB;
                     normalYDirection = contactData[i].manifold.normal.y; // Normal points from A(player) to B(other)
                 } else if (B2_ID_EQUALS(bodyB, playerBodyId)) {
                     otherBodyId = bodyA;
                     normalYDirection = -contactData[i].manifold.normal.y; // Normal points from A(other) to B(player), flip for player perspective
                 } else {
                     continue; // Contact doesn't involve the player
                 }

                 // Check if the other body is a ground/platform
                 bool isOtherGround = false;
                 for(const auto& groundId : groundPlatformIds) {
                    if (B2_ID_EQUALS(otherBodyId, groundId)) {
                        isOtherGround = true;
                        break;
                    }
                 }

                 // Check verticality of contact normal (relative to player being on top)
                 if (isOtherGround && normalYDirection > 0.7f) {
                     isGrounded = true;
                     break; // Found ground contact
                 }
            }
        }

        // --- Update Timers ---
        // ... (Timer updates are the same) ...
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

        // bool canJump = isGrounded || (coyoteTimer > 0.0f);
        bool canJump = true;
        bool tryJumpFromBuffer = justLanded && (jumpBufferTimer > 0.0f);

        // *** ADDED DEBUGGING ***
         std::cout << std::fixed << std::setprecision(2)
                   << "Grounded: " << isGrounded
                   << " | Coyote T: " << coyoteTimer
                   << " | Buffer T: " << jumpBufferTimer
                   << " | JustPressed: " << jumpKeyJustPressed
                   << " | CanJump: " << canJump
                   << " | TryBuffer: " << tryJumpFromBuffer
                   << " | isJumping: " << isJumping
                   << std::endl;
        // **********************

        if (tryJumpFromBuffer || (jumpKeyJustPressed && canJump))
        {
            // *** ADDED DEBUGGING ***
            // std::cout << "!!!!!! JUMPING !!!!!!" << std::endl;
            // **********************

            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            b2Body_SetLinearVelocity(playerBodyId, {currentVel.x, 0.0f});
            b2Body_ApplyLinearImpulseToCenter(playerBodyId, {0.0f, jumpImpulseMagnitude}, true);
            isJumping = true;
            jumpBufferTimer = 0.0f;
            coyoteTimer = 0.0f;
            isGrounded = false;
        }

        // --- Jump Height Control ---
        // ... (Jump cut logic is the same) ...
         if (!jumpKeyHeld && isJumping) {
            b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
            if (currentVel.y > 0) {
                b2Body_SetLinearVelocity(playerBodyId, {currentVel.x, currentVel.y * jumpCutMultiplier});
                isJumping = false;
            }
        }


        // --- Horizontal Movement ---
        // ... (Movement logic is the same) ...
        b2Vec2 currentVel = b2Body_GetLinearVelocity(playerBodyId);
        float forceX = 0.0f;
        if (wantsToMoveLeft && currentVel.x > -maxHorizontalSpeed) {
           forceX = -moveForce;
        }
        if (wantsToMoveRight && currentVel.x < maxHorizontalSpeed) {
           forceX = moveForce;
        }
        if(forceX != 0.0f) {
             b2Body_ApplyForceToCenter(playerBodyId, {forceX, 0.0f}, true);
        }


        // --- Box2D Step ---
        b2World_Step(worldId, dt, subSteps);

        // --- Update SFML Graphics ---
        // ... (Graphics update is the same) ...
         for (auto& obj : gameObjects) {
             if (!obj.hasVisual || B2_IS_NULL(obj.bodyId)) continue;
            b2Transform transform = b2Body_GetTransform(obj.bodyId);
            obj.sfShape.setPosition(b2VecToSfVec(transform.p));
            float angleDegrees = -b2Rot_GetAngle(transform.q) * 180.0f / (float)M_PI;
            obj.sfShape.setRotation(sf::degrees(angleDegrees));
        }

        // --- Rendering ---
        window.clear(sf::Color(135, 206, 235));

        // Draw Game Objects
        // ... (Drawing game objects is the same) ...
        for (const auto& obj : gameObjects) {
             if (obj.hasVisual && !B2_IS_NULL(obj.bodyId)) {
                window.draw(obj.sfShape);
             }
        }


        // Draw the "Rope" (Distance Joint Visualization)
        // ... (Drawing the rope is the same) ...
         if (!B2_IS_NULL(distanceJointId)) {
             b2BodyId bodyA = b2Joint_GetBodyA(distanceJointId);
             b2BodyId bodyB = b2Joint_GetBodyB(distanceJointId);
             b2Vec2 worldAnchorA = b2Body_GetWorldPoint(bodyA, distanceDef.localAnchorA);
             b2Vec2 worldAnchorB = b2Body_GetWorldPoint(bodyB, distanceDef.localAnchorB);
             sf::Vector2f sfAnchorA = b2VecToSfVec(worldAnchorA);
             sf::Vector2f sfAnchorB = b2VecToSfVec(worldAnchorB);
             sf::VertexArray ropeLine(sf::PrimitiveType::Lines, 2);
             ropeLine[0].position = sfAnchorA; ropeLine[0].color = sf::Color::Black;
             ropeLine[1].position = sfAnchorB; ropeLine[1].color = sf::Color::Black;
             window.draw(ropeLine);
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