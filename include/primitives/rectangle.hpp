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
 * @param groundPlatformIds Optional: Pointer to a vector to add this object's bodyId if it's considered ground.
 * @return b2BodyId of the created rectangle, or b2_nullBodyId on failure.
 */
inline b2BodyId createRectangle(
    b2WorldId worldId,
    std::vector<GameObject>& gameObjects,
    float x_m, float y_m, float width_m, float height_m,
    bool isDynamic, sf::Color color,
    bool fixedRotation = false, float linearDamping = 0.0f,
    float density = 1.0f, float friction = 0.7f, float restitution = 0.1f,
    std::vector<b2BodyId>* groundPlatformIds = nullptr) {
    
    GameObject rectObj;
    if (rectObj.init(worldId, x_m, y_m, width_m, height_m, isDynamic, color,
                     fixedRotation, linearDamping, density, friction, restitution)) {
        gameObjects.push_back(rectObj);
        if (groundPlatformIds) {
            groundPlatformIds->push_back(rectObj.bodyId);
        }
        return rectObj.bodyId;
    }
    return b2_nullBodyId;
}

#endif // PRIMITIVES_RECTANGLE_HPP
