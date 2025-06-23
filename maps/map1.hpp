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
#include <chrono>   // For timing

/**
 * @brief Loads the game objects for Map 1 into the world.
 * This includes the ground with a big hole in the middle, player, 
 * and a box spawning system that drops boxes every second.
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
        playerObj.setColor(sf::Color::Blue);
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
            playerIndex = static_cast<int>(gameObjects.size() - 1);
        } else {
            std::cerr << "Failed to create player object in map1." << std::endl;
            playerIndex = -1; 
        }
    }

    // Left Ground (before the hole)
    {
        GameObject leftGroundObj;
        float groundWidthM = pixelsToMeters(800);
        float groundHeightM = pixelsToMeters(300);
        leftGroundObj.setPosition(-pixelsToMeters(200), - groundHeightM / 2.0f);
        leftGroundObj.setSize(groundWidthM, groundHeightM);
        leftGroundObj.setDynamic(false);
        leftGroundObj.setColor(sf::Color(34, 139, 34));
        leftGroundObj.setFriction(0.7f);
        leftGroundObj.setRestitution(0.0f);
        leftGroundObj.setIsPlayerProperty(false);
        leftGroundObj.setCanJumpOnProperty(true);
        leftGroundObj.setCollidesWithPlayerProperty(true);

        if (leftGroundObj.finalize(worldId)) {
            gameObjects.push_back(leftGroundObj);
        } else {
            std::cerr << "Failed to create left ground object in map1." << std::endl;
        }
    }

    // Right Ground (after the hole)
    {
        GameObject rightGroundObj;
        float groundWidthM = pixelsToMeters(800);
        float groundHeightM = pixelsToMeters(300);
        rightGroundObj.setPosition(pixelsToMeters(1800), - groundHeightM / 2.0f);
        rightGroundObj.setSize(groundWidthM, groundHeightM);
        rightGroundObj.setDynamic(false);
        // Dark green
        rightGroundObj.setColor(sf::Color(34, 139, 34));
        rightGroundObj.setFriction(0.7f);
        rightGroundObj.setRestitution(0.0f);
        rightGroundObj.setIsPlayerProperty(false);
        rightGroundObj.setCanJumpOnProperty(true);
        rightGroundObj.setCollidesWithPlayerProperty(true);

        if (rightGroundObj.finalize(worldId)) {
            gameObjects.push_back(rightGroundObj);
        } else {
            std::cerr << "Failed to create right ground object in map1." << std::endl;
        }
    }

    // Flag
    float flagX_m = pixelsToMeters(1700.0f);
    float flagY_m = pixelsToMeters(0.0f);
    float flagHeight = pixelsToMeters(120.0f);
    createFlag(worldId, gameObjects, flagX_m, flagY_m + flagHeight / 2.0f);

    return playerIndex;
}

/**
 * @brief Updates the map1 spawning system. Call this every frame.
 * @param worldId The ID of the Box2D world.
 * @param gameObjects A reference to the vector that stores all GameObjects.
 * @param timeFreeze A boolean indicating whether time is currently frozen.
 */
inline void updateMap1(b2WorldId worldId, std::vector<GameObject>& gameObjects, bool timeFreeze) {
    static auto lastSpawnTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto timeSinceLastSpawn = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastSpawnTime);
    
    // Spawn boxes every 1000ms (1 second)
    if (!timeFreeze && timeSinceLastSpawn.count() >= 1000) {
        float boxSizeM = pixelsToMeters(80);
        
        // Spawn first box
        {
            GameObject boxObj;
            float spawnX = pixelsToMeters(450 + (rand() % 100)); // Random X between 450-550 pixels
            float spawnY = pixelsToMeters(800); // High above the platform
            
            boxObj.setPosition(spawnX, spawnY);
            boxObj.setSize(boxSizeM, boxSizeM);
            boxObj.setDynamic(true);
            boxObj.setColor(sf::Color::Red);
            boxObj.setSpriteTexturePath("../assets/objects/box.png");
            boxObj.setLinearDamping(0.1f);  
            boxObj.setDensity(0.5f);
            boxObj.setFriction(0.7f);   
            boxObj.setRestitution(0.0f);    
            boxObj.setIsPlayerProperty(false);
            boxObj.setCanJumpOnProperty(true);
            boxObj.setCollidesWithPlayerProperty(true);

            if (boxObj.finalize(worldId)) {
                gameObjects.push_back(boxObj);
            } else {
                std::cerr << "Failed to create first falling box in map1." << std::endl;
            }
        }
        
        // Spawn second box 1000 pixels later
        {
            GameObject boxObj2;
            float spawnX2 = pixelsToMeters(450 + (rand() % 100) + 500); // Random X between 1450-1550 pixels
            float spawnY2 = pixelsToMeters(800); // High above the platform
            
            boxObj2.setPosition(spawnX2, spawnY2);
            boxObj2.setSize(boxSizeM, boxSizeM);
            boxObj2.setDynamic(true);
            boxObj2.setColor(sf::Color::Red);
            boxObj2.setSpriteTexturePath("../assets/objects/box.png");
            boxObj2.setLinearDamping(0.1f);  
            boxObj2.setDensity(0.5f);
            boxObj2.setFriction(0.7f);   
            boxObj2.setRestitution(0.0f);    
            boxObj2.setIsPlayerProperty(false);
            boxObj2.setCanJumpOnProperty(true);
            boxObj2.setCollidesWithPlayerProperty(true);

            if (boxObj2.finalize(worldId)) {
                gameObjects.push_back(boxObj2);
            } else {
                std::cerr << "Failed to create second falling box in map1." << std::endl;
            }
        }
        
        lastSpawnTime = currentTime;
    }
}

#endif // MAP1_HPP