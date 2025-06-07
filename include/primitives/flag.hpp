#ifndef PRIMITIVES_FLAG_HPP
#define PRIMITIVES_FLAG_HPP

#include "../game_object.hpp"
#include <vector>
#include <SFML/Graphics.hpp>
#include <iostream>

/**
 * @brief Creates a flag GameObject and adds it to the game.
 * The flag serves as a level completion marker.
 *
 * @param worldId The Box2D world ID.
 * @param gameObjects Reference to the vector storing all game objects.
 * @param x_m Initial x-position of the flag's center in meters.
 * @param y_m Initial y-position of the flag's center in meters.
 * @return b2BodyId of the created flag, or b2_nullBodyId on failure.
 */
inline b2BodyId createFlag(
    b2WorldId worldId,
    std::vector<GameObject>& gameObjects,
    float x_m, float y_m) {

    GameObject flagObj;

    // Flag dimensions (80x120 pixels)
    float flagWidthM = pixelsToMeters(80.0f);
    float flagHeightM = pixelsToMeters(120.0f);

    flagObj.setPosition(x_m, y_m);
    flagObj.setSize(flagWidthM, flagHeightM);
    flagObj.setDynamic(false); // Flag is static
    flagObj.setColor(sf::Color::Yellow); // Fallback color if sprite fails
    
    flagObj.setIsFlagProperty(true); // Mark as flag for collision and logic
    flagObj.setSpriteTexturePath("../assets/objects/flag.png"); // Path to flag image
    flagObj.setIsSensorProperty(true); // Make the flag a sensor
    flagObj.setEnableSensorEventsProperty(true); // Enable sensor events for the flag

    // Default physics properties for a static sensor-like object
    flagObj.setFriction(0.0f);
    flagObj.setRestitution(0.0f);
    
    // Collision properties are handled by setIsFlagProperty and setIsPlayerProperty

    if (flagObj.finalize(worldId)) {
        gameObjects.push_back(flagObj);
        GameObject& actualFlagInVector = gameObjects.back(); // Get a reference to the flag in the vector
        actualFlagInVector.ensureCorrectSpriteTextureLink(); 

        return actualFlagInVector.bodyId; // Return the bodyId of the object in the vector
    }
    std::cerr << "Failed to create flag object." << std::endl;
    return b2_nullBodyId;
}

#endif // PRIMITIVES_FLAG_HPP
