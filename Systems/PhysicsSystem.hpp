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
            if (ecs->hasComponent<Velocity>(entity)) {
                Acceleration& acceleration = ecs->getData<Acceleration>(entity);
                Velocity& velocity = ecs->getData<Velocity>(entity);
                Position& position = ecs->getData<Position>(entity);
                Mass& mass = ecs->getData<Mass>(entity);
                PredictedPosition& predictedPosition = ecs->getData<PredictedPosition>(entity);
                for (int i = 0; i < position.positions.size(); i++) {
                    acceleration.accelerations[i] = GRAVITY + externForce[entity] / mass.m;
                    velocity.velocities[i] += acceleration.accelerations[i] * dt;
                    predictedPosition.predictedPositions[i] = position.positions[i] + velocity.velocities[i] * dt;
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
            if (ecs->hasComponent<Velocity>(entity)) {
                Position& position = ecs->getData<Position>(entity);
                PredictedPosition& predictedPosition = ecs->getData<PredictedPosition>(entity);
                Velocity& velocity = ecs->getData<Velocity>(entity);
                for (int i = 0; i < position.positions.size(); i++) {
                    velocity.velocities[i] = (predictedPosition.predictedPositions[i] - position.positions[i]) / dt;
                    position.positions[i] = predictedPosition.predictedPositions[i];
                }
            }
        }
    }

};
#endif // PHYSICS_SYSTEM_HPP