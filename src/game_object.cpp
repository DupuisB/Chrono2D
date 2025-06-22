#include "game_object.hpp" // Includes SFML, Box2D, utils.hpp, constants.hpp
#include <iostream> // For error reporting
#include <cmath> // For M_PI / b2_pi

/**
 * @brief Default constructor for GameObject.
 * Initializes bodyId and shapeId to null, hasVisual to false,
 * and sets default values for configurable properties.
 */
GameObject::GameObject() : bodyId(b2_nullBodyId), shapeId(b2_nullShapeId), hasVisual(false), canJumpOn(false),
                           isPlayer(false), currentFrame(0), animationTimer(0.0f), spriteFlipped(false) {
    // Default property values are set via member initialization in the header.
    // Or can be explicitly set here if preferred.
    // Example: x_m_ = 0.0f; y_m_ = 0.0f; width_m_ = 1.0f; etc.
}

GameObject::GameObject(const GameObject& other)
    : x_m_(other.x_m_), y_m_(other.y_m_), width_m_(other.width_m_), height_m_(other.height_m_),
      isDynamic_val_(other.isDynamic_val_), fixedRotation_val_(other.fixedRotation_val_),
      linearDamping_val_(other.linearDamping_val_), density_val_(other.density_val_),
      friction_val_(other.friction_val_), restitution_val_(other.restitution_val_),
      isPlayer_prop_(other.isPlayer_prop_), canJumpOn_prop_(other.canJumpOn_prop_),
      collidesWithPlayer_prop_(other.collidesWithPlayer_prop_), isFlag_prop_(other.isFlag_prop_),
      isTremplin_prop_(other.isTremplin_prop_), isSensor_prop_(other.isSensor_prop_),
      enableSensorEvents_prop_(other.enableSensorEvents_prop_), categoryBits_(other.categoryBits_),
      maskBits_(other.maskBits_), pendingImpulsion(other.pendingImpulsion),
      bodyId(other.bodyId), shapeId(other.shapeId), sfShape(other.sfShape),
      color_val_(other.color_val_), hasVisual(other.hasVisual), canJumpOn(other.canJumpOn),
      isFlag_(other.isFlag_), isTremplin(other.isTremplin), sprite(other.sprite),
      isPlayer(other.isPlayer), animations(other.animations),
      animationFrameDurations(other.animationFrameDurations), genericTexture_(other.genericTexture_),
      spriteTexturePath_prop_(other.spriteTexturePath_prop_), playerShapeInfo(other.playerShapeInfo),
      currentAnimationName(other.currentAnimationName), currentFrame(other.currentFrame),
      animationTimer(other.animationTimer), spriteFlipped(other.spriteFlipped) {
    
    // After copying, re-link the sprite's texture pointer to the new object's texture.
    ensureCorrectSpriteTextureLink();
}

// --- Property Setters ---
void GameObject::setPosition(float x, float y) {
    x_m_ = x;
    y_m_ = y;
}

void GameObject::setSize(float w, float h) {
    width_m_ = w;
    height_m_ = h;
}

void GameObject::setDynamic(bool dynamic) {
    isDynamic_val_ = dynamic;
    if (!isDynamic_val_) { // Static bodies typically have 0 density by convention in Box2D
        density_val_ = 0.0f;
    }
}

void GameObject::setColor(sf::Color c) {
    color_val_ = c;
    if (hasVisual) { // If sfShape already exists (e.g. finalize was called, then color changed)
        sfShape.setFillColor(color_val_);
    }
}

void GameObject::setFixedRotation(bool fixed) {
    fixedRotation_val_ = fixed;
    if (!B2_IS_NULL(bodyId) && isDynamic_val_) {
        b2Body_SetFixedRotation(bodyId, fixedRotation_val_);
    }
}

void GameObject::setLinearDamping(float damping) {
    linearDamping_val_ = damping;
    if (!B2_IS_NULL(bodyId) && isDynamic_val_) {
        b2Body_SetLinearDamping(bodyId, linearDamping_val_);
    }
}

void GameObject::setDensity(float d) {
    density_val_ = d;
    // Note: Density is used at shape creation. Changing it live requires
    // destroying and recreating the shape or using b2Shape_SetDensity
    // and then b2Body_ResetMassData or similar, if available and appropriate.
    // For this refactor, density is set before finalize().
}

void GameObject::setFriction(float f) {
    friction_val_ = f;
    if (!B2_IS_NULL(shapeId)) {
        b2Shape_SetFriction(shapeId, friction_val_);
    }
}

void GameObject::setRestitution(float r) {
    restitution_val_ = r;
    if (!B2_IS_NULL(shapeId)) {
        b2Shape_SetRestitution(shapeId, restitution_val_);
    }
}

