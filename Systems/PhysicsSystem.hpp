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
            if (ecs->hasComponent<Dynamic>(entity)) {
                Dynamic& dynamic = ecs->getData<Dynamic>(entity);
                Body& body = ecs->getData<Body>(entity);
                dynamic.acceleration = (GRAVITY + externForce[entity]/body.mass);
                dynamic.velocity += dynamic.acceleration * dt;
                dynamic.predictedPosition = dynamic.position + dynamic.velocity * dt;
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
            if (ecs->hasComponent<Dynamic>(entity)) {
                Dynamic& dynamic = ecs->getData<Dynamic>(entity);
                dynamic.velocity = (dynamic.predictedPosition - dynamic.position) / dt;
                dynamic.position = dynamic.predictedPosition;
                if (ecs->hasComponent<Renderable>(entity)) {
                    Renderable& renderable = ecs->getData<Renderable>(entity);
                    if (ecs->hasComponent<Composite>(entity)) {
                        Composite& composite = ecs->getData<Composite>(entity);
                        renderable.pointA = dynamic.position;
                        renderable.pointB = dynamic.position + composite.size;
                    }
                }
            }
        }
    }

};
#endif // PHYSICS_SYSTEM_HPP