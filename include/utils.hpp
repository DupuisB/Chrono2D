#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/System/Vector2.hpp>
#include <box2d/box2d.h>
#include "constants.hpp"
#include <cmath>

// Conversion Functions

/**
 * @brief Converts a value from pixels to meters.
 * @param pixels The value in pixels.
 * @return The corresponding value in meters.
 */
inline float pixelsToMeters(float pixels) {
    return pixels / PIXELS_PER_METER;
}

/**
 * @brief Converts a value from meters to pixels.
 * @param meters The value in meters.
 * @return The corresponding value in pixels.
 */
inline float metersToPixels(float meters) {
    return meters * PIXELS_PER_METER;
}

/**
 * @brief Converts a Box2D vector (b2Vec2) to an SFML vector (sf::Vector2f).
 * Handles scaling from meters to pixels and optionally flips the Y-coordinate
 * to match SFML's coordinate system (origin top-left, Y positive downwards).
 * @param vec The Box2D vector (typically in meters).
 * @param scale If true (default), scales the vector from meters to pixels.
 * @param flipY If true (default), flips the Y-coordinate (e.g., SFML_Y = WINDOW_HEIGHT - Box2D_Y_pixels).
 * @return The converted SFML vector.
 */
inline sf::Vector2f b2VecToSfVec(b2Vec2 vec, bool scale = true, bool flipY = true) {
    float sfX = scale ? metersToPixels(vec.x) : vec.x;
    float sfY = scale ? metersToPixels(vec.y) : vec.y;
    if (flipY) {
        // Box2D typically has Y positive upwards, SFML has Y positive downwards.
        // This conversion assumes Box2D's origin is at the bottom-left for world coordinates
        // when mapping to SFML's top-left origin.
        return sf::Vector2f(sfX, WINDOW_HEIGHT - sfY);
    } else {
         return sf::Vector2f(sfX, sfY);
    }
}

/**
 * @brief Converts an SFML vector (sf::Vector2f) to a Box2D vector (b2Vec2).
 * Handles scaling from pixels to meters and optionally flips the Y-coordinate
 * to match Box2D's coordinate system (origin bottom-left, Y positive upwards).
 * @param vec The SFML vector (typically in pixels).
 * @param scale If true (default), scales the vector from pixels to meters.
 * @param flipY If true (default), flips the Y-coordinate (e.g., Box2D_Y_pixels = WINDOW_HEIGHT - SFML_Y).
 * @return The converted Box2D vector.
 */
inline b2Vec2 sfVecToB2Vec(sf::Vector2f vec, bool scale = true, bool flipY = true) {
    float b2X = scale ? pixelsToMeters(vec.x) : vec.x;
    float b2Y_intermediate = flipY ? WINDOW_HEIGHT - vec.y : vec.y; // Y in pixel space, potentially flipped
    float b2Y = scale ? pixelsToMeters(b2Y_intermediate) : b2Y_intermediate; // Convert to meters
    return {b2X, b2Y};
}

#endif