void GameObject::setPendingImpulsion(b2Vec2 impulse) {
    pendingImpulsion = impulse;
}

void GameObject::setIsPlayerProperty(bool isPlayerProp) {
    isPlayer_prop_ = isPlayerProp;
    // This might also influence default collision filter bits if called before finalize
    if (isPlayer_prop_) {
        categoryBits_ = CATEGORY_PLAYER;
        maskBits_ = CATEGORY_WORLD | CATEGORY_FLAG | CATEGORY_TREMPLIN; // Player collides with world and flag and tremplin
    } else {
        // Revert to default world object if it was player before
        // This logic depends on how you want setIsPlayerProperty to behave regarding filters
        categoryBits_ = CATEGORY_WORLD;
        maskBits_ = CATEGORY_PLAYER | CATEGORY_WORLD | CATEGORY_TREMPLIN; // Default for world object
        if (!collidesWithPlayer_prop_) { // If it shouldn't collide with player
             maskBits_ = CATEGORY_WORLD;
        }
    }

    if (!B2_IS_NULL(shapeId)) { // If shape exists, update filter
        b2Filter filter;
        filter.categoryBits = categoryBits_;
        filter.maskBits = maskBits_;
        filter.groupIndex = 0; // Default group
        b2Shape_SetFilter(shapeId, filter);
    }
    if (!B2_IS_NULL(bodyId)) { // Update the GameObject's own 'isPlayer' animation flag
        this->isPlayer = isPlayer_prop_;
    }
}

void GameObject::setCanJumpOnProperty(bool canJumpOnProp) {
    canJumpOn_prop_ = canJumpOnProp;
    if (!B2_IS_NULL(bodyId)) { // Update the GameObject's own 'canJumpOn' gameplay flag
         this->canJumpOn = canJumpOn_prop_;
    }
}

void GameObject::setCollidesWithPlayerProperty(bool collidesProp) {
    collidesWithPlayer_prop_ = collidesProp;
    if (!isPlayer_prop_) { // Only relevant if this object is not the player itself
        if (collidesWithPlayer_prop_) {
            maskBits_ = CATEGORY_PLAYER | CATEGORY_WORLD | CATEGORY_TREMPLIN; // Default: collides with player and world
            if (isFlag_prop_) { // If it's a flag, it specifically collides with player
                maskBits_ = CATEGORY_PLAYER;
            }
        } else {
            maskBits_ = CATEGORY_WORLD | CATEGORY_TREMPLIN; // Only collides with world
        }
        if (!B2_IS_NULL(shapeId)) { // If shape exists, update filter
            b2Filter filter;
            filter.categoryBits = categoryBits_; // categoryBits_ should be CATEGORY_WORLD or CATEGORY_FLAG
            filter.maskBits = maskBits_;
            filter.groupIndex = 0;
            b2Shape_SetFilter(shapeId, filter);
        }
    }
}

void GameObject::setIsFlagProperty(bool isFlagProp) {
    isFlag_prop_ = isFlagProp;
    if (isFlag_prop_) {
        categoryBits_ = CATEGORY_FLAG;
        maskBits_ = CATEGORY_PLAYER; // Flag collides with Player
    } else {
        // If it's not a flag, and also not a player, revert to default world object
        if (!isPlayer_prop_) {
            categoryBits_ = CATEGORY_WORLD;
            maskBits_ = CATEGORY_PLAYER | CATEGORY_WORLD | CATEGORY_TREMPLIN;
            if (!collidesWithPlayer_prop_) {
                 maskBits_ = CATEGORY_WORLD;
            }
         }
    }
    // Filter update will happen in finalize or if setCollisionFilterData is called later
}


void GameObject::setIsTremplinProperty(bool isTremplinProp) {
    isTremplin_prop_ = isTremplinProp;
    if (isTremplin_prop_) {
        categoryBits_ = CATEGORY_TREMPLIN;
        maskBits_ = CATEGORY_PLAYER | CATEGORY_WORLD;// Tremplin collides with Player and objects
    } else {
        // If it's not a tremplin, and also not a player, revert to default world object
        if (!isPlayer_prop_) {
            categoryBits_ = CATEGORY_WORLD;
            maskBits_ = CATEGORY_PLAYER | CATEGORY_WORLD | CATEGORY_TREMPLIN;
            if (!collidesWithPlayer_prop_) {
                 maskBits_ = CATEGORY_WORLD;
            }
        }
    }
    // Filter update will happen in finalize or if setCollisionFilterData is called later
}

void GameObject::setSpriteTexturePath(const std::string& path) {
    spriteTexturePath_prop_ = path;
}

