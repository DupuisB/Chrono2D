#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

/**
 * @file constants.hpp
 * @brief Defines global constants used throughout the application.
 * This includes settings for map selection, window dimensions, and physics scaling.
 */

// --- Map Selection ---
#define SELECTED_MAP 1 // Set this to the desired map number

// --- Window ---
const unsigned int WINDOW_WIDTH  = 1600;
const unsigned int WINDOW_HEIGHT = 1200;
const float PIXELS_PER_METER = 32.0f;

// --- Game Loop ---
const float UPDATE_DELTA = 1.0f / 60.0f;


// --- Physics Collision Categories ---
// For simplicity in this setup, we'll use const.
const uint64_t CATEGORY_PLAYER = 0x0001;
const uint64_t CATEGORY_WORLD  = 0x0002;
const uint64_t CATEGORY_FLAG   = 0x0004;
const uint64_t CATEGORY_TREMPLIN = 0x0005;

#endif // CONSTANTS_HPP
