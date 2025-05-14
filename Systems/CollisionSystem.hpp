#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include <memory>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"
#include "../Components.hpp"


class CollisionSystem {
private:
    std::shared_ptr<ECS> ecs;
    const float DAMPING_STATIC = 1.0f;
    const float DAMPING_DYNAMIC = 0.8f;

public:
    CollisionSystem(std::shared_ptr<ECS> ecs) : ecs(ecs) {}

    void detectCollisions() {
        for (Entity entityA = 0; entityA < MAX_ENTITIES; entityA++) {
            for (Entity entityB = 0; entityB < MAX_ENTITIES; entityB++) {
                if (entityA == entityB) continue; // Skip self-collisin
                // Rigid Rect collision detection
                if (ecs->hasComponent<RigidRect>(entityA) && ecs->hasComponent<RigidRect>(entityB)) {
                    RigidRect& rectA = ecs->getData<RigidRect>(entityA);
                    RigidRect& rectB = ecs->getData<RigidRect>(entityB);
                    if (rectA.isStatic && rectB.isStatic) continue; // we can't move static bodies
                    if (rectA.isStatic || rectB.isStatic) { // Dynamic vs Static
                        resolveDynamicVsStaticRect(
                            rectA.isStatic ? rectB : rectA,
                            rectA.isStatic ? rectA : rectB
                        );
                    }
                    // Dynamic vs Dynamic
                    resolveDynamicVsDynamicRect(entityA, entityB);
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

    void resolveDynamicVsDynamicRect(Entity entityA, Entity entityB) {
    }

    void resolveDynamicVsStaticRect(RigidRect& dynamicRect, RigidRect& staticRect) {
        Vec2f rPos = staticRect.positions[0];
        float size1 = (staticRect.positions[1] - staticRect.positions[0]).length();
        float size2 = (staticRect.positions[3] - staticRect.positions[0]).length();
        Vec2f rSize = Vec2f(size1, size2);
        Vec2f rCenter = rPos + rSize * 0.5f;
        for (int i = 0; i < 4; i++) {
            Vec2f& p = dynamicRect.predictedPositions[i];
            Vec2f delta = p -rCenter;
            Vec2f half = rSize * 0.5f;

            float overlapX = half.x - std::abs(delta.x);
            float overlapY = half.y - std::abs(delta.y);
            
            Vec2f& v = dynamicRect.velocities[i];

            if (overlapX>0 && overlapY>0) {
                std::cout << "Collision detected between dynamic and static rect" << "\n";
                if (overlapX<overlapY) {
                    float sign = (delta.x > 0) ? 1.0f : -1.0f;
                    p.x += sign * overlapX;
                } else {
                    float sign = (delta.y > 0) ? 1.0f : -1.0f;
                    p.y += sign * overlapY;
                }
            }
        }
    }

};
#endif // COLLISION_SYSTEM_HPP