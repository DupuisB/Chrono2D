#ifndef MAP1_HPP
#define MAP1_HPP

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
inline int loadMap1(b2WorldId worldId,
                     std::vector<GameObject>& gameObjects,
                     b2BodyId& playerBodyId) { 

    playerBodyId = b2_nullBodyId;
    int playerIndex = -1;

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

    // Ground
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(1000);
        float groundHeightM = pixelsToMeters(300);
        groundObj.setPosition(0, - groundHeightM / 2.0f);
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

    // Ground
    {
        GameObject groundObj2;
        float groundWidthM = pixelsToMeters(1000);
        float groundHeightM = pixelsToMeters(300);
        groundObj2.setPosition(pixelsToMeters(2000), - groundHeightM / 2.0f);
        groundObj2.setSize(groundWidthM, groundHeightM);
        groundObj2.setDynamic(false); // Sets density to 0
        groundObj2.setColor(sf::Color::Green);
        groundObj2.setFriction(0.7f);
        groundObj2.setRestitution(0.1f);
        groundObj2.setIsPlayerProperty(false);
        groundObj2.setCanJumpOnProperty(true);
        groundObj2.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj2.finalize(worldId)) {
            gameObjects.push_back(groundObj2);
        } else {
            std::cerr << "Failed to create ground object in map1." << std::endl;
        }
    }

    // Middle ground
    {
        GameObject groundObj3;
        float groundWidthM = pixelsToMeters(1000);
        float groundHeightM = pixelsToMeters(100);
        groundObj3.setPosition(pixelsToMeters(1000), -pixelsToMeters(300));
        groundObj3.setSize(groundWidthM, groundHeightM);
        groundObj3.setDynamic(false); // Sets density to 0
        groundObj3.setColor(sf::Color::Green);
        groundObj3.setFriction(0.7f);
        groundObj3.setRestitution(0.1f);
        groundObj3.setIsPlayerProperty(false);
        groundObj3.setCanJumpOnProperty(true);
        groundObj3.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj3.finalize(worldId)) {
            gameObjects.push_back(groundObj3);
        } else {
            std::cerr << "Failed to create middle ground object in map1." << std::endl;
        }
    }

    // Stack of Pushable Boxes
    {
        const int numberOfBoxes = 50;
        float boxSizeM = pixelsToMeters(40);    // Size of each box (width and height)
        float boxX_m = pixelsToMeters(400);

        float originalGroundReferenceY_m = pixelsToMeters(50);
        float smallOffset_m = pixelsToMeters(1);
        
        float firstBoxBottomY_m = originalGroundReferenceY_m + smallOffset_m;

        for (int i = 0; i < numberOfBoxes; ++i) {
            GameObject boxObj;
            
            float currentBoxBottomY_m = firstBoxBottomY_m + (i * boxSizeM);
            float boxCenterY_m = currentBoxBottomY_m + (boxSizeM / 2.0f);

            boxObj.setPosition(boxX_m, boxCenterY_m);
            boxObj.setSize(boxSizeM, boxSizeM);
            boxObj.setDynamic(true);
            boxObj.setColor(sf::Color::Red);
            boxObj.setLinearDamping(0.2f);  
            boxObj.setDensity(0.2f);
            boxObj.setFriction(0.7f);   
            boxObj.setRestitution(0.1f);    
            boxObj.setIsPlayerProperty(false);
            boxObj.setCanJumpOnProperty(true);
            boxObj.setCollidesWithPlayerProperty(true);

            if (boxObj.finalize(worldId)) {
                gameObjects.push_back(boxObj);
            } else {
                std::cerr << "Failed to create pushable box object " << i << " in stack for map1." << std::endl;
            }
        }
    }

    // Flag
    float flagX_m = pixelsToMeters(1600.0f);
    float flagY_m = pixelsToMeters(0.0f);
    float flagHeight = pixelsToMeters(120.0f);
    createFlag(worldId, gameObjects, flagX_m, flagY_m + flagHeight / 2.0f);

    return playerIndex;
}

#endif // MAP1_HPP
