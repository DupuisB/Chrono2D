// Define this before including cmath if needed (often required on Windows/MSVC)
// #define _USE_MATH_DEFINES
#include <cmath> // For std::cos, std::sin, std::hypot, M_PI

#include <SFML/Graphics.hpp>
#include <vector>
#include <numeric> // For std::copysign
#include <iostream>
#include <optional> // For SFML 3 event handling
#include <algorithm> // For std::min/max
#include <limits>    // For numeric_limits

// --- Configuration ---
constexpr unsigned int SCREEN_WIDTH = 1000;
constexpr unsigned int SCREEN_HEIGHT = 800;
constexpr float GROUND_HEIGHT = 50.0f;
constexpr int   POINT_COUNT = 10;

constexpr float OUTER_STIFFNESS = 50000.0f;
constexpr float INTERNAL_STIFFNESS = 50000.0f;
constexpr float SPRING_LENGTH_FACTOR = 2.0f;
constexpr float OUTWARD_FORCE_STRENGTH = 1000000.0f;

constexpr float POINT_MASS = 1.0f;
constexpr float GRAVITY_ACCELERATION = 981.0f;
// Damping applied once per fixed update, not per substep
constexpr float DAMPING_PER_STEP = 0.99f; // Damping factor applied once per fixed timestep

constexpr float MOVEMENT_ACCELERATION = 1500.0f;
constexpr float MAX_HORIZONTAL_VELOCITY = 400.0f;

constexpr int   PHYSICS_SUBSTEPS = 128; // Reduced substeps, less implicit damping
constexpr float COLLISION_OFFSET = 0.1f; // Small offset to prevent sticking

// Friction: Factor to *reduce* tangential velocity by (0=none, 1=full stop)
constexpr float GROUND_FRICTION = 0.2f;
constexpr float WALL_FRICTION = 0.2f;
// Bounce: Factor to retain normal velocity (0=none, 1=perfect bounce)
constexpr float BOUNCE_FACTOR = 0.6f;   // More moderate bounce

// --- Fixed Timestep for Physics ---
constexpr float PHYSICS_UPDATES_PER_SECOND = 120.0f;
constexpr float FIXED_DELTA_TIME = 1.0f / PHYSICS_UPDATES_PER_SECOND;

// --- Helper Functions ---
float vectorLength(const sf::Vector2f& v) {
    return std::hypot(v.x, v.y);
}

template <typename T>
constexpr const T& clampValue(const T& value, const T& low, const T& high) {
    // std::clamp is C++17, provide alternative
    return std::max(low, std::min(value, high));
}

// --- Helper Structures ---
struct Point {
    sf::Vector2f pos;
    sf::Vector2f velocity = {0.0f, 0.0f};
    sf::Vector2f acceleration = {0.0f, 0.0f};

    Point(sf::Vector2f p) : pos(p) {}

    // Update physics state for one SUB-step (without damping)
    void updateSubstep(float sub_dt) {
        velocity += acceleration * sub_dt;

        // Clamp horizontal velocity - applied per substep for responsiveness
        if (std::abs(velocity.x) > MAX_HORIZONTAL_VELOCITY) {
            velocity.x = std::copysign(MAX_HORIZONTAL_VELOCITY, velocity.x);
        }

        pos += velocity * sub_dt;
        acceleration = {0.0f, 0.0f}; // Reset acceleration for the next substep's forces
    }

    void applyDamping(float dampingFactor) {
         velocity *= dampingFactor;
    }

    void addForce(sf::Vector2f force) {
        if (POINT_MASS > 1e-9f) {
            acceleration += force / POINT_MASS;
        }
    }
};

struct Spring {
    int p1_idx;
    int p2_idx;
    float rest_length;
    float stiffness;
};

// --- Blob Class ---
class Blob {
public:
    std::vector<Point> points;
    std::vector<Spring> springs;
    sf::Vector2f center = {0.0f, 0.0f};
    bool outward_force_active = false;
    float movement_input_direction = 0.0f;

