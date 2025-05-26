#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "utils.hpp" // Includes constants.hpp

/**
 * @brief Represents a game entity with both a physical (Box2D) and visual (SFML) component.
 */
class GameObject {
public:
    b2BodyId bodyId;        ///< ID of the Box2D rigid body.
    b2ShapeId shapeId;      ///< ID of the Box2D shape attached to the body.
    sf::RectangleShape sfShape; ///< SFML shape for visual representation.
    bool hasVisual;         ///< Flag indicating if the object has a visual component.
    bool canJumpOn;       ///< Flag indicating if the player can jump off this object.
    // bool doPlayerCollide; ///< This is implicitly handled by collision filtering setup.

    /**
     * @brief Default constructor.
     */
    GameObject();

    /**
     * @brief Initializes the GameObject.
     * Creates a Box2D body and shape, and sets up the SFML visual component.
     * Also configures collision filtering and jump properties.
     * @param worldId The Box2D world ID.
     * @param x_m Initial x-position in meters.
     * @param y_m Initial y-position in meters.
     * @param width_m Width of the object in meters.
     * @param height_m Height of the object in meters.
     * @param isDynamic True if the body should be dynamic, false for static.
     * @param color Color of the SFML shape.
     * @param fixedRotation True to prevent the body from rotating (default: false).
     * @param linearDamping Damping to reduce linear velocity over time (default: 0.0f).
     * @param density Density of the body's material (default: 1.0f).
     * @param friction Friction coefficient (default: 0.7f).
     * @param restitution Restitution (bounciness) coefficient (default: 0.1f).
     * @param isPlayerObject True if this game object represents the player.
     * @param canJumpOnObject True if the player can jump from this object.
     * @param doPlayerCollideWithObject True if the player should collide with this object.
     * @return True if initialization was successful, false otherwise.
     */
    bool init(b2WorldId worldId, float x_m, float y_m, float width_m, float height_m,
              bool isDynamic, sf::Color color,
              bool fixedRotation = false, float linearDamping = 0.0f,
              float density = 1.0f, float friction = 0.7f, float restitution = 0.1f,
              bool isPlayerObject = false, bool canJumpOnObject = false, bool doPlayerCollideWithObject = true);

    /**
     * @brief Updates the SFML shape's position and rotation from the Box2D body.
     * Must be called each frame before drawing.
     */
    void updateShape();

    /**
     * @brief Draws the SFML shape to the render window.
     * @param window The SFML RenderWindow to draw on.
     */
    void draw(sf::RenderWindow& window) const;

    /**
     * @brief Checks if the GameObject has a valid Box2D body.
     * @return True if the bodyId is not null, false otherwise.
     */
    bool isValid() const;
};

/**
 * @brief Creates a static anchor body at a specified world position.
 * Useful for attaching joints to a fixed point in the world.
 * @param worldId The ID of the Box2D world.
 * @param x_m The x-coordinate of the anchor in meters.
 * @param y_m The y-coordinate of the anchor in meters.
 * @return The b2BodyId of the created anchor body, or b2_nullBodyId on failure.
 */
b2BodyId createAnchorBody(b2WorldId worldId, float x_m, float y_m);

#endif
