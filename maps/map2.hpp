#ifndef MAP2_HPP
#define MAP2_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "../include/game_object.hpp" // Includes utils.hpp and constants.hpp
#include "../include/primitives/rope.hpp"      // For createSegmentedRope
#include "../include/primitives/flag.hpp"      // For createFlag
#include "../include/primitives/tremplin.hpp" // For createTremplin
#include <vector>
#include <iostream> // For std::cout, std::cerr
#include <cmath>    // For b2Distance, M_PI / b2_pi

/**
 * @brief Loads the game objects for Map 2 into the world.
 * This includes the ground, player, a pushable box, a hanging platform with a rope,
 * and a horizontal rope bridge.
 * @param worldId The ID of the Box2D world.
 * @param gameObjects A reference to the vector that will store all created GameObjects.
 * @param playerBodyId A reference to store the b2BodyId of the created player object.
 * @return The index of the player GameObject in the gameObjects vector, or -1 if not created.
 */
inline int loadMap2(b2WorldId worldId,
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
            std::cerr << "Failed to create ground object in map2." << std::endl;
        }
    }

    // Wall
    {   GameObject wallObj;
        float wallWidthM = pixelsToMeters(600);
        float wallHeightM = pixelsToMeters(600);
        wallObj.setPosition(wallWidthM / 2.0f + pixelsToMeters(1000), wallHeightM / 2.0f + pixelsToMeters(50));
        wallObj.setSize(wallWidthM, wallHeightM);
        wallObj.setDynamic(false); // Sets density to 0
        wallObj.setColor(sf::Color::Green);
        wallObj.setFriction(0.7f);
        wallObj.setRestitution(0.1f);
        wallObj.setIsPlayerProperty(false);
        wallObj.setCanJumpOnProperty(true);
        wallObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (wallObj.finalize(worldId)) {
            gameObjects.push_back(wallObj);
        } else {
            std::cerr << "Failed to create wall object in map2." << std::endl;
        }
    }

    // Player
    {
        GameObject playerObj;
        float playerWidthM = pixelsToMeters(70);
        float playerHeightM = pixelsToMeters(90);
        playerObj.setPosition(pixelsToMeters(300), pixelsToMeters(200));
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
            std::cerr << "Failed to create player object in map2." << std::endl;
            playerIndex = -1; 
        }
    }

    // Pushable Box
    {
        GameObject boxObj;
        float boxSizeM = pixelsToMeters(40);
        float groundHeightM_val = pixelsToMeters(50); // Use the actual value
        boxObj.setPosition(pixelsToMeters(400), pixelsToMeters(160));
        boxObj.setSize(boxSizeM, boxSizeM);
        boxObj.setDynamic(true);
        boxObj.setSpriteTexturePath("../assets/objects/box.png");
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
            GameObject& actualboxinInVector = gameObjects.back(); // Get a reference to the tremplin in the vector
            actualboxinInVector.ensureCorrectSpriteTextureLink();
        } else {
            std::cerr << "Failed to create pushable box object in map2." << std::endl;
        }
    }

    // ---  Platform ---
    b2BodyId platformBodyId = b2_nullBodyId; // Declare here for scope
    {
        GameObject platformObj;
        float anchorX_m = pixelsToMeters(400);
        float anchorY_m = pixelsToMeters(160); 
        
        float platWidthM = pixelsToMeters(300);
        float platHeightM = pixelsToMeters(20);
        platformObj.setPosition(anchorX_m, anchorY_m);
        platformObj.setSize(platWidthM, platHeightM);
        platformObj.setDynamic(false);
        platformObj.setColor(sf::Color(160, 82, 45));
        // platformObj.setFixedRotation(false); // Default
        platformObj.setLinearDamping(0.5f);
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
            std::cerr << "Failed to create hanging platform object in map2." << std::endl;
        }
    }
    // --- End Platform ---


    // --- Create Tremplin ---
    {
        createTremplin(worldId, gameObjects, false, pixelsToMeters(930), pixelsToMeters(75));
    }
    // --- End Tremplin ---
    

    // --- Create Flag ---
    // Place the flag somewhere in the map, e.g., near the right side
    float flagX_m = pixelsToMeters(WINDOW_WIDTH - 150.0f);
    float groundHeightM_val_for_flag = pixelsToMeters(650); // Assuming ground height is 50px
    float flagHeight_m_val = pixelsToMeters(120.0f); // Flag's own height
    float flagY_m = groundHeightM_val_for_flag + flagHeight_m_val / 2.0f; // Position flag on the ground
    
    createFlag(worldId, gameObjects, flagX_m, flagY_m);
    // --- End Flag Creation ---

    return playerIndex;
}

#endif // MAP2_HPP
