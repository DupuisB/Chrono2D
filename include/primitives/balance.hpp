#ifndef PRIMITIVES_BALANCE_HPP
#define PRIMITIVES_BALANCE_HPP
#include "../game_object.hpp" // Access to GameObject class and Box2D/SFML
#include <vector>
#include <SFML/Graphics.hpp> // For sf::Color



inline b2BodyId createBalance(
    b2WorldId worldId,
    std::vector<GameObject>& gameObjects,
    float x_m, float y_m, float width_m, float height_m,
    bool isDynamic, sf::Color color,
    bool fixedRotation = false, float linearDamping = 0.0f,
    float density = 1.0f, float friction = 0.7f, float restitution = 0.1f,
    bool isPlayerObject = false, bool canJumpOn = false, bool doPlayerCollide = true,
    bool isBalance = false) {  // Add this parameter
    
    GameObject rectObj;

    rectObj.setPosition(x_m, y_m);
    rectObj.setSize(width_m, height_m);
    rectObj.setDynamic(isDynamic);
    if (isDynamic) {
        rectObj.setDensity(density);
    }
    
    rectObj.setColor(color);
    rectObj.setFixedRotation(false); // Balance must be able to rotate
    rectObj.setLinearDamping(linearDamping);
    rectObj.setFriction(friction);
    rectObj.setRestitution(restitution);
    
    // Configure gameplay and collision properties
    rectObj.setIsPlayerProperty(isPlayerObject);
    rectObj.setCanJumpOnProperty(canJumpOn);
    if (!isPlayerObject) { 
        rectObj.setCollidesWithPlayerProperty(doPlayerCollide);
    }

    if (rectObj.finalize(worldId)) {
        gameObjects.push_back(rectObj);
        
        if (isBalance) {
            // Create a static anchor point at the center
            b2BodyDef anchorDef = b2DefaultBodyDef();
            anchorDef.position = {x_m, y_m};
            anchorDef.type = b2_staticBody;
            b2BodyId anchorBodyId = b2CreateBody(worldId, &anchorDef);
            
            // Create revolute joint to pin the rectangle to the anchor
            b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
            jointDef.bodyIdA = anchorBodyId;
            jointDef.bodyIdB = rectObj.bodyId;
            jointDef.localAnchorA = {0.0f, 0.0f}; // Center of anchor
            jointDef.localAnchorB = {0.0f, 0.0f}; // Center of rectangle
            jointDef.enableLimit = false; // Allow full rotation
            
            b2CreateRevoluteJoint(worldId, &jointDef);
        }
        
        return rectObj.bodyId;
    }
    return b2_nullBodyId;
}

#endif