    Blob(sf::Vector2f start_pos) {
        for (int i = 0; i < POINT_COUNT; ++i) {
            float angle = (2.0f * static_cast<float>(M_PI) / POINT_COUNT) * i;
            float radius = static_cast<float>(POINT_COUNT) * SPRING_LENGTH_FACTOR;
            points.emplace_back(start_pos + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius));
        }
        springs.clear();
        int outer_spring_count = 0;
        int internal_spring_count = 0;

        // Create springs connecting all pairs of points
        for (int i = 0; i < POINT_COUNT; ++i) {
            for (int j = i + 1; j < POINT_COUNT; ++j) {
                sf::Vector2f delta = points[j].pos - points[i].pos;
                float dist = vectorLength(delta);

                // Determine if this is an outer ring spring
                // An outer spring connects i to i+1 OR the last point (N-1) to the first point (0)
                bool is_outer_ring_spring = (j == i + 1) || (i == 0 && j == POINT_COUNT - 1);

                float current_stiffness;
                if (is_outer_ring_spring) {
                    current_stiffness = OUTER_STIFFNESS;
                    outer_spring_count++;
                } else {
                    current_stiffness = INTERNAL_STIFFNESS;
                    internal_spring_count++;
                }

                springs.push_back({i, j, dist, current_stiffness}); // Use determined stiffness
            }
        }
        // Updated message to show the different counts
        std::cout << "Created " << outer_spring_count << " outer springs and "
                  << internal_spring_count << " internal springs (Total: "
                  << springs.size() << ")." << std::endl;
        calculateCenter();
    }

    void calculateCenter() {
        center = {0.0f, 0.0f};
        if (points.empty()) return;
        for (const auto& p : points) {
            center += p.pos;
        }
        center /= static_cast<float>(points.size());
    }

    void applyForces() {
        calculateCenter();
        for (auto& p : points) {
            p.addForce({0.0f, GRAVITY_ACCELERATION * POINT_MASS});
            p.addForce({movement_input_direction * MOVEMENT_ACCELERATION, 0.0f});
            if (outward_force_active) {
                sf::Vector2f dir_from_center = p.pos - center;
                float current_dist = vectorLength(dir_from_center);
                if (current_dist > 1e-6f) {
                    sf::Vector2f norm_dir = dir_from_center / current_dist;
                    p.addForce(norm_dir * OUTWARD_FORCE_STRENGTH);
                }
            }
        }
        for (const auto& s : springs) {
             // Ensure indices are valid before accessing points
             if (s.p1_idx >= points.size() || s.p2_idx >= points.size()) continue;

             Point& p1 = points[s.p1_idx];
             Point& p2 = points[s.p2_idx];
             sf::Vector2f delta = p2.pos - p1.pos;
             float dist = vectorLength(delta);
             if (dist > 1e-6f) {
                 float diff_ratio = (dist - s.rest_length) / dist;
                 sf::Vector2f force = delta * (s.stiffness * diff_ratio * 0.5f);
                 p1.addForce(force);
                 p2.addForce(-force);
             }
        }
    }

    void setMovementDirection(float direction) {
        movement_input_direction = clampValue(direction, -1.0f, 1.0f);
    }

    // --- Refined Collision Handling ---
    void solveConstraints(const sf::RectangleShape& ground, const std::vector<sf::RectangleShape>& walls) {
        sf::FloatRect groundBounds = ground.getGlobalBounds();
        float ground_y = groundBounds.position.y;

        for (auto& p : points) {
            // --- Ground Collision ---
            if (p.pos.y >= ground_y) {
                p.pos.y = ground_y - COLLISION_OFFSET;

                // Decompose velocity
                float normalVelocity = p.velocity.y;
                float tangentVelocity = p.velocity.x;

                // Apply bounce (restitution) to normal velocity only if moving into ground
                if (normalVelocity > 0) {
                   p.velocity.y = -normalVelocity * BOUNCE_FACTOR;
                } else {
                   p.velocity.y = std::min(0.0f, p.velocity.y);
                }

                p.velocity.x = tangentVelocity * (1.0f - GROUND_FRICTION);
            }

            // --- Wall Collision ---
            for (const auto& wall : walls) {
                sf::FloatRect wallBounds = wall.getGlobalBounds();
                float wallLeft = wallBounds.position.x;
                float wallRight = wallBounds.position.x + wallBounds.size.x;
                float wallTop = wallBounds.position.y;
                float wallBottom = wallBounds.position.y + wallBounds.size.y;

                // Check if point is within the wall's AABB
                if (p.pos.x >= wallLeft && p.pos.x <= wallRight &&
                    p.pos.y >= wallTop && p.pos.y <= wallBottom)
                {
                    // Calculate penetration depths from each side
                    float overlapLeft = p.pos.x - wallLeft;
                    float overlapRight = wallRight - p.pos.x;
                    float overlapTop = p.pos.y - wallTop;
                    float overlapBottom = wallBottom - p.pos.y;

                    // Find the minimum penetration depth to determine collision side
                    float minOverlap = std::numeric_limits<float>::max();
                    sf::Vector2f collisionNormal = {0, 0};

                    if (overlapLeft < minOverlap) { minOverlap = overlapLeft; collisionNormal = {-1, 0}; } // Hit from right moving left
                    if (overlapRight < minOverlap) { minOverlap = overlapRight; collisionNormal = {1, 0}; } // Hit from left moving right
                    if (overlapTop < minOverlap) { minOverlap = overlapTop; collisionNormal = {0, -1}; } // Hit from bottom moving up
                    if (overlapBottom < minOverlap) { minOverlap = overlapBottom; collisionNormal = {0, 1}; } // Hit from top moving down

                    // Correct position by pushing out along the normal
                    p.pos += collisionNormal * (minOverlap + COLLISION_OFFSET);

                    // Calculate velocity component along the normal
                    float normalVelocityMag = p.velocity.x * collisionNormal.x + p.velocity.y * collisionNormal.y;

                    // Apply bounce only if moving towards the wall surface
                    if (normalVelocityMag > 0) {
                        // Calculate impulse for restitution (bounce)
                        sf::Vector2f bounceImpulse = collisionNormal * (-normalVelocityMag * (1.0f + BOUNCE_FACTOR));
                        p.velocity += bounceImpulse; // Apply bounce impulse directly
                    } else {
                         // If already moving away or resting along normal, just ensure it stays out
                         // Project velocity onto normal, if positive (moving away), zero it out?
                         // Or simply ensure the normal component isn't pushing into wall
                         float velAlongNormal = p.velocity.x * collisionNormal.x + p.velocity.y * collisionNormal.y;
                         if(velAlongNormal > 0) {
                             p.velocity -= collisionNormal * velAlongNormal;
                         }
                    }

                    // Apply friction (dampen velocity tangential to collision normal)
                    sf::Vector2f tangentDirection = {-collisionNormal.y, collisionNormal.x}; // Perpendicular to normal
                    float tangentVelocityMag = p.velocity.x * tangentDirection.x + p.velocity.y * tangentDirection.y;
                    sf::Vector2f tangentVelocity = tangentDirection * tangentVelocityMag;
                    sf::Vector2f velocityNormalComponent = collisionNormal * (p.velocity.x * collisionNormal.x + p.velocity.y * collisionNormal.y);

                    // Reduce tangential velocity
                    tangentVelocity *= (1.0f - WALL_FRICTION);

                    // Reconstruct velocity
                    p.velocity = velocityNormalComponent + tangentVelocity;
                }
            }
        }
    }

    // Physics update for one fixed timestep
    void updatePhysics(float dt, const sf::RectangleShape& ground, const std::vector<sf::RectangleShape>& walls) {
        if (PHYSICS_SUBSTEPS <= 0) return;
        float sub_dt = dt / static_cast<float>(PHYSICS_SUBSTEPS);

        for (int i = 0; i < PHYSICS_SUBSTEPS; ++i) {
            applyForces(); // Accumulate forces for this substep
            for (auto& p : points) {
                p.updateSubstep(sub_dt); // Update velocity & position
                solveConstraints(ground, walls);
            }
            // Solve constraints multiple times per substep can improve stability,
            // but let's do it once after position update for now.
        }

        // Apply damping ONCE per fixed timestep, after all substeps
        for(auto& p : points) {
            p.applyDamping(DAMPING_PER_STEP);
        }

        // Update visual center after physics step
        calculateCenter();
    }

    // SFML 3 Corrected Rendering (No changes needed from previous correction)
    void render(sf::RenderWindow& window) {
        sf::VertexArray lines(sf::PrimitiveType::Lines);
        for (const auto& s : springs) {
             if (s.p1_idx < points.size() && s.p2_idx < points.size()) {
                sf::Vertex v1;
                v1.position = points[s.p1_idx].pos;
                v1.color = sf::Color::Green;
                lines.append(v1);
                sf::Vertex v2;
                v2.position = points[s.p2_idx].pos;
                v2.color = sf::Color::Green;
                lines.append(v2);
             }
        }
        window.draw(lines);

        sf::CircleShape pointShape(3.0f);
        pointShape.setOrigin({3.0f, 3.0f});
        pointShape.setFillColor(sf::Color::Yellow);
        for (const auto& p : points) {
            pointShape.setPosition(p.pos);
            window.draw(pointShape);
        }

        sf::CircleShape centerShape(4.0f);
        centerShape.setOrigin({4.0f, 4.0f});
        centerShape.setFillColor(sf::Color::Red);
        centerShape.setPosition(center);
        window.draw(centerShape);
    }

    void toggleOutwardForce() {
        outward_force_active = !outward_force_active;
    }
};