void GameObject::setCollisionFilterData(uint64_t category, uint64_t mask) {
    categoryBits_ = category;
    maskBits_ = mask;
    if (!B2_IS_NULL(shapeId)) {
        b2Filter filter;
        filter.categoryBits = categoryBits_;
        filter.maskBits = maskBits_;
        filter.groupIndex = 0;
        b2Shape_SetFilter(shapeId, filter);
    }
}

void GameObject::setIsSensorProperty(bool isSensorProp) {
    isSensor_prop_ = isSensorProp;
}

void GameObject::setEnableSensorEventsProperty(bool enableSensorEventsProp) {
    enableSensorEvents_prop_ = enableSensorEventsProp;
}


// --- Finalization ---
bool GameObject::finalize(b2WorldId worldId) {
    if (!B2_IS_NULL(bodyId)) {
        std::cerr << "GameObject already finalized or has a body." << std::endl;
        return false; // Already finalized
    }

    // Setup SFML Shape
    sfShape.setSize(sf::Vector2f(metersToPixels(width_m_), metersToPixels(height_m_)));
    sfShape.setFillColor(color_val_);
    sfShape.setOrigin(sf::Vector2f(metersToPixels(width_m_) / 2.0f, metersToPixels(height_m_) / 2.0f));
    sfShape.setPosition(b2VecToSfVec({x_m_, y_m_})); // Initial position
    hasVisual = true;

    // Create Box2D Body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = isDynamic_val_ ? b2_dynamicBody : b2_staticBody;
    bodyDef.position = {x_m_, y_m_};
    if (isDynamic_val_) {
        bodyDef.fixedRotation = fixedRotation_val_;
        bodyDef.linearDamping = linearDamping_val_;
    }
    bodyId = b2CreateBody(worldId, &bodyDef);
    if (B2_IS_NULL(bodyId)) {
        std::cerr << "Error creating Box2D body for GameObject!" << std::endl;
        hasVisual = false;
        return false;
    }

    // Create Box2D Shape
    b2Polygon box = b2MakeBox(width_m_ / 2.0f, height_m_ / 2.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density_val_; // density_val_ should be 0 for static if isDynamic_val_ is false
    shapeDef.material.friction = friction_val_;
    shapeDef.material.restitution = restitution_val_;
    shapeDef.isSensor = isSensor_prop_; // Use the property here
    shapeDef.enableSensorEvents = enableSensorEvents_prop_; // Use the property here

    // Setup collision filtering based on properties
    shapeDef.filter.categoryBits = categoryBits_;
    shapeDef.filter.maskBits = maskBits_;
    shapeDef.filter.groupIndex = 0; // Default group index

    shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
    if (B2_IS_NULL(shapeId)) {
        std::cerr << "Error creating Box2D shape for GameObject!" << std::endl;
        b2DestroyBody(bodyId); // Clean up
        bodyId = b2_nullBodyId;
        hasVisual = false;
        return false;
    }

    // Set internal gameplay flags
    this->isPlayer = isPlayer_prop_;
    this->canJumpOn = canJumpOn_prop_;
    this->isFlag_ = isFlag_prop_;
    this->isTremplin = isTremplin_prop_;

    // Load generic sprite if path is provided and not a player object
    if (!isPlayer && !spriteTexturePath_prop_.empty()) {
        bool loadSuccess = genericTexture_.loadFromFile(spriteTexturePath_prop_);
        if (loadSuccess) {
            sprite.emplace(genericTexture_); // Construct the sprite with the loaded texture
            sf::Vector2u textureSize = genericTexture_.getSize();
            sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
        } else {
            std::cerr << "Failed to load generic texture from path: " << spriteTexturePath_prop_ << std::endl;
        }
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
                sprite->setTexture(tex);
            }
            sf::Vector2u textureSize = tex.getSize();
            sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
        } else if (sprite) { 
            // No frames for this animation, but sprite exists.
            sprite.reset(); 
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
        if (!animFrames.empty() && (&sprite->getTexture() != &animFrames[0])) {
             sprite->setTexture(animFrames[0]); // Ensure correct texture is set
             sf::Vector2u textureSize = animFrames[0].getSize();
             sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
        }
        return;
    }

    animationTimer += dt;
    float frameDuration = animationFrameDurations[currentAnimationName];

    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame = (currentFrame + 1) % animFrames.size();
        sprite->setTexture(animFrames[currentFrame]);
        sf::Vector2u textureSize = animFrames[currentFrame].getSize();
        sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
    }
}


/**
 * @brief Updates the current player animation frame based on delta time.
 * @param dt Delta time since the last frame.
 */
