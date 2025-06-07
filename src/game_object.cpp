#include "game_object.hpp" // Includes SFML, Box2D, utils.hpp, constants.hpp
#include <iostream> // For error reporting
#include <cmath> // For M_PI / b2_pi

/**
 * @brief Default constructor for GameObject.
 * Initializes bodyId and shapeId to null and hasVisual to false.
 */
GameObject::GameObject() : bodyId(b2_nullBodyId), shapeId(b2_nullShapeId), hasVisual(false), canJumpOn(false),
                           isPlayer(false), /* sprite is default-initialized (empty optional) */ currentFrame(0), animationTimer(0.0f), spriteFlipped(false) {}

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
    this->isPlayer = isPlayerObject; // Set the player flag
    this->canJumpOn = canJumpOnObject; 

    // If it's a player, we might not want the default sfShape visual, or use it for debug.
    // For now, sfShape is always initialized. Drawing logic will decide.
    hasVisual = true; 

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
 * @brief Loads a sequence of textures for a player animation state.
 * @param name The name of the animation (e.g., "idle", "walk").
 * @param framePaths Vector of file paths to the textures for each frame.
 * @param frameDuration The time each frame should be displayed.
 */
void GameObject::loadPlayerAnimation(const std::string& name, const std::vector<std::string>& framePaths, float frameDuration) {
    if (!isPlayer) return;
    std::vector<sf::Texture> textures;
    for (const std::string& path : framePaths) {
        sf::Texture tex;
        if (tex.loadFromFile(path)) {
            textures.push_back(tex);
        } else {
            std::cerr << "Failed to load texture: " << path << " for animation: " << name << std::endl;
        }
    }
    if (!textures.empty()) {
        animations[name] = textures;
        animationFrameDurations[name] = frameDuration;
    }
}

/**
 * @brief Sets the current player animation.
 * @param name The name of the animation to play.
 * @param flipped True if the sprite should be flipped horizontally (facing left).
 */
void GameObject::setPlayerAnimation(const std::string& name, bool flipped) {
    if (!isPlayer || animations.find(name) == animations.end()) return;

    if (currentAnimationName != name || spriteFlipped != flipped) {
        currentAnimationName = name;
        spriteFlipped = flipped;
        currentFrame = 0;
        animationTimer = 0.0f;

        if (!animations[currentAnimationName].empty()) {
            sf::Texture& tex = animations[currentAnimationName][currentFrame];
            if (!sprite) { // If sprite is not yet constructed
                sprite.emplace(tex); // Construct it with the texture
            } else {
                sprite->setTexture(tex); // Otherwise, just set the texture
            }
            sprite->setOrigin({static_cast<float>(tex.getSize().x) / 2.f, static_cast<float>(tex.getSize().y) / 2.f});
        } else if (sprite) { 
            // No frames for this animation, but sprite exists. 
            // Option 1: Reset the optional, effectively removing the sprite.
            sprite.reset(); 
            // Option 2: Keep the sprite but clear its texture (if SFML allows setting a "null" or empty texture state).
            // For now, resetting is cleaner if no texture means no visible sprite.
        }
    }
}

/**
 * @brief Updates the current player animation frame based on delta time.
 * @param dt Delta time since the last frame.
 */
void GameObject::updatePlayerAnimation(float dt) {
    if (!isPlayer || !sprite || currentAnimationName.empty() || animations.find(currentAnimationName) == animations.end()) {
        return;
    }

    const auto& animFrames = animations[currentAnimationName];
    if (animFrames.size() <= 1) { // Single frame animation or no frames
        if (!animFrames.empty() && (&sprite->getTexture() != &animFrames[0])) { // Compare addresses
             sprite->setTexture(animFrames[0]); // Ensure correct texture is set
             sprite->setOrigin({static_cast<float>(animFrames[0].getSize().x) / 2.f, static_cast<float>(animFrames[0].getSize().y) / 2.f});
        }
        return;
    }

    animationTimer += dt;
    float frameDuration = animationFrameDurations[currentAnimationName];

    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame = (currentFrame + 1) % animFrames.size();
        sprite->setTexture(animFrames[currentFrame]);
        sprite->setOrigin({static_cast<float>(animFrames[currentFrame].getSize().x) / 2.f, static_cast<float>(animFrames[currentFrame].getSize().y) / 2.f});
    }
}


/**
 * @brief Updates the SFML shape's position and rotation based on the Box2D body.
 * Also updates the player sprite if applicable.
 */
void GameObject::updateShape() {
    if (B2_IS_NULL(bodyId)) return;

    b2Transform transform = b2Body_GetTransform(bodyId);
    sf::Vector2f sfmlPos = b2VecToSfVec(transform.p);

    // Update sfShape (can be used for non-player objects or as debug visual)
    if (hasVisual) {
        sfShape.setPosition(sfmlPos);
        // Convert Box2D angle (radians, CCW positive) to SFML angle (degrees, CW positive)
        float angleDegreesFloat = -b2Rot_GetAngle(transform.q) * 180.0f / B2_PI;
        sfShape.setRotation(sf::degrees(angleDegreesFloat));
    }

    // Update player sprite
    if (isPlayer && sprite && sprite->getTexture().getSize() != sf::Vector2u(0,0)) {
        sprite->setPosition(sfmlPos);
        sprite->setScale({spriteFlipped ? -1.f : 1.f, 1.f});
        sprite->setRotation(sf::degrees(0.f)); 
    }
}

/**
 * @brief Draws the GameObject's SFML shape or sprite to the given render window.
 * @param window The SFML render window to draw on.
 */
void GameObject::draw(sf::RenderWindow& window) const {
    if (isPlayer && sprite && sprite->getTexture().getSize() != sf::Vector2u(0,0)) {
        window.draw(*sprite); // Draw the dereferenced optional
    } else if (hasVisual && !B2_IS_NULL(bodyId)) { // Fallback or for non-player objects
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
