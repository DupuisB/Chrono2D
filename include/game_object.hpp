#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "utils.hpp" // Includes constants.hpp
#include <map>
#include <string>
#include <vector>
#include <optional> // Required for std::optional

/**
 * @brief Represents a game entity with both a physical (Box2D) and visual (SFML) component.
 */
class GameObject {
public:
    // --- Properties to be set before finalize() ---
    // Position and Size (meters)
    float x_m_ {0.0f};
    float y_m_ {0.0f};
    float width_m_ {1.0f};
    float height_m_ {1.0f};

    // Physics properties
    bool isDynamic_val_ {false};
    bool fixedRotation_val_ {false};
    float linearDamping_val_ {0.0f};
    float density_val_ {1.0f};
    float friction_val_ {0.7f};
    float restitution_val_ {0.1f};

    // Gameplay and Collision properties
    bool isPlayer_prop_ {false}; // Property to guide setup, distinct from 'isPlayer' animation flag
    bool canJumpOn_prop_ {false}; // Property to guide setup
    bool collidesWithPlayer_prop_ {true}; // Property to guide setup
    bool isFlag_prop_ {false}; // Property to identify a flag object
    bool isTremplin_prop_ {false}; //Property to identify a tremplin object
    bool isSensor_prop_ {false}; // Property to make the shape a sensor
    bool enableSensorEvents_prop_ {false}; // Property to enable sensor events for this shape
    uint64_t categoryBits_ {CATEGORY_WORLD}; // Changed to uint64_t
    uint64_t maskBits_ {CATEGORY_PLAYER | CATEGORY_WORLD | CATEGORY_TREMPLIN}; // Changed to uint64_t
    b2Vec2 pendingImpulsion {b2Vec2{0.0f,0.0f}}; //Memorizes if some impulsion must be applied to the object


    // --- SFML/Box2D members ---
    b2BodyId bodyId;
    b2ShapeId shapeId;
    sf::RectangleShape sfShape;
    sf::Color color_val_ {sf::Color::White}; // Visual property
    bool hasVisual;
    bool canJumpOn; // Actual gameplay flag, set during finalize
    bool isFlag_ {false}; // Actual flag, set during finalize
    bool isTremplin {false};

    // Sprite and Animation specific (primarily for Player)
    std::optional<sf::Sprite> sprite;
    bool isPlayer; // Flag to identify the player object for animation, set during finalize
    std::map<std::string, std::vector<sf::Texture>> animations; // e.g., "idle" -> {texture_idle}, "walk" -> {walk_tex1, walk_tex2}
    std::map<std::string, float> animationFrameDurations; // e.g., "walk" -> 0.15f (seconds per frame)
    sf::Texture genericTexture_; // Texture for non-animated sprites (e.g., flag)
    std::string spriteTexturePath_prop_; // Path for generic sprite texture


    // Store original shape definition for dynamic resizing (player only)
    struct PlayerShapeInfo {
        float density;
        float friction;
        float restitution;
        b2Filter filter;
    };
    std::optional<PlayerShapeInfo> playerShapeInfo; // Store if this is a player

    std::string currentAnimationName;
    int currentFrame;
    float animationTimer;
    bool spriteFlipped; // True if sprite should be flipped horizontally (facing left)


    /**
     * @brief Default constructor.
     */
    GameObject();

    // --- Setters for properties ---
    void setPosition(float x, float y);
    void setSize(float w, float h);
    void setDynamic(bool dynamic);
    void setColor(sf::Color c);
    void setFixedRotation(bool fixed);
    void setLinearDamping(float damping);
    void setDensity(float d);
    void setFriction(float f);
    void setRestitution(float r);
    
    // Gameplay/Collision Setters
    void setIsPlayerProperty(bool isPlayerProp); // Sets the property used for initial setup
    void setCanJumpOnProperty(bool canJumpOnProp); // Sets the property used for initial setup
    void setCollidesWithPlayerProperty(bool collidesProp); // Sets the property used for initial setup
    void setIsFlagProperty(bool isFlagProp); // Sets the property for flag identification
    void setIsTremplinProperty(bool isTremplinProp); // Sets the property for tremplin identification
    void setSpriteTexturePath(const std::string& path); // Sets path for generic sprite
    void setCollisionFilterData(uint64_t category, uint64_t mask); // Changed to uint64_t
    void setIsSensorProperty(bool isSensorProp); // Sets the property to make the shape a sensor
    void setEnableSensorEventsProperty(bool enableSensorEventsProp); // Sets property to enable sensor events
    void setPendingImpulsion(b2Vec2 impulsion); // Sets the pending impulsion


    // --- Finalization ---
    /**
     * @brief Finalizes the GameObject by creating its Box2D body and shape,
     * and setting up SFML visuals based on previously set properties.
     * @param worldId The Box2D world ID.
     * @return True if finalization was successful, false otherwise.
     */
    bool finalize(b2WorldId worldId);


    // Methods for player animation
    void loadPlayerAnimation(const std::string& name, const std::vector<std::string>& framePaths, float frameDuration);
    void setPlayerAnimation(const std::string& name, bool flipped); // 'flipped' is true if facing left
    void updatePlayerAnimation(float dt);
    void updateTremplinAnimation(float dt);

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

    /**
     * @brief Ensures the sprite's internal texture pointer is correctly linked
     * to this GameObject's own texture members. Crucial after copying a GameObject.
     */
    void ensureCorrectSpriteTextureLink();
};

#endif
