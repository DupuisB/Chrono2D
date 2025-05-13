#ifndef CONSTRAINT_SYSTEM_HPP
#define CONSTRAINT_SYSTEM_HPP

#include <memory>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"

class ConstraintSystem {
private:
    std::shared_ptr<ECS> ecs;

public:
    ConstraintSystem(std::shared_ptr<ECS> ecs) : ecs(ecs) {}

    void update() {
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (ecs->hasComponent<Composite>(entity) && ecs->hasComponent<Dynamic>(entity)) {
                Vec2f& pointA = ecs->getData<Composite>(entity).points[0];
                Vec2f& pointB = ecs->getData<Composite>(entity).points[1];
                Vec2f& pointC = ecs->getData<Composite>(entity).points[2];
                Vec2f& pointD = ecs->getData<Composite>(entity).points[3];
                float constraintAB = ecs->getData<Composite>(entity).size.x;
                float constraintAD = ecs->getData<Composite>(entity).size.y;
                float constraintAC = std::sqrt(constraintAB * constraintAB + constraintAD * constraintAD);
                float mass = ecs->getData<Body>(entity).mass;
                // for point A
                applyConstraint(pointA, pointB, constraintAB, mass, mass);
                applyConstraint(pointA, pointC, constraintAC, mass, mass);
                applyConstraint(pointA, pointD, constraintAD, mass, mass);
                // for point B
                applyConstraint(pointB, pointA, constraintAB, mass, mass);
                applyConstraint(pointB, pointC, constraintAD, mass, mass);
                applyConstraint(pointB, pointD, constraintAC, mass, mass);
                // for point C
                applyConstraint(pointC, pointA, constraintAC, mass, mass);
                applyConstraint(pointC, pointB, constraintAD, mass, mass);
                applyConstraint(pointC, pointD, constraintAB, mass, mass);
                // for point D
                applyConstraint(pointD, pointA, constraintAD, mass, mass);
                applyConstraint(pointD, pointB, constraintAC, mass, mass);
                applyConstraint(pointD, pointC, constraintAB, mass, mass);
            }
        }
    }

    void applyConstraint(Vec2f& pA, Vec2f& pB, float constraint, float massA, float massB) {
        Vec2f delta = pB - pA;
        float distance = delta.length();
        if (distance == 0) return;

        Vec2f n = delta/distance;

        float C = distance - constraint;
        float wSum = (1.0f / massA) + (1.0f / massB);

        if (wSum == 0) return;

        Vec2f correction = C * n;

        pA += - (massA/wSum) * correction;
        pB += (massB/wSum) * correction;
    }
};

#endif // CONSTRAINT_SYSTEM_HPP