#ifndef TREMPLIN_HPP
#define TREMPLIN_HPP

#include "../game_object.hpp"
#include <vector>
#include <SFML/Graphics.hpp>
#include <iostream>


/**
 * @brief Creates a segmented rope connecting two bodies.
 *
 * @param worldId The Box2D world ID.
 * @param gameObjects Reference to the vector storing all game objects (rope segments will be added here).
 * @param bodyA The first body to attach the rope to.
 * @param localAnchorA Local attachment point on bodyA.
 * @param bodyB The second body to attach the rope to.
 * @param localAnchorB Local attachment point on bodyB.
 * @param numSegments Number of segments in the rope (must be >= 1).
 * @param segmentPrimaryDim Length of each segment along the rope's primary axis.
 * @param segmentSecondaryDim Thickness of each segment.
 * @param isVerticalOrientation True if segments are primarily vertical (primaryDim=height, secondaryDim=width),
 *                              False if segments are primarily horizontal (primaryDim=width, secondaryDim=height).
 * @param color Color of the rope segments.
 * @param segmentLinearDamping Linear damping for segments (default: 0.2f).
 * @param segmentDensity Density for segments (default: 0.05f).
 * @param segmentFriction Friction for segments (default: 0.5f).
 * @param segmentRestitution Restitution for segments (default: 0.1f).
 * @param segmentsCanBeJumpedOn Can player jump on these rope segments? (default: false)
 * @param segmentsCollideWithPlayer Should player collide with these rope segments? (default: true)
 * @return True if the rope was created successfully, false otherwise.
 */
inline void createTremplin(
    b2WorldId worldId,
    std::vector<GameObject>& gameObjects,
    bool is_dynamic,
    float x_m, float y_m) {

    GameObject tremplinObj;
    GameObject tremplinObj2;
    GameObject tremplinSensor;

    // Tremplin dimensions (140x50 pixels)
    float tremplinWidthM = pixelsToMeters(140.0f);
    float tremplinHeightM = pixelsToMeters(50.0f);

    tremplinObj.setPosition(x_m, y_m);
    tremplinObj.setSize(tremplinWidthM+pixelsToMeters(14), tremplinHeightM-pixelsToMeters(6));
    tremplinObj.setDynamic(is_dynamic);
    tremplinObj.setColor(sf::Color::Transparent); // Fallback color if sprite fails
    
    tremplinObj.setCanJumpOnProperty(false);
    tremplinObj.setCollidesWithPlayerProperty(true);
    //tremplinObj.setIsTremplinProperty(true);
    //tremplinObj.setIsSensorProperty(true); // Make the tremplin a sensor
    //tremplinObj.setEnableSensorEventsProperty(true); // Enable sensor events for the tremplin

    // Default physics properties for a static sensor-like object
    tremplinObj.setFriction(0.7f);
    tremplinObj.setRestitution(0.1f);
    
    // Collision properties are handled by setIstremplinProperty and setIsPlayerProperty

    tremplinObj2.setPosition(x_m, y_m);
    tremplinObj2.setSize(tremplinWidthM, tremplinHeightM);
    tremplinObj2.setDynamic(is_dynamic);
    tremplinObj2.setCanJumpOnProperty(true);
    tremplinObj2.setCollidesWithPlayerProperty(true);
    tremplinObj2.setColor(sf::Color::Transparent);
    tremplinObj2.setFriction(0.0f);
    tremplinObj2.setRestitution(0.45f);



    tremplinSensor.setPosition(x_m , y_m);
    tremplinSensor.setSize(tremplinWidthM, tremplinHeightM);
    tremplinSensor.setDynamic(is_dynamic);
    tremplinSensor.setCanJumpOnProperty(false);

    tremplinSensor.setSpriteTexturePath("../assets/sprite/objects/tremplin-1.png"); // Path to tremplin image
    tremplinSensor.setIsTremplinProperty(true);
    tremplinSensor.setIsSensorProperty(true);
    tremplinSensor.setEnableSensorEventsProperty(true);



    if (tremplinObj.finalize(worldId)) {
        gameObjects.push_back(tremplinObj);
    }

    if (tremplinObj2.finalize(worldId)) {
        gameObjects.push_back(tremplinObj2);
    }

    if (tremplinSensor.finalize(worldId)) {
        gameObjects.push_back(tremplinSensor);
    }

}  


#endif // TREMPLIN_HPP
