#ifndef PRIMITIVES_ROPE_HPP
#define PRIMITIVES_ROPE_HPP

#include "../game_object.hpp" // Access to GameObject, Box2D, SFML, createAnchorBody
#include <vector>
#include <cmath> // For b2Distance
#include <SFML/Graphics.hpp> // For sf::Color
#include <iostream> // For std::cerr

/**
 * @brief Creates a segmented rope connecting two bodies.
 *
 * @param worldId The Box2D world ID.
 * @param gameObjects Reference to the vector storing all game objects (rope segments will be added here).
 * @param bodyA The first body to attach the rope to.
 * @param localAnchorA Local attachment point on bodyA.
 * @param bodyB The second body to attach the rope to.
 * @param localAnchorB Local attachment point on bodyB.
 * @param numSegments Number of segments in the rope (must be >= 1).
 * @param segmentPrimaryDim Length of each segment along the rope's primary axis.
 * @param segmentSecondaryDim Thickness of each segment.
 * @param isVerticalOrientation True if segments are primarily vertical (primaryDim=height, secondaryDim=width),
 *                              False if segments are primarily horizontal (primaryDim=width, secondaryDim=height).
 * @param color Color of the rope segments.
 * @param segmentLinearDamping Linear damping for segments (default: 0.2f).
 * @param segmentDensity Density for segments (default: 0.05f).
 * @param segmentFriction Friction for segments (default: 0.5f).
 * @param segmentRestitution Restitution for segments (default: 0.1f).
 * @return True if the rope was created successfully, false otherwise.
 */
inline bool createSegmentedRope(
    b2WorldId worldId,
    std::vector<GameObject>& gameObjects,
    b2BodyId bodyA, b2Vec2 localAnchorA,
    b2BodyId bodyB, b2Vec2 localAnchorB,
    int numSegments,
    float segmentPrimaryDim, // This will be calculated based on total length and numSegments
    float segmentSecondaryDim, // This is the thickness
    bool isVerticalOrientation,
    sf::Color color,
    float segmentLinearDamping = 0.2f,
    float segmentDensity = 0.05f,
    float segmentFriction = 0.5f,
    float segmentRestitution = 0.1f) {

    if (B2_IS_NULL(bodyA) || B2_IS_NULL(bodyB) || numSegments < 1) {
        std::cerr << "Error: Invalid parameters for createSegmentedRope." << std::endl;
        return false;
    }

    b2Vec2 worldPosA = b2Body_GetWorldPoint(bodyA, localAnchorA);
    b2Vec2 worldPosB = b2Body_GetWorldPoint(bodyB, localAnchorB);

    float totalIdealLength = b2Distance(worldPosA, worldPosB);
    if (totalIdealLength < 0.001f) { // Effectively zero length
        // Connect directly if zero length and one segment requested (or handle as error)
        if (numSegments == 1) {
             // Potentially create a single joint, but this function is for *segmented* ropes.
             // For now, let's consider this an edge case that might not be desired.
            std::cerr << "Warning: Rope total length is near zero." << std::endl;
        } else {
            // Multiple segments for a zero length rope is problematic.
            return false;
        }
    }
    
    float actualSegmentLength = totalIdealLength / numSegments;
    if (actualSegmentLength < 0.001f) actualSegmentLength = 0.001f; // Prevent zero dimension segments

    b2BodyId prevBodyId = bodyA;
    b2Vec2 prevBodyLocalConnectAnchor = localAnchorA;

    for (int i = 0; i < numSegments; ++i) {
        GameObject segmentObj;
        float segWidth, segHeight;
        b2Vec2 currentSegmentLocalConnectAnchorToPrev; 
        b2Vec2 currentSegmentLocalConnectAnchorToNext;

        if (isVerticalOrientation) {
            segWidth = segmentSecondaryDim;  // thickness
            segHeight = actualSegmentLength; // length
            currentSegmentLocalConnectAnchorToPrev = {0.0f, segHeight / 2.0f};    // Top-middle
            currentSegmentLocalConnectAnchorToNext = {0.0f, -segHeight / 2.0f}; // Bottom-middle
        } else { // Horizontal orientation
            segWidth = actualSegmentLength; // length
            segHeight = segmentSecondaryDim; // thickness
            currentSegmentLocalConnectAnchorToPrev = {-segWidth / 2.0f, 0.0f};  // Middle-left
            currentSegmentLocalConnectAnchorToNext = {segWidth / 2.0f, 0.0f}; // Middle-right
        }

        // Initial position for segment (interpolated along the straight line between worldPosA and worldPosB)
        float t = (i + 0.5f) / numSegments;
        b2Vec2 segmentCenterPos = {
            worldPosA.x + t * (worldPosB.x - worldPosA.x),
            worldPosA.y + t * (worldPosB.y - worldPosA.y)
        };
        
        // Initial orientation: Box2D bodies are created with angle 0.
        // The revolute joints will allow them to hang/drape naturally.
        // For very stiff ropes or specific initial configurations, one might set bodyDef.angle.
        // Here, we assume segments are created axis-aligned and physics sorts it out.

        if (segmentObj.init(worldId, segmentCenterPos.x, segmentCenterPos.y,
                            segWidth, segHeight,
                            true, color, false, // isDynamic, no fixed rotation
                            segmentLinearDamping, segmentDensity, segmentFriction, segmentRestitution)) {
            gameObjects.push_back(segmentObj);
            b2BodyId currentSegmentBodyId = segmentObj.bodyId;

            b2RevoluteJointDef revoluteDef = b2DefaultRevoluteJointDef();
            revoluteDef.bodyIdA = prevBodyId;
            revoluteDef.bodyIdB = currentSegmentBodyId;
            revoluteDef.localAnchorA = prevBodyLocalConnectAnchor;
            revoluteDef.localAnchorB = currentSegmentLocalConnectAnchorToPrev;
            revoluteDef.collideConnected = false; // Segments of the same rope should not collide
            b2CreateRevoluteJoint(worldId, &revoluteDef);

            prevBodyId = currentSegmentBodyId;
            prevBodyLocalConnectAnchor = currentSegmentLocalConnectAnchorToNext;
        } else {
            std::cerr << "Failed to create rope segment " << i << std::endl;
            // Consider cleanup of already created segments if this happens mid-rope.
            // For simplicity, we'll just return false.
            return false; 
        }
    }

    // Connect the last rope segment to bodyB
    b2RevoluteJointDef revoluteDef = b2DefaultRevoluteJointDef();
    revoluteDef.bodyIdA = prevBodyId; // Last segment created
    revoluteDef.bodyIdB = bodyB;
    revoluteDef.localAnchorA = prevBodyLocalConnectAnchor; // Connection point on the last segment
    revoluteDef.localAnchorB = localAnchorB;               // User-defined local anchor on bodyB
    revoluteDef.collideConnected = false;
    b2CreateRevoluteJoint(worldId, &revoluteDef);

    return true;
}

#endif // PRIMITIVES_ROPE_HPP
