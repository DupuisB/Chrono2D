#ifndef PHYSICS_SYSTEM_HPP
#define PHYSICS_SYSTEM_HPP

#include <memory>
#include <array>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"
#include "../Components.hpp"


class PhysicsSystem {
private:
    std::shared_ptr<ECS> ecs;
    const Vec2f GRAVITY = Vec2f(0, 10.0f);
    std::array<Vec2f, MAX_ENTITIES> externForce;

public:
    PhysicsSystem(std::shared_ptr<ECS> ecs) : ecs(ecs) {}

    void update(float dt) {
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            // Rigid Rect physics
            if (ecs->hasComponent<RigidRect>(entity)) {
                RigidRect& rigidRect = ecs->getData<RigidRect>(entity);
                if (rigidRect.isStatic) {
                    continue;
                }
                for (int i = 0; i < 4; i++) {
                    rigidRect.accelerations[i] = (GRAVITY + externForce[entity]/rigidRect.mass);
                    rigidRect.velocities[i] += rigidRect.accelerations[i] * dt;
                    rigidRect.predictedPositions[i] = rigidRect.positions[i] + rigidRect.velocities[i] * dt;
                }
            }
        }
    }

    void applyForce(Entity entity, const Vec2f& force) {
        externForce[entity] = force;
    }

    void removeForce(Entity entity) {
        externForce[entity] = Vec2f(0, 0);
    }

    void PBDupdate(float dt) {
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (ecs->hasComponent<RigidRect>(entity)) {
                RigidRect& rigidRect = ecs->getData<RigidRect>(entity);
                if (rigidRect.isStatic) {
                    continue;
                }
                for (int i = 0; i < 4; i++) {
                    rigidRect.velocities[i] = (rigidRect.predictedPositions[i] - rigidRect.positions[i]) / dt;
                    rigidRect.positions[i] = rigidRect.predictedPositions[i];
                }
            }
        }
    }

};
#endif // PHYSICS_SYSTEM_HPP