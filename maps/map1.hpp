#ifndef MAP1_HPP
#define MAP1_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "../include/game_object.hpp" // Includes utils.hpp and constants.hpp
#include <vector>
#include <iostream> // For std::cout, std::cerr
#include <cmath>    // For b2Distance, M_PI / b2_pi

/**
 * @brief Loads the game objects for Map 1 into the world.
 * This includes the ground, player, a pushable box, a hanging platform with a rope,
 * and a horizontal rope bridge.
 * @param worldId The ID of the Box2D world.
 * @param gameObjects A reference to the vector that will store all created GameObjects.
 * @param playerBodyId A reference to store the b2BodyId of the created player object.
 * @param groundPlatformIds A reference to the vector that will store the b2BodyIds of objects considered as ground.
 */
inline void loadMap1(b2WorldId worldId,
                     std::vector<GameObject>& gameObjects,
                     b2BodyId& playerBodyId, // Pass by reference to update player's body ID
                     std::vector<b2BodyId>& groundPlatformIds) { // Pass by reference to populate

    // Ensure playerBodyId is initialized to null before attempting to create the player.
    playerBodyId = b2_nullBodyId;

    // Ground
    float groundWidthM = pixelsToMeters(WINDOW_WIDTH);
    float groundHeightM = pixelsToMeters(50);
    float groundXM = groundWidthM / 2.0f;
    float groundYM = groundHeightM / 2.0f;
    GameObject ground;
    if (ground.init(worldId, groundXM, groundYM, groundWidthM, groundHeightM, false, sf::Color::Green)) { // Static body
        gameObjects.push_back(ground);
        groundPlatformIds.push_back(ground.bodyId);
    }

    // Player
    float playerWidthM = pixelsToMeters(30);
    float playerHeightM = pixelsToMeters(50);
    float playerXM = pixelsToMeters(100);
    float playerYM = pixelsToMeters(300); // Adjusted for typical platformer start
    GameObject playerObject;
    // Player: dynamic, fixed rotation, some linear damping
    if (playerObject.init(worldId, playerXM, playerYM, playerWidthM, playerHeightM, true, sf::Color::Blue, true, 0.1f)) {
        gameObjects.push_back(playerObject);
        playerBodyId = playerObject.bodyId; // Assign here after successful init
    }

    // Pushable Box
    float boxSizeM = pixelsToMeters(40);
    float boxXM = pixelsToMeters(400);
    float boxYM = groundYM + groundHeightM / 2.0f + boxSizeM / 2.0f + pixelsToMeters(1); // Position slightly above ground
    GameObject pushableBox;
    // Pushable box: dynamic, allows rotation, some linear damping
    if (pushableBox.init(worldId, boxXM, boxYM, boxSizeM, boxSizeM, true, sf::Color::Red, false, 0.2f)) {
        gameObjects.push_back(pushableBox);
        groundPlatformIds.push_back(pushableBox.bodyId); // Pushable box can act as a platform
    }

    // --- Hanging Platform ---
    float anchorX_m = pixelsToMeters(1500);
    float anchorY_m = pixelsToMeters(500); // Position of the anchor point
    b2BodyId anchorBodyId = createAnchorBody(worldId, anchorX_m, anchorY_m); // Static anchor point for the rope

    float platWidthM = pixelsToMeters(120);
    float platHeightM = pixelsToMeters(20);
    float platX_m = anchorX_m;
    float platY_m = anchorY_m - pixelsToMeters(150); // Position platform below the anchor
    GameObject hangingPlatform;
    // Hanging platform: dynamic, allows rotation, some linear damping
    if (hangingPlatform.init(worldId, platX_m, platY_m, platWidthM, platHeightM, true, sf::Color(160, 82, 45), false, 0.5f)) {
        gameObjects.push_back(hangingPlatform);
        groundPlatformIds.push_back(hangingPlatform.bodyId);
    }
    b2BodyId platformBodyId = gameObjects.back().bodyId; // Assuming platform is last added

    // Optional: Output default mass data for debugging or fine-tuning.
    // std::cout << "Default mass data for hanging platform: " << b2Body_GetMass(platformBodyId) << std::endl;
    // std::cout << "Default inertia for hanging platform: " << b2Body_GetRotationalInertia(platformBodyId) << std::endl;

    // Example of setting custom mass data if needed
    b2MassData myMassData;
    myMassData.mass = 2.34375f;
    myMassData.center = (b2Vec2){0.0f, 0.0f}; // Local center of mass
    myMassData.rotationalInertia = 5.0f;
    b2Body_SetMassData(platformBodyId, myMassData);

    // --- Create Segmented Rope for Hanging Platform ---
    const int numRopeSegments = 10;
    std::vector<b2BodyId> ropeSegmentBodyIds; // Store IDs of rope segments
    ropeSegmentBodyIds.reserve(numRopeSegments);

    if (!B2_IS_NULL(anchorBodyId) && !B2_IS_NULL(platformBodyId)) {
        b2Vec2 anchorPos = b2Body_GetPosition(anchorBodyId); // World position of the static anchor
        b2Vec2 platformAttachPointLocal = {0.0f, platHeightM / 2.0f}; // Attach to top-center of platform
        b2Vec2 platformAttachPointWorld = b2Body_GetWorldPoint(platformBodyId, platformAttachPointLocal);

        float desiredTotalRopeLength = b2Distance(anchorPos, platformAttachPointWorld);
        float ropeSegmentLengthM = desiredTotalRopeLength / numRopeSegments;
        float segmentThicknessM = pixelsToMeters(8); // Thickness of each rope segment

        b2BodyId prevBodyId = anchorBodyId;
        b2Vec2 prevBodyAnchorLocal = {0.0f, 0.0f}; // Attachment point on the previous body (local coords)

        for (int i = 0; i < numRopeSegments; ++i) {
            GameObject segmentObj;
            // Initial position for segment (approximating a straight hang)
            float segmentCenterX = anchorPos.x;
            float segmentCenterY = anchorPos.y - (i * ropeSegmentLengthM) - (ropeSegmentLengthM / 2.0f);

            // Rope segment properties: dynamic, light, flexible, some damping
            if (segmentObj.init(worldId, segmentCenterX, segmentCenterY,
                                segmentThicknessM, ropeSegmentLengthM, // width (thickness), height (length)
                                true, sf::Color(139, 69, 19),      // Dynamic, Brown color
                                false, 0.2f, 0.05f, 0.5f, 0.1f)) { // no fixedRotation, linearDamping, density, friction, restitution
                gameObjects.push_back(segmentObj);
                ropeSegmentBodyIds.push_back(segmentObj.bodyId);

                b2RevoluteJointDef revoluteDef = b2DefaultRevoluteJointDef();
                revoluteDef.bodyIdA = prevBodyId;
                revoluteDef.bodyIdB = segmentObj.bodyId;
                revoluteDef.localAnchorA = prevBodyAnchorLocal; // Attach to bottom of prev segment (or anchor)
                revoluteDef.localAnchorB = {0.0f, ropeSegmentLengthM / 2.0f}; // Attach to top of current segment
                revoluteDef.collideConnected = false; // Segments of the same rope should not collide
                b2CreateRevoluteJoint(worldId, &revoluteDef);

                prevBodyId = segmentObj.bodyId;
                prevBodyAnchorLocal = {0.0f, -ropeSegmentLengthM / 2.0f}; // Next joint attaches to bottom of this segment
            } else {
                std::cerr << "Failed to create rope segment " << i << " for hanging platform." << std::endl;
            }
        }

        // Connect the last rope segment to the hanging platform
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
    // --- End Segmented Rope Creation for Hanging Platform ---
    
    // --- Create Horizontal Rope Bridge ---
    const int numHSegments = 20; // Number of segments in the horizontal rope
    b2Vec2 leftAnchorPos  = { pixelsToMeters(100), pixelsToMeters(200) };
    b2Vec2 rightAnchorPos = { pixelsToMeters(800), pixelsToMeters(200) };

    b2BodyId leftAnchor  = createAnchorBody(worldId, leftAnchorPos.x,  leftAnchorPos.y);
    b2BodyId rightAnchor = createAnchorBody(worldId, rightAnchorPos.x, rightAnchorPos.y);

    std::vector<b2BodyId> hRopeSegIds; // Store IDs of horizontal rope segments
    hRopeSegIds.reserve(numHSegments);

    float totalLength   = b2Distance(leftAnchorPos, rightAnchorPos);
    float segmentLength = totalLength / numHSegments; // Length of each segment (horizontal orientation)
    float thickness     = pixelsToMeters(3);          // Thickness of each segment (vertical orientation)

    b2BodyId prevBody    = leftAnchor;
    b2Vec2   prevLocalA  = {0.0f, 0.0f}; // Attachment point on the previous body (local coords)

    for (int i = 0; i < numHSegments; ++i) {
        // Interpolate segment center along the line between anchors
        float t = (i + 0.5f) / numHSegments; // Midpoint of the segment
        b2Vec2 center = {
            leftAnchorPos.x + t * (rightAnchorPos.x - leftAnchorPos.x),
            leftAnchorPos.y + t * (rightAnchorPos.y - leftAnchorPos.y)
        };

        GameObject segObj;
        // Horizontal rope segment properties: dynamic, some damping, higher density for more "weight"
        if (segObj.init(
                worldId,
                center.x, center.y,
                segmentLength, thickness, // width (length), height (thickness)
                true,               // dynamic
                sf::Color::Yellow,
                false,              // no fixedRotation
                1.0f,               // linearDamping
                1.0f,               // density
                0.0f,               // friction
                1.0f                // restitution
            ))
        {
            gameObjects.push_back(segObj);
            b2BodyId curBody = segObj.bodyId;
            hRopeSegIds.push_back(curBody);

            b2RevoluteJointDef jd = b2DefaultRevoluteJointDef();
            jd.bodyIdA        = prevBody;
            jd.bodyIdB        = curBody;
            jd.localAnchorA   = prevLocalA; // Attach to right end of prev segment (or anchor)
            jd.localAnchorB   = { -segmentLength * 0.5f, 0.0f }; // Attach to left end of current segment
            jd.collideConnected = false;
            b2CreateRevoluteJoint(worldId, &jd);

            prevBody   = curBody;
            prevLocalA = { segmentLength * 0.5f, 0.0f }; // Next joint attaches to right end of this segment
        }
    }

    // Connect the last segment of the horizontal rope to the right anchor
    if (!hRopeSegIds.empty()) {
        b2RevoluteJointDef jd = b2DefaultRevoluteJointDef();
        jd.bodyIdA        = hRopeSegIds.back();
        jd.bodyIdB        = rightAnchor;
        jd.localAnchorA   = { segmentLength * 0.5f, 0.0f }; // Right end of the last segment
        jd.localAnchorB   = { 0.0f, 0.0f };                 // Center of the static anchor
        jd.collideConnected = false;
        b2CreateRevoluteJoint(worldId, &jd);
    }
    // --- End Horizontal Rope Bridge Creation ---
}

#endif // MAP1_HPP
