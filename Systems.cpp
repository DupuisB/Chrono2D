#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

#include "ECS/ECS.hpp"
#include "utils/math.hpp"
#include "Components.cpp"

class PhysicsSystem {
private:
    ECS* ecs;
    const float gravity = 9.8f;
    const float timeStep = 1.0f / 60.0f;

public:
    PhysicsSystem(ECS* ecs) : ecs(ecs) {}

    void update(float deltaTime) {
        // Process all entities with Transform and RigidBody components
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (ecs->hasComponent<Transform>(entity) && ecs->hasComponent<RigidBody>(entity)) {
                Transform& transform = ecs->getData<Transform>(entity);
                RigidBody& rigidBody = ecs->getData<RigidBody>(entity);

                // Skip static objects
                if (rigidBody.isStatic) {
                    continue;
                }

                // Apply gravity
                transform.acceleration.y = gravity;

                // Update velocity based on acceleration
                transform.velocity += transform.acceleration * deltaTime;

                // Update position based on velocity
                transform.position += transform.velocity * deltaTime;

                // Reset acceleration
                transform.acceleration = Vec2f(0.0f, 0.0f);
            }
        }
    }
};

#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include "ECS/ECS.hpp"
#include "utils/math.hpp"

class CollisionSystem {
private:
    ECS* ecs;

public:
    CollisionSystem(ECS* ecs) : ecs(ecs) {}

    void update() {
        for (Entity a = 0; a < MAX_ENTITIES; ++a) {
            if (!ecs->hasComponent<Transform>(a) ||
                !ecs->hasComponent<Shape>(a) ||
                !ecs->hasComponent<RigidBody>(a)) continue;

            for (Entity b = a + 1; b < MAX_ENTITIES; ++b) {
                if (!ecs->hasComponent<Shape>(b) ||
                    !ecs->hasComponent<RigidBody>(b)) continue;

                if (ecs->hasComponent<Wall>(b)) {
                    handleWallCollision(a, b);
                }

                handleEntityCollision(a, b);
            }
        }
    }

private:
    bool isCircle(const Shape& shape) const {
        return dynamic_cast<sf::CircleShape*>(shape.shape.get()) != nullptr;
    }

    bool isRectangle(const Shape& shape) const {
        return dynamic_cast<sf::RectangleShape*>(shape.shape.get()) != nullptr;
    }

    bool checkRectCollision(const Vec2f& apos, const Shape& sa, const Vec2f& bpos, const Shape& sb) const {
        auto* ra = dynamic_cast<sf::RectangleShape*>(sa.shape.get());
        auto* rb = dynamic_cast<sf::RectangleShape*>(sb.shape.get());
        if (!ra || !rb) return false;

        sf::Vector2f sizeA = ra->getSize();
        sf::Vector2f sizeB = rb->getSize();

        // change pos from center to top-left
        Vec2f posA = apos - sizeA / 2.0f;
        Vec2f posB = bpos - sizeB / 2.0f;

        bool overlapX = std::abs(posA.x - posB.x) < (sizeA.x / 2 + sizeB.x / 2);
        bool overlapY = std::abs(posA.y - posB.y) < (sizeA.y / 2 + sizeB.y / 2);

        return overlapX && overlapY;
    }

    void handleWallCollision(Entity a, Entity b) {
        // entity b will always be the wall
        Transform& ta = ecs->getData<Transform>(a);
        Shape& sa = ecs->getData<Shape>(a);
        Wall& wall = ecs->getData<Wall>(b);

        // finish this

    }

    void resolveCollision(Transform& ta, RigidBody& ra, Transform& tb, RigidBody& rb) {
        if (ra.isStatic && rb.isStatic) return;

        const float damping = 0.8f;
        Vec2f normal = (tb.position - ta.position).normalized();

        // One static
        if (ra.isStatic && !rb.isStatic) {
            float vb = tb.velocity.dot(normal);
            if (vb < 0) {
                tb.velocity -= (1 + damping) * vb * normal;
            }
            return;
        }

        if (!ra.isStatic && rb.isStatic) {
            float va = ta.velocity.dot(normal);
            if (va > 0) {
                ta.velocity -= (1 + damping) * va * normal;
            }
            return;
        }

        // Both dynamic
        Vec2f relativeVelocity = ta.velocity - tb.velocity;
        float velAlongNormal = relativeVelocity.dot(normal);

        if (velAlongNormal > 0) return; // Moving apart

        float m1 = ra.mass;
        float m2 = rb.mass;
        float impulse = (2.0f * velAlongNormal) / (m1 + m2);

        ta.velocity -= impulse * m2 * normal;
        tb.velocity += impulse * m1 * normal;
    }
};


class RenderSystem {
private:
    ECS* ecs;
    sf::RenderWindow* window;

public:
    RenderSystem(ECS* ecs, sf::RenderWindow* window) : ecs(ecs), window(window) {}

    void update() {
        window->clear(sf::Color(30, 30, 30));
        
        // Render all entities with Transform and Shape components
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (ecs->hasComponent<Shape>(entity)) {
                Shape& shape = ecs->getData<Shape>(entity);
                
                if (ecs->hasComponent<Transform>(entity)) {
                    Transform& transform = ecs->getData<Transform>(entity);
                    
                    // Update shape position
                    if (shape.shape) {
                        shape.shape->setPosition({transform.position.x - shape.shape->getSize().x / 2, transform.position.y - shape.shape->getSize().y / 2});
                    }
                } else if (ecs->hasComponent<Wall>(entity)) {
                    Wall& wall = ecs->getData<Wall>(entity);
                    
                    // Update shape position
                    if (shape.shape) {
                        shape.shape->setPosition({wall.position.x - shape.shape->getSize().x / 2, wall.position.y - shape.shape->getSize().y / 2});
                    }
                }
                window->draw(*shape.shape);
            }
        }
        
        window->display();
    }
};