#ifndef MAP4_HPP
#define MAP4_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "../include/game_object.hpp" // Includes utils.hpp and constants.hpp
#include "../include/primitives/rope.hpp"      // For createSegmentedRope
#include "../include/primitives/flag.hpp"      // For createFlag
#include <vector>
#include <iostream> // For std::cout, std::cerr
#include <cmath>    // For b2Distance, M_PI / b2_pi

inline float createHangingPlatformWithRopes(b2WorldId worldId,
                                            std::vector<GameObject>& gameObjects,
                                            float whereAmI,
                                            float gapBefore,
                                            float platformWidthPx,
                                            float platformHeightPx,
                                            float anchorPointHeightPx,
                                            sf::Color platformColor = sf::Color(160, 82, 45));

inline int loadMap4(b2WorldId worldId,
                    std::vector<GameObject>& gameObjects,
                    b2BodyId& playerBodyId) 
{
    playerBodyId = b2_nullBodyId;
    int playerIndex = -1;

    float groundWidth = 2000.0f;
    float groundHeight = 800.0f;
    float firstGap = 150.0f;
    float hangingPlatform_width = 300.0f;
    float hangingPlatform_height = 20.0f;
    float anchorPointHeight = 400.0f;

    float whereAmI = 0.0f;

    // --- Ground ---
    {
        GameObject groundObj;
        groundObj.setPosition(0.0f, pixelsToMeters(-groundHeight / 2.0f));
        groundObj.setSize(pixelsToMeters(groundWidth), pixelsToMeters(groundHeight));
        groundObj.setDynamic(false);
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true);
        groundObj.finalize(worldId);
        gameObjects.push_back(groundObj);
    }
    whereAmI += groundWidth / 2.0f;

    // --- Left Wall ---
    {
        GameObject leftWallObj;
        float wallWidth = 200.0f;
        float wallHeight = 2000.0f;
        leftWallObj.setPosition(pixelsToMeters(-groundWidth / 2.0f - wallWidth / 2.0f), pixelsToMeters(800.0f / 2.0f));
        leftWallObj.setSize(pixelsToMeters(wallWidth), pixelsToMeters(wallHeight));
        leftWallObj.setDynamic(false);
        leftWallObj.setColor(sf::Color(139, 69, 19));
        leftWallObj.setFriction(0.7f);
        leftWallObj.setRestitution(0.0f);
        leftWallObj.setIsPlayerProperty(false);
        leftWallObj.setCanJumpOnProperty(true);
        leftWallObj.setCollidesWithPlayerProperty(true);
        leftWallObj.finalize(worldId);
        gameObjects.push_back(leftWallObj);
    }

    // --- Hanging Platforms ---
    whereAmI = createHangingPlatformWithRopes(worldId, gameObjects, whereAmI, firstGap, hangingPlatform_width, hangingPlatform_height, anchorPointHeight);
    whereAmI = createHangingPlatformWithRopes(worldId, gameObjects, whereAmI, 400.0f, hangingPlatform_width, hangingPlatform_height, anchorPointHeight);
    whereAmI = createHangingPlatformWithRopes(worldId, gameObjects, whereAmI, 500.0f, hangingPlatform_width, hangingPlatform_height, anchorPointHeight);

    // --- second ground ---
    {
        GameObject groundObj;
        groundObj.setPosition(pixelsToMeters(whereAmI + groundWidth / 2.0f + 500.0f), pixelsToMeters(-groundHeight / 2.0f));
        groundObj.setSize(pixelsToMeters(groundWidth), pixelsToMeters(groundHeight));
        groundObj.setDynamic(false);
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true);
        groundObj.finalize(worldId);
        gameObjects.push_back(groundObj);
    }

    whereAmI += groundWidth + 500.0f; // Advance whereAmI by ground width + gap

    // --- dynamic rectangle ---
    {
        GameObject dynamicRectObj;
        float dynamicRectWidth = 695.0f;
        float dynamicRectHeight = 25.0f;
        dynamicRectObj.setPosition(pixelsToMeters(whereAmI - groundWidth / 2.0f), pixelsToMeters(0.0f));
        dynamicRectObj.setSize(pixelsToMeters(dynamicRectWidth), pixelsToMeters(dynamicRectHeight));
        dynamicRectObj.setDynamic(true);
        dynamicRectObj.setColor(sf::Color(139, 69, 19));
        dynamicRectObj.setLinearDamping(0.5f);
        dynamicRectObj.setDensity(1.0f);
        dynamicRectObj.setFriction(0.7f);
        dynamicRectObj.setRestitution(0.0f);
        dynamicRectObj.setIsPlayerProperty(false);
        dynamicRectObj.setCanJumpOnProperty(true);
        dynamicRectObj.setCollidesWithPlayerProperty(true);
        if (dynamicRectObj.finalize(worldId)) {
            gameObjects.push_back(dynamicRectObj);
        }
    }

    // little cube blockers to prevent dynamic rect from falling
    {
        GameObject blockerLeftObj;
        blockerLeftObj.setPosition(pixelsToMeters(whereAmI), pixelsToMeters(-40.0f));
        blockerLeftObj.setSize(pixelsToMeters(20.0f), pixelsToMeters(20.0f));
        blockerLeftObj.setDynamic(false);
        blockerLeftObj.setColor(sf::Color(34, 139, 34));
        blockerLeftObj.setFriction(0.7f);
        blockerLeftObj.setRestitution(0.0f);
        blockerLeftObj.setIsPlayerProperty(false);
        blockerLeftObj.setCanJumpOnProperty(true);
        blockerLeftObj.setCollidesWithPlayerProperty(true);
        blockerLeftObj.finalize(worldId);
        gameObjects.push_back(blockerLeftObj);
    }

    // right blocker
    {
        GameObject blockerRightObj;
        blockerRightObj.setPosition(pixelsToMeters(whereAmI + 700.0f), pixelsToMeters(-40.0f));
        blockerRightObj.setSize(pixelsToMeters(20.0f), pixelsToMeters(20.0f));
        blockerRightObj.setDynamic(false);
        blockerRightObj.setColor(sf::Color(34, 139, 34));
        blockerRightObj.setFriction(0.7f);
        blockerRightObj.setRestitution(0.0f);
        blockerRightObj.setIsPlayerProperty(false);
        blockerRightObj.setCanJumpOnProperty(true);
        blockerRightObj.setCollidesWithPlayerProperty(true);
        blockerRightObj.finalize(worldId);
        gameObjects.push_back(blockerRightObj);
    }

    // --- third ground ---
    {
        GameObject groundObj;
        groundObj.setPosition(pixelsToMeters(whereAmI + groundWidth / 2.0f + 700.0f), pixelsToMeters(-groundHeight / 2.0f));
        groundObj.setSize(pixelsToMeters(groundWidth), pixelsToMeters(groundHeight));
        groundObj.setDynamic(false);
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true);
        groundObj.finalize(worldId);
        gameObjects.push_back(groundObj);
    }

    whereAmI += 700.0f;

    // small platform up
    {
        GameObject stair1Obj;
        stair1Obj.setPosition(pixelsToMeters(whereAmI + 200.0f), pixelsToMeters(100.0f));
        stair1Obj.setSize(pixelsToMeters(150.0f), pixelsToMeters(20.0f));
        stair1Obj.setDynamic(false);
        stair1Obj.setColor(sf::Color(139, 69, 19));
        stair1Obj.setFriction(0.7f);
        stair1Obj.setRestitution(0.0f);
        stair1Obj.setIsPlayerProperty(false);
        stair1Obj.setCanJumpOnProperty(true);
        stair1Obj.setCollidesWithPlayerProperty(true);
        stair1Obj.finalize(worldId);
        gameObjects.push_back(stair1Obj);
    }

    // 2nd stair
    {
        GameObject stair2Obj;
        stair2Obj.setPosition(pixelsToMeters(whereAmI + 400.0f), pixelsToMeters(200.0f));
        stair2Obj.setSize(pixelsToMeters(150.0f), pixelsToMeters(20.0f));
        stair2Obj.setDynamic(false);
        stair2Obj.setColor(sf::Color(139, 69, 19));
        stair2Obj.setFriction(0.7f);
        stair2Obj.setRestitution(0.0f);
        stair2Obj.setIsPlayerProperty(false);
        stair2Obj.setCanJumpOnProperty(true);
        stair2Obj.setCollidesWithPlayerProperty(true);
        stair2Obj.finalize(worldId);
        gameObjects.push_back(stair2Obj);
    }

    // 3rd stair
    {
        GameObject stair3Obj;
        stair3Obj.setPosition(pixelsToMeters(whereAmI + 600.0f), pixelsToMeters(300.0f));
        stair3Obj.setSize(pixelsToMeters(150.0f), pixelsToMeters(20.0f));
        stair3Obj.setDynamic(false);
        stair3Obj.setColor(sf::Color(139, 69, 19));
        stair3Obj.setFriction(0.7f);
        stair3Obj.setRestitution(0.0f);
        stair3Obj.setIsPlayerProperty(false);
        stair3Obj.setCanJumpOnProperty(true);
        stair3Obj.setCollidesWithPlayerProperty(true);
        stair3Obj.finalize(worldId);
        gameObjects.push_back(stair3Obj);
    }

    float finalPlatformWidth = 500.0f;
    // final platform
    {
        GameObject finalPlatformObj;
        finalPlatformObj.setPosition(pixelsToMeters(whereAmI + 800.0f + finalPlatformWidth / 2.0f), pixelsToMeters(400.0f));
        finalPlatformObj.setSize(pixelsToMeters(finalPlatformWidth), pixelsToMeters(20.0f));
        finalPlatformObj.setDynamic(false);
        finalPlatformObj.setColor(sf::Color(34, 139, 34));
        finalPlatformObj.setFriction(0.7f);
        finalPlatformObj.setRestitution(0.0f);
        finalPlatformObj.setIsPlayerProperty(false);
        finalPlatformObj.setCanJumpOnProperty(true);
        finalPlatformObj.setCollidesWithPlayerProperty(true);
        finalPlatformObj.finalize(worldId);
        gameObjects.push_back(finalPlatformObj);
    }

    // put dynamic squre on top of final platform
    {
        GameObject dynamicSquareObj;
        dynamicSquareObj.setPosition(pixelsToMeters(whereAmI + 800.0f + finalPlatformWidth / 2.0f), pixelsToMeters(400.0f + 50.0f));
        dynamicSquareObj.setSize(pixelsToMeters(50.0f), pixelsToMeters(50.0f));
        dynamicSquareObj.setDynamic(true);
        dynamicSquareObj.setColor(sf::Color::Blue);
        dynamicSquareObj.setSpriteTexturePath("../assets/objects/box.png");
        dynamicSquareObj.setLinearDamping(1.0f);
        dynamicSquareObj.setDensity(50.0f);
        dynamicSquareObj.setFriction(0.0f);
        dynamicSquareObj.setRestitution(0.0f);
        dynamicSquareObj.setIsPlayerProperty(false);
        dynamicSquareObj.setCanJumpOnProperty(true);
        dynamicSquareObj.setCollidesWithPlayerProperty(true);
        if (dynamicSquareObj.finalize(worldId)) {
            gameObjects.push_back(dynamicSquareObj);
        }
    }

    whereAmI += 800.0f + finalPlatformWidth; // Advance whereAmI by final platform width

    // Balance
    {
        GameObject balanceObj;
        float balanceWidthM = 400.0f; // Width in meters
        float balanceHeightM = 20.0f; // Height in meters
        balanceObj.setPosition(pixelsToMeters(whereAmI + 50.0f + balanceWidthM/2.0f), pixelsToMeters(100.0f));
        balanceObj.setSize(pixelsToMeters(balanceWidthM), pixelsToMeters(balanceHeightM));
        balanceObj.setDynamic(true);
        balanceObj.setColor(sf::Color::Yellow);
        balanceObj.setFixedRotation(false); // Balance must be able to rotate
        balanceObj.setLinearDamping(0.2f);
        balanceObj.setDensity(1.0f); 
        balanceObj.setFriction(0.7f);
        balanceObj.setRestitution(0.0f);
        balanceObj.setIsPlayerProperty(false);
        balanceObj.setCanJumpOnProperty(true);
        balanceObj.setCollidesWithPlayerProperty(true);

        if (balanceObj.finalize(worldId)) {
            gameObjects.push_back(balanceObj);
            // Create a static anchor point at the center
            b2BodyDef anchorDef = b2DefaultBodyDef();
            anchorDef.position = {pixelsToMeters(whereAmI + 50.0f + balanceWidthM/2.0f), pixelsToMeters(100.0f)};
            anchorDef.type = b2_staticBody;
            b2BodyId anchorBodyId = b2CreateBody(worldId, &anchorDef);
            
            // Create revolute joint to pin the rectangle to the anchor
            b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
            jointDef.bodyIdA = anchorBodyId;
            jointDef.bodyIdB = balanceObj.bodyId;
            jointDef.localAnchorA = {0.0f, 0.0f}; // Center of anchor
            jointDef.localAnchorB = {0.0f, 0.0f}; // Center of rectangle
            jointDef.enableLimit = false; // Allow full rotation
            
            b2CreateRevoluteJoint(worldId, &jointDef);
        } else {
            std::cerr << "Failed to create balance object in map1." << std::endl;
        }
    }

    whereAmI += -800.0f - finalPlatformWidth; // Reset whereAmI to the start of the final ground
    whereAmI += groundWidth; // Advance whereAmI by ground width

    // create final ground
    {
        GameObject finalGroundObj;
        finalGroundObj.setPosition(pixelsToMeters(whereAmI + groundWidth / 6.0f), pixelsToMeters(0.0f));
        finalGroundObj.setSize(pixelsToMeters(groundWidth / 3.0f), pixelsToMeters(400.0f));
        finalGroundObj.setDynamic(false);
        finalGroundObj.setColor(sf::Color(34, 139, 34));
        finalGroundObj.setFriction(0.7f);
        finalGroundObj.setRestitution(0.0f);
        finalGroundObj.setIsPlayerProperty(false);
        finalGroundObj.setCanJumpOnProperty(true);
        finalGroundObj.setCollidesWithPlayerProperty(true);
        finalGroundObj.finalize(worldId);
        gameObjects.push_back(finalGroundObj);
    }

    // --- Create Flag ---
    float flagX_m = pixelsToMeters(whereAmI + groundWidth / 6.0f + 50.0f);
    float groundHeightM_val_for_flag = pixelsToMeters(350);
    float flagHeight_m_val = pixelsToMeters(120.0f);
    float flagY_m = pixelsToMeters(250.0f + flagHeight_m_val / 2.0f);
    
    createFlag(worldId, gameObjects, flagX_m, flagY_m);
    // --- End Flag Creation ---

    // --- Player ---
    {
        GameObject playerObj;
        float playerWidthM = pixelsToMeters(70);
        float playerHeightM = pixelsToMeters(90);
        playerObj.setPosition(pixelsToMeters(100), pixelsToMeters(300));
        playerObj.setSize(playerWidthM, playerHeightM);
        playerObj.setDynamic(true);
        playerObj.setColor(sf::Color::Blue);
        playerObj.setFixedRotation(true);
        playerObj.setDensity(1.0f);
        playerObj.setFriction(0.7f);
        playerObj.setRestitution(0.0f);
        playerObj.setIsPlayerProperty(true);
        playerObj.setCanJumpOnProperty(true);
        playerObj.finalize(worldId);
        playerBodyId = playerObj.bodyId;
        gameObjects.push_back(playerObj);
        playerIndex = gameObjects.size() - 1;
    }

    return playerIndex;
}

