#include "game_object.hpp" // Includes SFML, Box2D, utils.hpp
#include <iostream> // For error reporting
#include <cmath> // For M_PI / b2_pi

GameObject::GameObject() : bodyId(b2_nullBodyId), shapeId(b2_nullShapeId), hasVisual(false) {}

bool GameObject::init(b2WorldId worldId, float x_m, float y_m, float width_m, float height_m,
                      bool isDynamic, sf::Color color,
                      bool fixedRotation, float linearDamping,
                      float density, float friction, float restitution) {
    hasVisual = true;
    sfShape.setSize({metersToPixels(width_m), metersToPixels(height_m)});
    sfShape.setFillColor(color);
    sfShape.setOrigin({metersToPixels(width_m) / 2.0f, metersToPixels(height_m) / 2.0f});
    // Initial position, will be updated by updateShape() if body is valid
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
        std::cerr << "Error creating body!" << std::endl;
        hasVisual = false;
        return false;
    }

    b2Polygon box = b2MakeBox(width_m / 2.0f, height_m / 2.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = isDynamic ? density : 0.0f;
    shapeDef.material.friction = friction;
    shapeDef.material.restitution = restitution;

    shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
    if (B2_IS_NULL(shapeId)) {
        std::cerr << "Error creating shape!" << std::endl;
        b2DestroyBody(bodyId); // Clean up created body
        bodyId = b2_nullBodyId;
        hasVisual = false;
        return false;
    }
    return true;
}

void GameObject::updateShape() {
    if (!hasVisual || B2_IS_NULL(bodyId)) return;
    b2Transform transform = b2Body_GetTransform(bodyId);
    sfShape.setPosition(b2VecToSfVec(transform.p));
    // Box2D angles are in radians, SFML in degrees. Box2D positive = CCW, SFML positive = CW.
    float angleDegrees = -b2Rot_GetAngle(transform.q) * 180.0f / B2_PI;
    sfShape.setRotation(sf::degrees(angleDegrees));
}

void GameObject::draw(sf::RenderWindow& window) const {
    if (hasVisual && !B2_IS_NULL(bodyId)) {
        window.draw(sfShape);
    }
}

bool GameObject::isValid() const {
    return !B2_IS_NULL(bodyId);
}

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
