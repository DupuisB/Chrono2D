#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include <memory>
#include <vector>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"
#include "../Components.hpp"

struct SATResult {
    bool collision = false;
    Vec2f mtv;
    float overlap = 0.0f;
    Vec2f axis;
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
            for (Entity entityB = 0; entityB < MAX_ENTITIES; entityB++) {
                if (ecs->hasComponent<Mass>(entityA) && ecs->hasComponent<Mass>(entityB) && ecs->hasComponent<Position>(entityA) && ecs->hasComponent<Position>(entityB) && entityA != entityB) {
                    Mass& massA = ecs->getData<Mass>(entityA);
                    Mass& massB = ecs->getData<Mass>(entityB);
                    if (massA.m == 0.0f && massB.m == 0.0f) continue;
                    PredictedPosition& positionA = ecs->getData<PredictedPosition>(entityA);
                    PredictedPosition& positionB = ecs->getData<PredictedPosition>(entityB);
                    std::vector<Vec2f>& polygonA = positionA.predictedPositions;
                    std::vector<Vec2f>& polygonB = positionB.predictedPositions;
                    // Check for each edge of polygonA against each edge of polygonB
                    for(int i = 0; i < polygonA.size(); i++) {
                        for (int j = 0; j < polygonB.size(); j++) {
                            std::vector<Vec2f> sideA = { polygonA[i], polygonA[(i + 1) % polygonA.size()] };
                            std::vector<Vec2f> sideB = { polygonB[j], polygonB[(j + 1) % polygonB.size()] };
                            SATResult result = satCollision(sideA, sideB);
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
                }
            }
        }
    }

    SATResult satCollision(const std::vector<Vec2f>& polygonA, const std::vector<Vec2f>& polygonB) {
        SATResult result;
        float minOverlap = 100000000000.0f;
        Vec2f smallestAxis;

        if (!checkAxes(polygonA ,polygonA, polygonB, minOverlap, smallestAxis)) return result;
        if (!checkAxes(polygonB ,polygonB, polygonA, minOverlap, smallestAxis)) return result;
            
        Vec2f direction = computePolygonCenter(polygonB) - computePolygonCenter(polygonA);
        if (direction.dot(smallestAxis) < 0) {
            smallestAxis = -1.0f * smallestAxis;
        }

        result.collision = true;
        result.axis = smallestAxis;
        result.overlap = minOverlap;
        result.mtv = smallestAxis * minOverlap;
        return result;
    }

    bool checkAxes(const std::vector<Vec2f>& polygonWithAxes, const std::vector<Vec2f>& polygonA, const std::vector<Vec2f>& polygonB, float& minOverlap, Vec2f& smallestAxis) {
        std::vector<Vec2f> axes = getPolygonSideNormals(polygonWithAxes);
        for (const Vec2f& axis : axes) {
            auto [minA, maxA] = projectPolygonOntoAxis(polygonA, axis);
            auto [minB, maxB] = projectPolygonOntoAxis(polygonB, axis);
            if (projectionDoNotOverlap(minA, maxA, minB, maxB)) return false;
            float overlap = std::min(maxA - minB, maxB - minA);
            if (overlap < minOverlap) {
                minOverlap = overlap;
                smallestAxis = axis;
            }
        }
        return true;
    }

    std::vector<Vec2f> getPolygonSideNormals(const std::vector<Vec2f>& polygon) {
        std::vector<Vec2f> axes;
        for(int i = 0; i < polygon.size(); i++) {
            Vec2f A = polygon[i];
            Vec2f B = polygon[(i + 1) % polygon.size()]; // Wrap around
            axes.push_back(normal2Segment(A, B));
        }
        return axes;
    }

    std::array<float, 2> projectPolygonOntoAxis(const std::vector<Vec2f>& polygon, const Vec2f& axis) {
        float min, max;
        float projection = polygon[0].dot(axis);
        min = max = projection;
        for (int i = 1; i < polygon.size(); i++) {
            projection = polygon[i].dot(axis);
            if (projection < min) min = projection;
            if (projection > max) max = projection;
        }
        return {min, max};
    }

    bool projectionDoNotOverlap(float minA, float maxA, float minB, float maxB) {
        return (minA >= maxB || minB >= maxA);
    }

    Vec2f computePolygonCenter(const std::vector<Vec2f>& polygon) {
        Vec2f center(0, 0);
        for (const Vec2f& point : polygon) {
            center += point;
        }
        return center / static_cast<float>(polygon.size());
    }
};
#endif // COLLISION_SYSTEM_HPP