// --- Main Function ---
int main() {
    // SFML Window Setup (using Vector2i for position if needed)
    sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "SFML 3 Soft Body - Explicit Velocity");
    window.setVerticalSyncEnabled(true);
    // Optional: Ensure window appears consistently
    window.setPosition({0, 0}); // Requires sf::Vector2i

    // Ground Setup
    sf::RectangleShape ground(sf::Vector2f(static_cast<float>(SCREEN_WIDTH), GROUND_HEIGHT));
    ground.setFillColor(sf::Color(80, 80, 80));
    ground.setPosition({0.f, static_cast<float>(SCREEN_HEIGHT) - GROUND_HEIGHT});

    // Wall Setup
    std::vector<sf::RectangleShape> walls;
    sf::RectangleShape leftWall(sf::Vector2f(50.f, static_cast<float>(SCREEN_HEIGHT)));
    leftWall.setFillColor(sf::Color(100, 50, 50));
    leftWall.setPosition({50.f, 0.f});
    walls.push_back(leftWall);
    // Example: Add a right wall
    sf::RectangleShape rightWall(sf::Vector2f(50.f, static_cast<float>(SCREEN_HEIGHT)));
    rightWall.setFillColor(sf::Color(100, 50, 50));
    rightWall.setPosition({SCREEN_WIDTH - 70.f, 0.f}); // Position near right edge
    walls.push_back(rightWall);


    Blob blob({static_cast<float>(SCREEN_WIDTH) / 2.0f, static_cast<float>(SCREEN_HEIGHT) / 3.0f});

    sf::Clock deltaClock;
    float accumulator = 0.0f;

    // --- Game Loop ---
    while (window.isOpen()) {
        float dt = deltaClock.restart().asSeconds();
        // Basic frame time clamping
        if (dt > 0.1f) {
            dt = 0.1f;
        }
        accumulator += dt;

        // --- SFML 3 Event Handling ---
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Space) {
                    blob.toggleOutwardForce();
                }
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                     window.close();
                 }
            }
        }

        // --- Continuous Input ---
        float moveDir = 0.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            moveDir -= 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            moveDir += 1.0f;
        }
        blob.setMovementDirection(moveDir);

        // --- Fixed Physics Update ---
        while (accumulator >= FIXED_DELTA_TIME) {
            blob.updatePhysics(FIXED_DELTA_TIME, ground, walls);
            accumulator -= FIXED_DELTA_TIME;
        }

        // --- Rendering ---
        window.clear(sf::Color(30, 30, 50));
        window.draw(ground);
        for(const auto& wall : walls) {
            window.draw(wall);
         }
        blob.render(window);
        window.display();
    }

    return 0;
}