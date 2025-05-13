#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "utils.hpp" // Includes constants.hpp

class GameObject {
public:
    b2BodyId bodyId;
    b2ShapeId shapeId;
    sf::RectangleShape sfShape;
    bool hasVisual;

    GameObject();

    bool init(b2WorldId worldId, float x_m, float y_m, float width_m, float height_m,
              bool isDynamic, sf::Color color,
              bool fixedRotation = false, float linearDamping = 0.0f,
              float density = 1.0f, float friction = 0.7f, float restitution = 0.1f);

    void updateShape();
    void draw(sf::RenderWindow& window) const;
    bool isValid() const;
};

// Helper function to create static anchor bodies
b2BodyId createAnchorBody(b2WorldId worldId, float x_m, float y_m);

#endif
