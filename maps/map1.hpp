#ifndef MAP1_HPP
#define MAP1_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "../include/game_object.hpp" // Includes utils.hpp and constants.hpp
#include <vector>
#include <iostream> // For std::cout, std::cerr
#include <cmath>    // For b2Distance, M_PI / b2_pi

// Function to load map 1 elements
inline void loadMap1(b2WorldId worldId,
                     std::vector<GameObject>& gameObjects,
                     b2BodyId& playerBodyId, // Pass by reference to update player's body ID
                     std::vector<b2BodyId>& groundPlatformIds) { // Pass by reference to populate

    // Clear existing potentially, or assume they are empty
    // gameObjects.clear();
    // groundPlatformIds.clear();
    playerBodyId = b2_nullBodyId;


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
}

#endif // MAP1_HPP
