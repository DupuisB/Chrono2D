#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include <memory>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"
#include "../Components.hpp"


class CollisionSystem {
private:
    std::shared_ptr<ECS> ecs;
    const float DAMPING_STATIC = 0.5f;
    const float DAMPING_DYNAMIC = 0.8f;

public:
    CollisionSystem(std::shared_ptr<ECS> ecs) : ecs(ecs) {}

    void detectCollisions() {
        for (Entity entityA = 0; entityA < MAX_ENTITIES; entityA++) {
            if (ecs->hasComponent<Body>(entityA)) {
                Body& bodyA = ecs->getData<Body>(entityA);
                if (bodyA.isStatic) {
                    continue; // Skip static bodies
                }
                
                for (Entity entityB = 0; entityB < MAX_ENTITIES; entityB++) {
                    if (ecs->hasComponent<Body>(entityB)) {
                        Body& bodyB = ecs->getData<Body>(entityB);
                        if (bodyB.isStatic) {
                            resolveDynamicVsStatic(entityA, entityB);
                        }

                        // Perform collision detection between entityA and entityB
                        // This is a placeholder for actual collision detection logic
                        // You would typically check the bounding boxes or shapes of the entities here
                    }
                }
            }
        }
    }

    // Returns wether 2 rects formed by A/B and C/D intersect
    bool AABB(const Vec2f& A, const Vec2f& Asize, const Vec2f& B, const Vec2f& Bsize) {
        return (A.x + Asize.x >= B.x &&
                A.x <= B.x + Bsize.x &&
                A.y + Asize.y >= B.y &&
                A.y <= B.y + Bsize.y);
    }

    bool pointInRect(const Vec2f& point, const Vec2f& rectPos, const Vec2f& rectSize) {
        return (point.x >= rectPos.x &&
                point.x <= rectPos.x + rectSize.x &&
                point.y >= rectPos.y &&
                point.y <= rectPos.y + rectSize.y);
    }

    void resolveDynamicVsStatic(Entity dynamicEntity, Entity staticEntity) {
        Vec2f& p = ecs->getData<Dynamic>(dynamicEntity).predictedPosition;
        Vec2f rPos = ecs->getData<Composite>(staticEntity).points[0];
        Vec2f rSize = ecs->getData<Composite>(staticEntity).size;

        Vec2f rCenter = rPos + rSize * 0.5f;
        Vec2f delta = p - rCenter;
        Vec2f half = rSize * 0.5f;

        float overlapX = half.x - std::abs(delta.x);
        float overlapY = half.y - std::abs(delta.y);

        Vec2f& v = ecs->getData<Dynamic>(dynamicEntity).velocity;
        if (overlapX > 0 && overlapY > 0) {

            std::cout << "Collision detected between dynamic entity " << dynamicEntity << " and static entity " << staticEntity << std::endl;
            if (overlapX < overlapY) {
                float sign = (delta.x < 0) ? -1.0f : 1.0f;
                p.x += overlapX * sign;
                //v.x = -v.x * DAMPING_STATIC;
            } else {
                p.y += -overlapY;
                //v.y = -v.y * DAMPING_STATIC;
            }
        }
    }

};
#endif // COLLISION_SYSTEM_HPP