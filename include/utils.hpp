#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/System/Vector2.hpp>
#include <box2d/box2d.h>
#include "constants.hpp"
#include <cmath>

// Conversion Functions
inline float pixelsToMeters(float pixels) {
    return pixels / PIXELS_PER_METER;
}

inline float metersToPixels(float meters) {
    return meters * PIXELS_PER_METER;
}

inline sf::Vector2f b2VecToSfVec(b2Vec2 vec, bool scale = true, bool flipY = true) {
    float sfX = scale ? metersToPixels(vec.x) : vec.x;
    float sfY = scale ? metersToPixels(vec.y) : vec.y;
    if (flipY) {
        return sf::Vector2f(sfX, WINDOW_HEIGHT - sfY);
    } else {
         return sf::Vector2f(sfX, sfY);
    }
}

inline b2Vec2 sfVecToB2Vec(sf::Vector2f vec, bool scale = true, bool flipY = true) {
    float b2X = scale ? pixelsToMeters(vec.x) : vec.x;
    float b2Y = flipY ? WINDOW_HEIGHT - vec.y : vec.y;
    b2Y = scale ? pixelsToMeters(b2Y) : b2Y;
    return {b2X, b2Y};
}

#endif
