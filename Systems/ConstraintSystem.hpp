#ifndef CONSTRAINT_SYSTEM_HPP
#define CONSTRAINT_SYSTEM_HPP

#include <memory>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"
#include "../Components.hpp"

class ConstraintSystem {
private:
    std::shared_ptr<ECS> ecs;

public:
    ConstraintSystem(std::shared_ptr<ECS> ecs) : ecs(ecs) {}

    void update() {
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (ecs->hasComponent<PolygonConstraint>(entity) && ecs->hasComponent<PredictedPosition>(entity)) {
                PredictedPosition& predictedPosition = ecs->getData<PredictedPosition>(entity);
                std::vector<Vec2f>& predictedPositions = predictedPosition.predictedPositions;
                PolygonConstraint& constraintData = ecs->getData<PolygonConstraint>(entity);
                std::vector<float>& constraints = constraintData.lengthConstraints;
                std::vector<std::array<int, 2>>& edges = constraintData.edges;
                for (int i = 0; i < edges.size(); i++) {
                    Vec2f& pointA = predictedPositions[edges[i][0]];
                    Vec2f& pointB = predictedPositions[edges[i][1]];
                    float constraint = constraints[i];
                    applyConstraint(pointA, pointB, constraint);
                }
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