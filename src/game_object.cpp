#include "game_object.hpp" // Includes SFML, Box2D, utils.hpp, constants.hpp
#include <iostream> // For error reporting
#include <cmath> // For M_PI / b2_pi

/**
 * @brief Default constructor for GameObject.
 * Initializes bodyId and shapeId to null and hasVisual to false.
 */
GameObject::GameObject() : bodyId(b2_nullBodyId), shapeId(b2_nullShapeId), hasVisual(false), canJumpOn(false) {}

/**
 * @brief Initializes the GameObject with a physical body and visual representation.
 * @param worldId The Box2D world ID.
 * @param x_m Initial x-position in meters.
 * @param y_m Initial y-position in meters.
 * @param width_m Width of the object in meters.
 * @param height_m Height of the object in meters.
 * @param isDynamic True if the body should be dynamic, false for static.
 * @param color Color of the SFML shape.
 * @param fixedRotation True to prevent the body from rotating.
 * @param linearDamping Damping to reduce linear velocity over time.
 * @param density Density of the body's material.
 * @param friction Friction coefficient of the body's material.
 * @param restitution Restitution (bounciness) of the body's material.
 * @param isPlayerObject True if this game object represents the player.
 * @param canJumpOnObject True if the player can jump from this object.
 * @param doPlayerCollideWithObject True if the player should collide with this object.
 * @return True if initialization was successful, false otherwise.
 */
bool GameObject::init(b2WorldId worldId, float x_m, float y_m, float width_m, float height_m,
                      bool isDynamic, sf::Color color,
                      bool fixedRotation, float linearDamping,
                      float density, float friction, float restitution,
                      bool isPlayerObject, bool canJumpOnObject, bool doPlayerCollideWithObject) {
    hasVisual = true;
    this->canJumpOn = canJumpOnObject; // Store the canJumpOn status

    sfShape.setSize({metersToPixels(width_m), metersToPixels(height_m)});
    sfShape.setFillColor(color);
    sfShape.setOrigin({metersToPixels(width_m) / 2.0f, metersToPixels(height_m) / 2.0f});
    // Set initial SFML shape position; this will be updated by updateShape() if the body is valid.
    sfShape.setPosition(b2VecToSfVec({x_m, y_m}));


    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = isDynamic ? b2_dynamicBody : b2_staticBody;
    bodyDef.position = {x_m, y_m};
    if (isDynamic) {
         bodyDef.fixedRotation = fixedRotation;
         bodyDef.linearDamping = linearDamping;
    }
    bodyId = b2CreateBody(worldId, &bodyDef);
    if (B2_IS_NULL(bodyId)) {
        std::cerr << "Error creating Box2D body for GameObject!" << std::endl;
        hasVisual = false;
        return false;
    }

    b2Polygon box = b2MakeBox(width_m / 2.0f, height_m / 2.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = isDynamic ? density : 0.0f; // Static bodies should have zero density
    shapeDef.material.friction = friction;
    shapeDef.material.restitution = restitution;

    // Setup collision filtering
    if (isPlayerObject) {
        shapeDef.filter.categoryBits = CATEGORY_PLAYER;
        shapeDef.filter.maskBits = CATEGORY_WORLD; // Player collides with world objects
    } else {
        shapeDef.filter.categoryBits = CATEGORY_WORLD;
        if (doPlayerCollideWithObject) {
            shapeDef.filter.maskBits = CATEGORY_PLAYER | CATEGORY_WORLD; // World object collides with player and other world objects
        } else {
            shapeDef.filter.maskBits = CATEGORY_WORLD; // World object collides only with other world objects
        }
    }

    shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
    if (B2_IS_NULL(shapeId)) {
        std::cerr << "Error creating Box2D shape for GameObject!" << std::endl;
        b2DestroyBody(bodyId); // Clean up the created body if shape creation fails
        bodyId = b2_nullBodyId;
        hasVisual = false;
        return false;
    }
    return true;
}

/**
 * @brief Updates the SFML shape's position and rotation based on the Box2D body.
 */
void GameObject::updateShape() {
    if (!hasVisual || B2_IS_NULL(bodyId)) return;

    b2Transform transform = b2Body_GetTransform(bodyId);
    sfShape.setPosition(b2VecToSfVec(transform.p));

    // Convert Box2D angle (radians, CCW positive) to SFML angle (degrees, CW positive)
    float angleDegrees = -b2Rot_GetAngle(transform.q) * 180.0f / B2_PI;
    sfShape.setRotation(sf::degrees(angleDegrees));
}

/**
 * @brief Draws the GameObject's SFML shape to the given render window.
 * @param window The SFML render window to draw on.
 */
void GameObject::draw(sf::RenderWindow& window) const {
    if (hasVisual && !B2_IS_NULL(bodyId)) {
        window.draw(sfShape);
    }
}

/**
 * @brief Checks if the GameObject has a valid Box2D body.
 * @return True if the bodyId is not null, false otherwise.
 */
bool GameObject::isValid() const {
    return !B2_IS_NULL(bodyId);
}

/**
 * @brief Helper function to create a static anchor body in the Box2D world.
 * Useful for joints that need a fixed point in the world.
 * @param worldId The Box2D world ID.
 * @param x_m The x-position of the anchor in meters.
 * @param y_m The y-position of the anchor in meters.
 * @return The b2BodyId of the created static anchor body, or b2_nullBodyId if creation failed.
 */
b2BodyId createAnchorBody(b2WorldId worldId, float x_m, float y_m) {
     b2BodyDef bodyDef = b2DefaultBodyDef();
     bodyDef.type = b2_staticBody;
     bodyDef.position = {x_m, y_m};
     b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
     if (B2_IS_NULL(bodyId)) {
         std::cerr << "Error creating anchor body!" << std::endl;
     }
     return bodyId;
}
