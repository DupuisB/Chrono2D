#ifndef MAP3_HPP
#define MAP3_HPP

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
inline int loadMap3(b2WorldId worldId,
                     std::vector<GameObject>& gameObjects,
                     b2BodyId& playerBodyId) { 

    playerBodyId = b2_nullBodyId;
    int playerIndex = -1;
    
    // Left Wall
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(1000);
        float groundHeightM = pixelsToMeters(300);
        groundObj.setPosition(-groundWidthM / 2.0f, groundHeightM / 2.0f);
        groundObj.setSize(groundWidthM, groundHeightM);
        groundObj.setDynamic(false); // Sets density to 0
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj.finalize(worldId)) {
            gameObjects.push_back(groundObj);
        } else {
            std::cerr << "Failed to create ground object in map1." << std::endl;
        }
    }
    // Leftest Wall
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(400);
        float groundHeightM = pixelsToMeters(600);
        groundObj.setPosition(-pixelsToMeters(1000)/ 2.0f - pixelsToMeters(100), groundHeightM / 2.0f);
        groundObj.setSize(groundWidthM, groundHeightM);
        groundObj.setDynamic(false); // Sets density to 0
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj.finalize(worldId)) {
            gameObjects.push_back(groundObj);
        } else {
            std::cerr << "Failed to create ground object in map1." << std::endl;
        }
    }
        // Right Wall
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(1000);
        float groundHeightM = pixelsToMeters(800);
        groundObj.setPosition(groundWidthM / 2.0f+ pixelsToMeters(1600), groundHeightM / 2.0f);
        groundObj.setSize(groundWidthM, groundHeightM);
        groundObj.setDynamic(false); // Sets density to 0
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj.finalize(worldId)) {
            gameObjects.push_back(groundObj);
        } else {
            std::cerr << "Failed to create ground object in map1." << std::endl;
        }
    }




    // Ground 0
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(660);
        float groundHeightM = pixelsToMeters(70);
        groundObj.setPosition(pixelsToMeters(330), groundHeightM);
        groundObj.setSize(groundWidthM, groundHeightM);
        groundObj.setDynamic(false); // Sets density to 0
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj.finalize(worldId)) {
            gameObjects.push_back(groundObj);
        } else {
            std::cerr << "Failed to create first ground object in map3." << std::endl;
        }
    }



    // Ground 1
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(4000);
        float groundHeightM = pixelsToMeters(500);
        groundObj.setPosition(0, pixelsToMeters(70)-groundHeightM / 2.0f);
        groundObj.setSize(groundWidthM, groundHeightM);
        groundObj.setDynamic(false); // Sets density to 0
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj.finalize(worldId)) {
            gameObjects.push_back(groundObj);
        } else {
            std::cerr << "Failed to create ground object in map1." << std::endl;
        }
    }

    // Ground 2
    {
        GameObject groundObj;
        float groundWidthM = pixelsToMeters(600);
        float groundHeightM = pixelsToMeters(350);
        groundObj.setPosition(groundWidthM / 2.0f + pixelsToMeters(1000), groundHeightM / 2.0f);
        groundObj.setSize(groundWidthM, groundHeightM);
        groundObj.setDynamic(false); // Sets density to 0
        groundObj.setColor(sf::Color(34, 139, 34));
        groundObj.setFriction(0.7f);
        groundObj.setRestitution(0.0f);
        groundObj.setIsPlayerProperty(false);
        groundObj.setCanJumpOnProperty(true);
        groundObj.setCollidesWithPlayerProperty(true); // Default for non-player, but explicit

        if (groundObj.finalize(worldId)) {
            gameObjects.push_back(groundObj);
        } else {
            std::cerr << "Failed to create second ground object in map1." << std::endl;
        }
    }

    // Player
    {
        GameObject playerObj;
        float playerWidthM = pixelsToMeters(70);
        float playerHeightM = pixelsToMeters(90);
        playerObj.setPosition(pixelsToMeters(100), pixelsToMeters(70) + playerHeightM / 2.0f);
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
            playerIndex = static_cast<int>(gameObjects.size() - 1);
        } else {
            std::cerr << "Failed to create player object in map1." << std::endl;
            playerIndex = -1; 
        }
    }

    // Balance

    {
        GameObject balanceObj;
        float balanceWidthM = pixelsToMeters(300);
        float balanceHeightM = pixelsToMeters(30);
        balanceObj.setPosition(pixelsToMeters(830), pixelsToMeters(150));
        balanceObj.setSize(balanceWidthM, balanceHeightM);
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
            anchorDef.position = {pixelsToMeters(830), pixelsToMeters(150)};
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


    // --- Create Flag ---
    float flagX_m = pixelsToMeters(WINDOW_WIDTH - 150.0f);
    float groundHeightM_val_for_flag = pixelsToMeters(350);
    float flagHeight_m_val = pixelsToMeters(120.0f);
    float flagY_m = groundHeightM_val_for_flag + flagHeight_m_val / 2.0f;
    
    createFlag(worldId, gameObjects, flagX_m, flagY_m);
    // --- End Flag Creation ---

    return playerIndex;
}

#endif // MAP3_HPP
