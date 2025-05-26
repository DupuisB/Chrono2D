#ifndef PRIMITIVES_RECTANGLE_HPP
#define PRIMITIVES_RECTANGLE_HPP

#include "../game_object.hpp" // Access to GameObject class and Box2D/SFML
#include <vector>
#include <SFML/Graphics.hpp> // For sf::Color

/**
 * @brief Creates a rectangular GameObject and adds it to the game.
 *
 * @param worldId The Box2D world ID.
 * @param gameObjects Reference to the vector storing all game objects.
 * @param x_m Initial x-position in meters.
 * @param y_m Initial y-position in meters.
 * @param width_m Width of the rectangle in meters.
 * @param height_m Height of the rectangle in meters.
 * @param isDynamic True if the body should be dynamic, false for static.
 * @param color Color of the SFML shape.
 * @param fixedRotation True to prevent rotation (default: false).
 * @param linearDamping Linear damping (default: 0.0f).
 * @param density Density (default: 1.0f).
 * @param friction Friction (default: 0.7f).
 * @param restitution Restitution (default: 0.1f).
 * @param isPlayerObject Is this rectangle the player object? (default: false).
 * @param canJumpOn Can the player jump on this rectangle? (default: false for static, true for dynamic, but should be explicit).
 * @param doPlayerCollide Should the player collide with this rectangle? (default: true).
 * @return b2BodyId of the created rectangle, or b2_nullBodyId on failure.
 */
inline b2BodyId createRectangle(
    b2WorldId worldId,
    std::vector<GameObject>& gameObjects,
    float x_m, float y_m, float width_m, float height_m,
    bool isDynamic, sf::Color color,
    bool fixedRotation = false, float linearDamping = 0.0f,
    float density = 1.0f, float friction = 0.7f, float restitution = 0.1f,
    bool isPlayerObject = false, bool canJumpOn = false, bool doPlayerCollide = true) {
    
    GameObject rectObj;
    if (rectObj.init(worldId, x_m, y_m, width_m, height_m, isDynamic, color,
                     fixedRotation, linearDamping, density, friction, restitution,
                     isPlayerObject, canJumpOn, doPlayerCollide)) {
        gameObjects.push_back(rectObj);
        return rectObj.bodyId;
    }
    return b2_nullBodyId;
}

#endif // PRIMITIVES_RECTANGLE_HPP
