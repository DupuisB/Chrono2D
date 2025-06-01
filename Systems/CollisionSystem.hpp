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
    int i, j; // indices of the contact edge
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

                    if (massA.m == 0.0f && massB.m == 0.0f) continue; // both are static, no collision

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

                        Vec2f contactEdgeA = polygonB[result.i];
                        Vec2f contactEdgeB = polygonB[result.j];

                        float kA = massA.m / (massA.m + massB.m);
                        float kB = massB.m / (massA.m + massB.m);
                        Vec2f mtv = result.mtv;

                        point -= kA * mtv;
                        // find the point q that is the projection of point onto the contact edge
                        Vec2f q = contactEdgeA + ((point - contactEdgeA).dot(contactEdgeB - contactEdgeA) / (contactEdgeB - contactEdgeA).lengthSquared()) * (contactEdgeB - contactEdgeA);
                        float qA = (q - contactEdgeA).length() / (contactEdgeB - contactEdgeA).length();
                        float qB = (q - contactEdgeB).length() / (contactEdgeB - contactEdgeA).length();
                        polygonB[result.i] += kB * qB * mtv;
                        polygonB[result.j] += kB * qA * mtv;
                        // recheck each edge of polygonB against each edge of polygonA
                        // if there is a collision, resolve it push both polygons apart

                    }
                }
            }
        }
    }

    void checkPolygonEdgeCollision(std::vector<Vec2f>& polygonA, std::vector<Vec2f>& polygonB) {
        for (int i = 0; i < polygonA.size(); i++) {
            Vec2f aStart = polygonA[i];
            Vec2f aEnd = polygonA[(i + 1) % polygonA.size()];
            for (int j = 0; j < polygonB.size(); j++) {
                Vec2f bStart = polygonB[j];
                Vec2f bEnd = polygonB[(j + 1) % polygonB.size()];
                if (checkEdgeOnEdgeCollision(aStart, aEnd, bStart, bEnd)) {
                    // Handle collision response here
                }
            }
        }
    }

    SATResult polygonToPolygon(const std::vector<Vec2f>& polygonA, const )

    SATResult satPointPolygon(const Vec2f& point, const std::vector<Vec2f>& polygon, const Vec2f centerA, const Vec2f centerB) {
        SATResult result;
        float minOverlap = 100000000000.0f;
        Vec2f smallestAxis = Vec2f(0, 0);

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
                result.i = i;
                result.j = (i + 1) % polygon.size();
            }
        }

        result.collision = true;
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

    bool projectionDoNotOverlap(float minA, float maxA, float minB, float maxB) {
        return (minA > maxB || minB > maxA);
    }

    bool checkEdgeOnEdgeCollision(const Vec2f& edgeAStart, const Vec2f& edgeAEnd, const Vec2f& edgeBStart, const Vec2f& edgeBEnd) {
        Vec2f dA = edgeAEnd - edgeAStart;
        Vec2f dB = edgeBEnd - edgeBStart;
        float cross = dA.cross(dB);
        if (cross == 0) return false; // edges are parallel

        Vec2f diff = edgeBStart - edgeAStart;
        float t = diff.cross(dB) / cross;
        float u = diff.cross(dA) / cross;

        return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
    }
};
#endif // COLLISION_SYSTEM_HPP