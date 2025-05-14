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
            if (ecs->hasComponent<RigidRect>(entity) && ecs->hasComponent<RectConstraint>(entity)) {
                RigidRect& data = ecs->getData<RigidRect>(entity);
                if (data.isStatic) continue;
                RectConstraint& constraintData = ecs->getData<RectConstraint>(entity);
                Vec2f constraint = constraintData.size;
                Vec2f& pointA = ecs->getData<RigidRect>(entity).predictedPositions[0];
                Vec2f& pointB = ecs->getData<RigidRect>(entity).predictedPositions[1];
                Vec2f& pointC = ecs->getData<RigidRect>(entity).predictedPositions[2];
                Vec2f& pointD = ecs->getData<RigidRect>(entity).predictedPositions[3];
                float constraintAB = constraint.x;
                float constraintAD = constraint.y;
                float constraintAC = std::sqrt(constraintAB * constraintAB + constraintAD * constraintAD);
                float mass = ecs->getData<RigidRect>(entity).mass;
                // AB constraint
                applyConstraint(pointA, pointB, constraintAB);
                // AC constraint
                applyConstraint(pointA, pointC, constraintAC);
                // AD constraint
                applyConstraint(pointA, pointD, constraintAD);
                // BD constraint
                applyConstraint(pointB, pointD, constraintAC);
                // CD constraint
                applyConstraint(pointC, pointD, constraintAB);
                // BC constraint
                applyConstraint(pointB, pointC, constraintAD);
            }
        }
    }

    void applyConstraint(Vec2f& pA, Vec2f& pB, float constraint) {
        Vec2f delta = pB - pA;
        float distance = delta.length();
        if (distance == constraint) return;
        float val = (1.0f - constraint/distance);
        Vec2f correction = delta * val;

        pA += 0.5f * correction;
        pB += -0.5f * correction;
    }
};

#endif // CONSTRAINT_SYSTEM_HPP