void GameObject::updateTremplinAnimation(float dt) {
    if (!sprite || currentAnimationName.empty() || animations.find(currentAnimationName) == animations.end()) {
        return;
    }

    const auto& animFrames = animations[currentAnimationName];
    if (animFrames.size() <= 1) { // Single frame animation or no frames
        if (!animFrames.empty() && (&sprite->getTexture() != &animFrames[0])) {
             sprite->setTexture(animFrames[0]); // Ensure correct texture is set
             sf::Vector2u textureSize = animFrames[0].getSize();
             sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
        }
        return;
    }

    animationTimer += dt;
    float frameDuration = animationFrameDurations[currentAnimationName];

    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame = (currentFrame + 1) % animFrames.size();
        sprite->setTexture(animFrames[currentFrame]);
        sf::Vector2u textureSize = animFrames[currentFrame].getSize();
        sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
    }
}



/**
 * @brief Updates the SFML shape's position and rotation based on the Box2D body.
 * Also updates the player sprite if applicable.
 */
void GameObject::updateShape() {
    if (B2_IS_NULL(bodyId)) return;

    if (isDynamic_val_) {
        b2Body_ApplyLinearImpulseToCenter(bodyId, pendingImpulsion, true);
        setPendingImpulsion(b2Vec2{pendingImpulsion.x/1.1f, pendingImpulsion.y/1.1f});
    };

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
    if (isPlayer && sprite.has_value() && sprite->getTexture().getSize() != sf::Vector2u(0,0)) {
        sprite->setPosition(sfmlPos);
        // Calculate proper scale based on GameObject size vs texture size
        sf::Vector2u textureSize = sprite->getTexture().getSize();
        float scaleX = metersToPixels(width_m_) / static_cast<float>(textureSize.x);
        float scaleY = metersToPixels(height_m_) / static_cast<float>(textureSize.y);
        sprite->setScale(sf::Vector2f(spriteFlipped ? -scaleX : scaleX, scaleY));
        sprite->setRotation(sf::degrees(0.f)); 
    } 
    // Update generic sprite for non-player objects (e.g., flag, box)
    else if (!isPlayer && sprite.has_value() && sprite->getTexture().getSize() != sf::Vector2u(0,0)) {
        sprite->setPosition(sfmlPos);
        // Calculate proper scale based on GameObject size vs texture size
        sf::Vector2u textureSize = sprite->getTexture().getSize();
        float scaleX = metersToPixels(width_m_) / static_cast<float>(textureSize.x);
        float scaleY = metersToPixels(height_m_) / static_cast<float>(textureSize.y);
        sprite->setScale(sf::Vector2f(scaleX, scaleY));
        sprite->setRotation(sf::degrees(0.f)); // Assuming non-player objects don't rotate
    }
}

/**
 * @brief Draws the GameObject's SFML shape or sprite to the given render window.
 * @param window The SFML render window to draw on.
 */
void GameObject::draw(sf::RenderWindow& window) const {

    if (isPlayer && sprite.has_value() && sprite->getTexture().getSize() != sf::Vector2u(0,0)) {
        window.draw(*sprite);
    } else if (!isPlayer && sprite.has_value() && sprite->getTexture().getSize() != sf::Vector2u(0,0)) { // Draw generic sprite
        window.draw(*sprite);
    } else if (hasVisual && !B2_IS_NULL(bodyId)) {
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

void GameObject::ensureCorrectSpriteTextureLink() {
    if (sprite.has_value()) {
        if (!isPlayer && !spriteTexturePath_prop_.empty()) {
            // For generic sprites, re-link to genericTexture_
            if (genericTexture_.getSize().x > 0 && genericTexture_.getSize().y > 0) {
                sprite->setTexture(genericTexture_, true); // true to reset texture rect
                
                // Set proper origin
                sf::Vector2u textureSize = genericTexture_.getSize();
                sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
                
                // Scale will be handled in updateShape()
            } else {
                sprite.reset(); // Cannot use the sprite if its intended texture is bad.
            }
        } else if (isPlayer && !currentAnimationName.empty() && animations.count(currentAnimationName)) {
            // For player sprites, re-link to the texture in the animations map
            const auto& animFrames = animations[currentAnimationName];
            if (currentFrame >= 0 && static_cast<size_t>(currentFrame) < animFrames.size()) {
                if (animFrames[currentFrame].getSize().x > 0 && animFrames[currentFrame].getSize().y > 0) {
                    sprite->setTexture(animFrames[currentFrame], true);
                    
                    // Set proper origin
                    sf::Vector2u textureSize = animFrames[currentFrame].getSize();
                    sprite->setOrigin(sf::Vector2f(static_cast<float>(textureSize.x) / 2.f, static_cast<float>(textureSize.y) / 2.f));
                    
                    // Scale will be handled in updateShape()
                } else {
                    std::cerr << "Invalid texture for current frame in animation: " << currentAnimationName << std::endl;
                    sprite.reset();
                }
            } else {
                std::cerr << "Current frame index out of bounds for animation: " << currentAnimationName << std::endl;
                sprite.reset();
            }
        }
    }
}
