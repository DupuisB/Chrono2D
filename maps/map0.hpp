#ifndef MAP0_HPP
#define MAP0_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "../include/game_object.hpp" // Includes utils.hpp and constants.hpp
#include "../include/primitives/rope.hpp"      // For createSegmentedRope
#include "../include/primitives/flag.hpp"      // For createFlag
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
 * @return The index of the player GameObject in the gameObjects vector, or -1 if not created.
 */
inline int loadMap0(b2WorldId worldId,
                     std::vector<GameObject>& gameObjects,
                     b2BodyId& playerBodyId) { 

    playerBodyId = b2_nullBodyId;
    int playerIndex = -1;

    // Ground
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(WINDOW_WIDTH);
        float groundHeightM = pixelsToMeters(50);
        groundObj.setPosition(groundWidthM / 2.0f, groundHeightM / 2.0f);
        groundObj.setSize(groundWidthM, groundHeightM);
        groundObj.setDynamic(false); // Sets density to 0
        groundObj.setColor(sf::Color::Green);
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.1f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj.finalize(worldId)) {
            gameObjects.push_back(groundObj);
        } else {
            std::cerr << "Failed to create ground object in map1." << std::endl;
        }
    }

    // Player
    {
        GameObject playerObj;
        float playerWidthM = pixelsToMeters(70);
        float playerHeightM = pixelsToMeters(90);
        playerObj.setPosition(pixelsToMeters(100), pixelsToMeters(300));
        playerObj.setSize(playerWidthM, playerHeightM);
        playerObj.setDynamic(true);
        playerObj.setColor(sf::Color::Blue); // Color is for sfShape, sprite will be used
        playerObj.setFixedRotation(true);
        // playerObj.setLinearDamping(0.0f); // Default
        playerObj.setDensity(1.0f);
        playerObj.setFriction(0.7f); 
        playerObj.setRestitution(0.0f); // Player usually doesn't bounce
        
        playerObj.setIsPlayerProperty(true);
        playerObj.setEnableSensorEventsProperty(true);
        // playerObj.setCanJumpOnProperty(false); // Default is false
        // playerObj.setCollidesWithPlayerProperty(true); // Player collides with world, handled by setIsPlayerProperty

        if (playerObj.finalize(worldId)) {
            playerBodyId = playerObj.bodyId;
            gameObjects.push_back(playerObj);
            gameObjects.back().ensureCorrectSpriteTextureLink(); // Add for player object
            playerIndex = static_cast<int>(gameObjects.size() - 1);
        } else {
            std::cerr << "Failed to create player object in map1." << std::endl;
            playerIndex = -1; 
        }
    }

    // Pushable Box
    {
        GameObject boxObj;
        float boxSizeM = pixelsToMeters(40);
        float groundCenterY_m = pixelsToMeters(50) / 2.0f; // Assuming ground is at y=0 to y=50px
        float groundTopY_m = pixelsToMeters(50); // If ground bottom is at y=0
        
        // Corrected Y position calculation:
        // Ground is created with its center at groundHeightM / 2.0f.
        // So its top surface is at groundHeightM.
        float groundHeightM_val = pixelsToMeters(50); // Use the actual value
        boxObj.setPosition(pixelsToMeters(400), groundHeightM_val + boxSizeM / 2.0f + pixelsToMeters(1));
        boxObj.setSize(boxSizeM, boxSizeM);
        boxObj.setDynamic(true);
        boxObj.setColor(sf::Color::Red);
        // boxObj.setFixedRotation(false); // Default
        boxObj.setLinearDamping(0.2f);
        boxObj.setDensity(1.0f); 
        boxObj.setFriction(0.7f);
        boxObj.setRestitution(0.1f);
        boxObj.setIsPlayerProperty(false);
        boxObj.setCanJumpOnProperty(true);
        boxObj.setCollidesWithPlayerProperty(true);

        if (boxObj.finalize(worldId)) {
            gameObjects.push_back(boxObj);
        } else {
            std::cerr << "Failed to create pushable box object in map1." << std::endl;
        }
    }

    // --- Hanging Platform ---
    b2BodyId platformBodyId = b2_nullBodyId; // Declare here for scope
    {
        GameObject platformObj;
        float anchorX_m = pixelsToMeters(1500);
        float anchorY_m = pixelsToMeters(500); 
        
        float platWidthM = pixelsToMeters(120);
        float platHeightM = pixelsToMeters(20);
        platformObj.setPosition(anchorX_m, anchorY_m - pixelsToMeters(150));
        platformObj.setSize(platWidthM, platHeightM);
        platformObj.setDynamic(true);
        platformObj.setColor(sf::Color(160, 82, 45));
        // platformObj.setFixedRotation(false); // Default
        platformObj.setLinearDamping(0.5f);
        platformObj.setDensity(1.0f); // Initial density, mass data will be set explicitly
        platformObj.setFriction(0.7f);
        platformObj.setRestitution(0.1f);
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
            std::cerr << "Failed to create hanging platform object in map1." << std::endl;
        }
    }
    
    // --- Create Segmented Rope for Hanging Platform ---
    b2BodyId hangingAnchorBodyId = b2_nullBodyId;
    {
        GameObject anchorObj;
        anchorObj.setPosition(pixelsToMeters(1500), pixelsToMeters(500));
        anchorObj.setSize(pixelsToMeters(1), pixelsToMeters(1)); // Small, effectively invisible
        anchorObj.setDynamic(false);
        anchorObj.setColor(sf::Color::Transparent); // Invisible
        // Default collision: CATEGORY_WORLD, MASK_PLAYER | CATEGORY_WORLD. Fine for a static anchor.
        // Not jumpable, not player.
        if (anchorObj.finalize(worldId)) {
            gameObjects.push_back(anchorObj);
            hangingAnchorBodyId = anchorObj.bodyId;
        } else {
            std::cerr << "Failed to create hanging anchor for rope." << std::endl;
        }
    }

    const int numRopeSegments = 10;
    float segmentThicknessM = pixelsToMeters(8); 

    if (!B2_IS_NULL(hangingAnchorBodyId) && !B2_IS_NULL(platformBodyId)) {
        b2Vec2 platformAttachPointLocal = {0.0f, pixelsToMeters(20) / 2.0f}; 
        
        createSegmentedRope(worldId, gameObjects,
                            hangingAnchorBodyId, {0.0f, 0.0f}, 
                            platformBodyId, platformAttachPointLocal, 
                            numRopeSegments,
                            0.0f, 
                            segmentThicknessM,
                            true, 
                            sf::Color(139, 69, 19),
                            0.2f, 0.05f, 0.5f, 0.1f,
                            false, false); // segmentsCanBeJumpedOn=false, segmentsCollideWithPlayer=false (original intent)
    }
    // --- End Segmented Rope Creation for Hanging Platform ---
    
    // --- Create Horizontal Rope Bridge ---
    const int numHSegments = 20; 
    b2Vec2 leftAnchorPosWorld  = { pixelsToMeters(100), pixelsToMeters(200) };
    b2Vec2 rightAnchorPosWorld = { pixelsToMeters(800), pixelsToMeters(200) };

    b2BodyId leftBridgeAnchorBodyId = b2_nullBodyId;
    {
        GameObject anchorObj;
        anchorObj.setPosition(leftAnchorPosWorld.x, leftAnchorPosWorld.y);
        anchorObj.setSize(pixelsToMeters(1), pixelsToMeters(1));
        anchorObj.setDynamic(false);
        anchorObj.setColor(sf::Color::Transparent);
        if (anchorObj.finalize(worldId)) {
            gameObjects.push_back(anchorObj);
            leftBridgeAnchorBodyId = anchorObj.bodyId;
        } else {
            std::cerr << "Failed to create left bridge anchor." << std::endl;
        }
    }

    b2BodyId rightBridgeAnchorBodyId = b2_nullBodyId;
    {
        GameObject anchorObj;
        anchorObj.setPosition(rightAnchorPosWorld.x, rightAnchorPosWorld.y);
        anchorObj.setSize(pixelsToMeters(1), pixelsToMeters(1));
        anchorObj.setDynamic(false);
        anchorObj.setColor(sf::Color::Transparent);
        if (anchorObj.finalize(worldId)) {
            gameObjects.push_back(anchorObj);
            rightBridgeAnchorBodyId = anchorObj.bodyId;
        } else {
            std::cerr << "Failed to create right bridge anchor." << std::endl;
        }
    }

    float hSegmentThickness = pixelsToMeters(3); 

    if (!B2_IS_NULL(leftBridgeAnchorBodyId) && !B2_IS_NULL(rightBridgeAnchorBodyId)) {
        createSegmentedRope(worldId, gameObjects,
                            leftBridgeAnchorBodyId, {0.0f, 0.0f},
                            rightBridgeAnchorBodyId, {0.0f, 0.0f},
                            numHSegments,
                            0.0f, 
                            hSegmentThickness,
                            false, 
                            sf::Color::Yellow,
                            1.0f, 1.0f, 0.0f, 1.0f, // High damping, high density, no friction, high restitution for a "bouncy" bridge
                            true, true); 
    }
    // --- End Horizontal Rope Bridge Creation ---

    // --- Create Flag ---
    float flagX_m = pixelsToMeters(WINDOW_WIDTH - 150.0f);
    float groundHeightM_val_for_flag = pixelsToMeters(50);
    float flagHeight_m_val = pixelsToMeters(120.0f);
    float flagY_m = groundHeightM_val_for_flag + flagHeight_m_val / 2.0f;
    
    createFlag(worldId, gameObjects, flagX_m, flagY_m);
    // --- End Flag Creation ---

    return playerIndex;
}

#endif // MAP1_HPP
    