inline float createHangingPlatformWithRopes(b2WorldId worldId,
                                            std::vector<GameObject>& gameObjects,
                                            float whereAmI,
                                            float gapBefore,
                                            float platformWidthPx,
                                            float platformHeightPx,
                                            float anchorPointHeightPx,
                                            sf::Color platformColor) 
{
    float segmentThicknessM = pixelsToMeters(8);
    const int numRopeSegments = 10;

    // Calculate platform X position
    float platformX_m = pixelsToMeters(whereAmI + gapBefore + platformWidthPx / 2.0f);
    float platformY_m = pixelsToMeters(50);

    float platWidthM = pixelsToMeters(platformWidthPx);
    float platHeightM = pixelsToMeters(platformHeightPx);

    // Create dynamic hanging platform
    b2BodyId platformBodyId = b2_nullBodyId;
    {
        GameObject platformObj;
        platformObj.setPosition(platformX_m, platformY_m);
        platformObj.setSize(platWidthM, platHeightM);
        platformObj.setDynamic(true);
        platformObj.setColor(platformColor);
        platformObj.setLinearDamping(0.5f);
        platformObj.setDensity(1.0f);
        platformObj.setFriction(0.7f);
        platformObj.setRestitution(0.0f);
        platformObj.setIsPlayerProperty(false);
        platformObj.setCanJumpOnProperty(true);
        platformObj.setCollidesWithPlayerProperty(true);

        if (platformObj.finalize(worldId)) {
            platformBodyId = platformObj.bodyId;
            gameObjects.push_back(platformObj);

            if (!B2_IS_NULL(platformBodyId)) {
                b2MassData massData;
                massData.mass = 2.34375f;
                massData.center = {0.0f, 0.0f};
                massData.rotationalInertia = 5.0f;
                b2Body_SetMassData(platformBodyId, massData);
            }
        } else {
            std::cerr << "Failed to create hanging platform object." << std::endl;
            return whereAmI + gapBefore + platformWidthPx; // Fail-safe advance
        }
    }

    // --- Anchors ---
    b2BodyId leftAnchorBodyId = b2_nullBodyId;
    b2BodyId rightAnchorBodyId = b2_nullBodyId;
    float leftAnchorX = whereAmI + gapBefore;
    float rightAnchorX = whereAmI + gapBefore + platformWidthPx;

    // Left anchor
    {
        GameObject leftAnchorObj;
        leftAnchorObj.setPosition(pixelsToMeters(leftAnchorX), pixelsToMeters(anchorPointHeightPx));
        leftAnchorObj.setSize(pixelsToMeters(1), pixelsToMeters(1));
        leftAnchorObj.setDynamic(false);
        leftAnchorObj.setColor(sf::Color::Transparent);
        if (leftAnchorObj.finalize(worldId)) {
            gameObjects.push_back(leftAnchorObj);
            leftAnchorBodyId = leftAnchorObj.bodyId;
        }
    }

    // Right anchor
    {
        GameObject rightAnchorObj;
        rightAnchorObj.setPosition(pixelsToMeters(rightAnchorX), pixelsToMeters(anchorPointHeightPx));
        rightAnchorObj.setSize(pixelsToMeters(1), pixelsToMeters(1));
        rightAnchorObj.setDynamic(false);
        rightAnchorObj.setColor(sf::Color::Transparent);
        if (rightAnchorObj.finalize(worldId)) {
            gameObjects.push_back(rightAnchorObj);
            rightAnchorBodyId = rightAnchorObj.bodyId;
        }
    }

    // --- Ropes ---
    if (!B2_IS_NULL(leftAnchorBodyId) && !B2_IS_NULL(platformBodyId)) {
        b2Vec2 leftAttachPointLocal = {-platWidthM / 2.0f, platHeightM / 2.0f}; // Left edge, top
        createSegmentedRope(worldId, gameObjects,
                            leftAnchorBodyId, {0.0f, 0.0f},
                            platformBodyId, leftAttachPointLocal,
                            numRopeSegments, 0.0f, segmentThicknessM,
                            true, sf::Color(139, 69, 19),
                            0.1f, 5.0f, 0.0f, 0.1f,
                            false, false);
    }
    if (!B2_IS_NULL(rightAnchorBodyId) && !B2_IS_NULL(platformBodyId)) {
        b2Vec2 rightAttachPointLocal = {platWidthM / 2.0f, platHeightM / 2.0f}; // Right edge, top
        createSegmentedRope(worldId, gameObjects,
                            rightAnchorBodyId, {0.0f, 0.0f},
                            platformBodyId, rightAttachPointLocal,
                            numRopeSegments, 0.0f, segmentThicknessM,
                            true, sf::Color(139, 69, 19),
                            0.1f, 5.0f, 0.0f, 0.1f,
                            false, false);
    }

    // Advance whereAmI by gap + width for next item placement
    return whereAmI + gapBefore + platformWidthPx;
}


#endif // MAP4_HPP