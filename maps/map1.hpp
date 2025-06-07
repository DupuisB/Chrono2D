#ifndef MAP1_HPP
#define MAP1_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "../include/game_object.hpp" // Includes utils.hpp and constants.hpp
#include "../include/primitives/rectangle.hpp" // For createRectangle
#include "../include/primitives/rope.hpp"      // For createSegmentedRope
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
                     b2BodyId& playerBodyId) { // Pass by reference to update player's body ID

    // Ensure playerBodyId is initialized to null before attempting to create the player.
    playerBodyId = b2_nullBodyId;
    int playerIndex = -1;

    // Ground
    float groundWidthM = pixelsToMeters(WINDOW_WIDTH);
    float groundHeightM = pixelsToMeters(50);
    float groundXM = groundWidthM / 2.0f;
    float groundYM = groundHeightM / 2.0f;
    createRectangle(worldId, gameObjects, groundXM, groundYM, groundWidthM, groundHeightM,
                    false, sf::Color::Green, false, 0.0f, 1.0f, 0.7f, 0.1f,
                    false, true, true); // isPlayer=false, canJumpOn=true, doPlayerCollide=true

    // Player
    float playerWidthM = pixelsToMeters(30);
    float playerHeightM = pixelsToMeters(50);
    float playerXM = pixelsToMeters(100);
    float playerYM = pixelsToMeters(300);
    playerBodyId = createRectangle(worldId, gameObjects, playerXM, playerYM, playerWidthM, playerHeightM,
                                   true, sf::Color::Blue, true, 0.0f, 1.0f, 0.7f, 0.0f,
                                   true, false, true); // isPlayer=true, canJumpOn=false (player can't jump on itself), doPlayerCollide=true (player collides with world)

    // Find player index - assumes createRectangle adds the object and sets its 'isPlayer' flag correctly.
    // And that the player is the last one added if multiple calls to createRectangle happen for player.
    // A more robust way would be for createRectangle to return the object or its index if it's special.
    // For now, search by playerBodyId or rely on the isPlayer flag set by init.
    for (size_t i = 0; i < gameObjects.size(); ++i) {
        if (gameObjects[i].isPlayer && B2_ID_EQUALS(gameObjects[i].bodyId, playerBodyId)) {
            playerIndex = static_cast<int>(i);
            break;
        }
    }
    if (playerIndex == -1 && !B2_IS_NULL(playerBodyId)) { // Fallback if isPlayer wasn't set by createRectangle immediately
         for (size_t i = 0; i < gameObjects.size(); ++i) {
            if (B2_ID_EQUALS(gameObjects[i].bodyId, playerBodyId)) {
                 // This assumes the GameObject corresponding to playerBodyId is indeed the player.
                 // The isPlayerObject flag in createRectangle should handle this.
                 // gameObjects[i].isPlayer = true; // Ensure it's marked if not already
                 playerIndex = static_cast<int>(i);
                 break;
            }
        }
    }


    // Pushable Box
    float boxSizeM = pixelsToMeters(40);
    float boxXM = pixelsToMeters(400);
    float boxYM = groundYM + groundHeightM / 2.0f + boxSizeM / 2.0f + pixelsToMeters(1); // Position slightly above ground
    createRectangle(worldId, gameObjects, boxXM, boxYM, boxSizeM, boxSizeM,
                    true, sf::Color::Red, false, 0.2f, 1.0f, 0.7f, 0.1f,
                    false, true, true); // isPlayer=false, canJumpOn=true, doPlayerCollide=true


    // --- Hanging Platform ---
    float anchorX_m = pixelsToMeters(1500);
    float anchorY_m = pixelsToMeters(500); // Position of the anchor point
    b2BodyId anchorBodyId = createAnchorBody(worldId, anchorX_m, anchorY_m); // Static anchor point for the rope

    float platWidthM = pixelsToMeters(120);
    float platHeightM = pixelsToMeters(20);
    float platX_m = anchorX_m;
    float platY_m = anchorY_m - pixelsToMeters(150); // Position platform below the anchor
    
    b2BodyId platformBodyId = createRectangle(worldId, gameObjects, platX_m, platY_m, platWidthM, platHeightM,
                                             true, sf::Color(160, 82, 45), false, 0.5f, 1.0f, 0.7f, 0.1f,
                                             false, true, true); // isPlayer=false, canJumpOn=true, doPlayerCollide=true

    if (!B2_IS_NULL(platformBodyId)) {
        b2MassData myMassData;
        myMassData.mass = 2.34375f;
        myMassData.center = (b2Vec2){0.0f, 0.0f}; // Local center of mass
        myMassData.rotationalInertia = 5.0f;
        b2Body_SetMassData(platformBodyId, myMassData);
    }

    // --- Create Segmented Rope for Hanging Platform ---
    const int numRopeSegments = 10;
    float segmentThicknessM = pixelsToMeters(8); // Thickness of each rope segment

    if (!B2_IS_NULL(anchorBodyId) && !B2_IS_NULL(platformBodyId)) {
        b2Vec2 platformAttachPointLocal = {0.0f, platHeightM / 2.0f}; // Attach to top-center of platform
        
        createSegmentedRope(worldId, gameObjects,
                            anchorBodyId, {0.0f, 0.0f}, // BodyA and its local anchor
                            platformBodyId, platformAttachPointLocal, // BodyB and its local anchor
                            numRopeSegments,
                            0.0f, // segmentPrimaryDim will be calculated by the function
                            segmentThicknessM,
                            true, // isVerticalOrientation
                            sf::Color(139, 69, 19),
                            0.2f, 0.05f, 0.5f, 0.1f,
                            false, false); // segmentsCanBeJumpedOn=false, segmentsCollideWithPlayer=true
    }
    // --- End Segmented Rope Creation for Hanging Platform ---
    
    // --- Create Horizontal Rope Bridge ---
    const int numHSegments = 20; // Number of segments in the horizontal rope
    b2Vec2 leftAnchorPos  = { pixelsToMeters(100), pixelsToMeters(200) };
    b2Vec2 rightAnchorPos = { pixelsToMeters(800), pixelsToMeters(200) };

    b2BodyId leftAnchor  = createAnchorBody(worldId, leftAnchorPos.x,  leftAnchorPos.y);
    b2BodyId rightAnchor = createAnchorBody(worldId, rightAnchorPos.x, rightAnchorPos.y);

    float hSegmentThickness = pixelsToMeters(3); // Thickness of each segment (vertical dimension for horizontal rope)

    if (!B2_IS_NULL(leftAnchor) && !B2_IS_NULL(rightAnchor)) {
        createSegmentedRope(worldId, gameObjects,
                            leftAnchor, {0.0f, 0.0f},  // BodyA and its local anchor
                            rightAnchor, {0.0f, 0.0f}, // BodyB and its local anchor
                            numHSegments,
                            0.0f, // segmentPrimaryDim will be calculated
                            hSegmentThickness,
                            false, // isVerticalOrientation = false for horizontal bridge
                            sf::Color::Yellow,
                            1.0f, 1.0f, 0.0f, 1.0f,
                            true, true); // segmentsCanBeJumpedOn=true (for a bridge), segmentsCollideWithPlayer=true
    }
    // --- End Horizontal Rope Bridge Creation ---
    return playerIndex;
}

#endif // MAP1_HPP
