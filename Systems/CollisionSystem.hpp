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
            for (Entity entityB = 0; entityB < MAX_ENTITIES; entityB++) {
                if (ecs->hasComponent<Mass>(entityA) && ecs->hasComponent<Mass>(entityB) && ecs->hasComponent<Position>(entityA) && ecs->hasComponent<Position>(entityB) && entityA != entityB) {
                    Mass& massA = ecs->getData<Mass>(entityA);
                    Mass& massB = ecs->getData<Mass>(entityB);

                    PredictedPosition& positionA = ecs->getData<PredictedPosition>(entityA);
                    PredictedPosition& positionB = ecs->getData<PredictedPosition>(entityB);
                    std::vector<Vec2f>& polygonA = positionA.predictedPositions;
                    std::vector<Vec2f>& polygonB = positionB.predictedPositions;
                    Vec2f centerA = ecs->getData<Position>(entityA).center;
                    Vec2f centerB = ecs->getData<Position>(entityB).center;
                    // Check for each edge of polygonA against each edge of polygonB
                    for(int i = 0; i < polygonA.size(); i++) {
                        Vec2f& point = polygonA[i];
                        SATResult result = satPointPolygon(point, polygonB, centerA, centerB);
                        if (!result.collision) continue;

                        float kA = massA.m / (massA.m + massB.m);
                        float kB = massB.m / (massA.m + massB.m);
                        Vec2f mtv = result.mtv;

                        point -= kA * mtv;
                        for (auto& pb : polygonB) pb += (kB / polygonB.size()) * mtv;
                    }
                }
            }
        }
    }

    SATResult satPointPolygon(const Vec2f& point, const std::vector<Vec2f>& polygon, const Vec2f centerA, const Vec2f centerB) {
        SATResult result;
        float minOverlap = 100000000000.0f;
        Vec2f smallestAxis;

        for (int i=0; i < polygon.size(); i++) {
            Vec2f a = polygon[i];
            Vec2f b = polygon[(i + 1) % polygon.size()];
            Vec2f axis = normal2Segment(a, b);
            std::array<Vec2f, 2> edge = {a, b};

            float projPoint = point.dot(axis);
            auto [minPoly, maxPoly] = projectPolygonOntoAxis(polygon, axis);
            if (projectionDoNotOverlap(projPoint, projPoint, minPoly, maxPoly)) return result;

            float overlap = std::min(std::abs(projPoint - minPoly), std::abs(projPoint - maxPoly));
            if (overlap < minOverlap) {
                minOverlap = overlap;
                smallestAxis = axis;
            }
        }

        result.collision = true;
        result.axis = smallestAxis;
        result.overlap = minOverlap;
        Vec2f direction = centerB - centerA;
        if (direction.dot(smallestAxis) < 0) {
            smallestAxis = -1.0f * smallestAxis;
        }
        result.mtv = smallestAxis * minOverlap;
        return result;
    }

    std::array<float, 2> projectPolygonOntoAxis(const std::vector<Vec2f> polygon, const Vec2f axis) {
        float min = polygon[0].dot(axis);
        float max = min;
        for (int i = 1; i < polygon.size(); ++i) {
            float proj = polygon[i].dot(axis);
            if (proj < min) min = proj;
            if (proj > max) max = proj;
        }
        return {min, max};
    }

    std::array<float, 2> projectSideOnAxis(const std::array<Vec2f, 2> side, const Vec2f axis) {
        float min = side[0].dot(axis);
        float max = min;
        for (int i = 1; i < 2; ++i) {
            float proj = side[i].dot(axis);
            if (proj < min) min = proj;
            if (proj > max) max = proj;
        }
        return {min, max};
    }

    SATResult satCollision(const std::array<Vec2f, 3> triangleA, const std::array<Vec2f, 3> triangleB, const Vec2f centerA, const Vec2f centerB) {
        SATResult result;
        float minOverlap = 100000000000.0f;
        Vec2f smallestAxis;

        float overlapA, overlapB;
        Vec2f axisA, axisB;

        for (int i=0; i < 3; i++) {
            Vec2f pt0 = triangleA[(i + 1) % 3];
            Vec2f pt1 = triangleA[i];
            Vec2f axis = normal2Segment(pt0, pt1);
            if (!checkAxes(axis, triangleA, triangleB, minOverlap, smallestAxis)) return result;
        }

        for (int i=0; i < 3; i++) {
            Vec2f pt0 = triangleB[(i + 1) % 3];
            Vec2f pt1 = triangleB[i];
            Vec2f axis = normal2Segment(pt0, pt1);
            if (!checkAxes(axis, triangleA, triangleB, minOverlap, smallestAxis)) return result;
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

    bool checkAxes(const Vec2f axis, const std::array<Vec2f, 3> triA, const std::array<Vec2f, 3> triB, float& minOverlap, Vec2f& smallestAxis) {
        auto [minA, maxA] = projectTriangleOntoAxis(triA, axis);
        auto [minB, maxB] = projectTriangleOntoAxis(triB, axis);
        if (projectionDoNotOverlap(minA, maxA, minB, maxB)) return false;
        float overlap = std::min(maxA - minB, maxB - minA);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
        return true;
    }

    std::array<float, 2> projectTriangleOntoAxis(const std::array<Vec2f, 3> tri, const Vec2f axis) {
        float min = tri[0].dot(axis);
        float max = min;
        for (int i = 1; i < 3; ++i) {
            float proj = tri[i].dot(axis);
            if (proj < min) min = proj;
            if (proj > max) max = proj;
        }
        return {min, max};
    }

    bool projectionDoNotOverlap(float minA, float maxA, float minB, float maxB) {
        return (minA > maxB || minB > maxA);
    }
};
#endif // COLLISION_SYSTEM_HPP