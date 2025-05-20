#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include <memory>
#include <vector>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"
#include "../Components.hpp"

struct SATResult {
    bool collision = false;
    Vec2f mtv = Vec2f(0, 0);
    float overlap = 0.0f;
    Vec2f axis = Vec2f(0, 0);
};


class CollisionSystem {
private:
    std::shared_ptr<ECS> ecs;
    const float DAMPING_STATIC = 1.0f;
    const float DAMPING_DYNAMIC = 0.8f;

public:
    CollisionSystem(std::shared_ptr<ECS> ecs) : ecs(ecs) {}

    void detectCollisions() {
        for (Entity entityA = 0; entityA < MAX_ENTITIES; entityA++) {
            for (Entity entityB = entityA + 1; entityB < MAX_ENTITIES; entityB++) {
                if (ecs->hasComponent<Mass>(entityA) && ecs->hasComponent<Mass>(entityB) && ecs->hasComponent<Position>(entityA) && ecs->hasComponent<Position>(entityB) && entityA != entityB) {
                    Mass& massA = ecs->getData<Mass>(entityA);
                    Mass& massB = ecs->getData<Mass>(entityB);
                    if (massA.m == 0.0f && massB.m == 0.0f) continue;
                    PredictedPosition& positionA = ecs->getData<PredictedPosition>(entityA);
                    PredictedPosition& positionB = ecs->getData<PredictedPosition>(entityB);
                    std::vector<Vec2f> polygonA = positionA.predictedPositions;
                    std::vector<Vec2f> polygonB = positionB.predictedPositions;
                    Vec2f centerA = ecs->getData<Position>(entityA).center;
                    Vec2f centerB = ecs->getData<Position>(entityB).center;
                    // Check for each edge of polygonA against each edge of polygonB
                    for(int i = 0; i < polygonA.size(); i++) {
                        for (int j = 0; j < polygonB.size(); j++) {
                            std::array<Vec2f, 2> sideA = {polygonA[i], polygonA[(i+1) % polygonA.size()]};
                            std::array<Vec2f, 2> sideB = {polygonB[j], polygonB[(j+1) % polygonB.size()]};
                            SATResult result = satCollision(sideA, sideB, centerA, centerB);
                            if(!result.collision) continue;
                            // move the current sides that are colliding
                            float kA = massA.m / (massA.m + massB.m);
                            float kB = massB.m / (massA.m + massB.m);
                            Vec2f mtv = result.mtv;

                            polygonA[i] -= kA * mtv;
                            polygonA[(i + 1) % polygonA.size()] -= kA * mtv;
                            polygonB[j] += kB * mtv;
                            polygonB[(j + 1) % polygonB.size()] += kB * mtv;
                        }
                    }
                    // update the predicted positions of the polygons
                    positionA.predictedPositions = polygonA;
                    positionB.predictedPositions = polygonB;
                }
            }
        }
    }

    SATResult satCollision(const std::array<Vec2f, 2> sideA, const std::array<Vec2f, 2> sideB, const Vec2f centerA, const Vec2f centerB) {
        SATResult result;
        float minOverlap = 100000000000.0f;
        Vec2f smallestAxis;

        float overlapA, overlapB;
        Vec2f axisA, axisB;

        if (!checkAxes(sideA ,sideA, sideB, overlapA, axisA)) return result;
        if (!checkAxes(sideB ,sideB, sideA, overlapB, axisB)) return result;

        if (overlapA < overlapB) {
            minOverlap = overlapA;
            smallestAxis = axisA;
        } else {
            minOverlap = overlapB;
            smallestAxis = axisB;
        }

        Vec2f direction = centerB - centerA;
        if (direction.dot(smallestAxis) < 0) {
            smallestAxis = -1.0f * smallestAxis;
        }

        result.collision = true;
        result.axis = smallestAxis;
        result.overlap = minOverlap;
        result.mtv = smallestAxis * minOverlap;
        return result;
    }

    bool checkAxes(const std::array<Vec2f, 2> sideWithAxis, const std::array<Vec2f, 2> sideA, const std::array<Vec2f, 2> sideB, float& overlap, Vec2f& axis) {
        Vec2f A = sideWithAxis[0];
        Vec2f B = sideWithAxis[1];
        axis = normal2Segment(A, B);
        auto [minA, maxA] = projectSideOntoAxis(sideA, axis);
        auto [minB, maxB] = projectSideOntoAxis(sideB, axis);
        if (projectionDoNotOverlap(minA, maxA, minB, maxB)) return false;
        overlap = std::min(maxA - minB, maxB - minA);
        return true;
    }

    std::array<float, 2> projectSideOntoAxis(const std::array<Vec2f, 2> side, const Vec2f axis) {
        float min, max;
        float projection = side[0].dot(axis);
        min = max = projection;
        float projection2 = side[1].dot(axis);
        if (projection2 < min) min = projection2;
        if (projection2 > max) max = projection2;
        return {min, max};
    }

    bool projectionDoNotOverlap(float minA, float maxA, float minB, float maxB) {
        return (minA >= maxB || minB >= maxA);
    }
};
#endif // COLLISION_SYSTEM_HPP