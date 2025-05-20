#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include <memory>

#include "../ECS/ECS.hpp"
#include "../utils/math.hpp"
#include "../Components.hpp"


class RenderSystem {
private:
    std::shared_ptr<ECS> ecs;
    sf::RenderWindow* window;

public:
    RenderSystem(std::shared_ptr<ECS> ecs, sf::RenderWindow* window) : ecs(ecs), window(window) {}

    void render() {
        window->clear(sf::Color::Black);
        for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
            if (ecs->hasComponent<RenderablePolygon>(entity)) {
                renderRigidBodies(entity);
            }
            if (ecs->hasComponent<Velocity>(entity)) {
                renderVelocity(entity);
            }
            if (ecs->hasComponent<Position>(entity)) {
                renderCenter(entity);
            }
        }
        window->display();
    }

    void renderCenter(Entity entity) {
        Position& position = ecs->getData<Position>(entity);
        sf::CircleShape circle(5.0f);
        sf::Vector2f center(position.center.x - 5.0f, position.center.y - 5.0f);
        circle.setFillColor(sf::Color::Red);
        circle.setPosition(center);
        window->draw(circle);
    }

    void renderRigidBodies(Entity entity) {
        RenderablePolygon& renderData = ecs->getData<RenderablePolygon>(entity);
        Position& position = ecs->getData<Position>(entity);

        std::vector<Vec2f> positions = position.positions;

        sf::VertexArray lines(sf::PrimitiveType::LineStrip, positions.size() + 1);
        for (size_t i = 0; i < positions.size(); i++) {
            lines[i].position = sf::Vector2f(positions[i].x, positions[i].y);
            lines[i].color = renderData.color;
        }
        lines[positions.size()].position = sf::Vector2f(positions[0].x, positions[0].y);
        lines[positions.size()].color = renderData.color;
        window->draw(lines);
    }

    void renderVelocity(Entity entity) {
        Velocity& velocityData = ecs->getData<Velocity>(entity);
        Position& position = ecs->getData<Position>(entity);

        std::vector<Vec2f> positions = position.positions;
        std::vector<Vec2f> velocities = velocityData.velocities;

        sf::VertexArray lines(sf::PrimitiveType::Lines, positions.size() * 2);

        for (size_t i = 0; i < positions.size(); i++) {
            lines[i * 2].position = sf::Vector2f(positions[i].x, positions[i].y);
            lines[i * 2].color = sf::Color::Green;
            lines[i * 2 + 1].position = sf::Vector2f(positions[i].x + velocities[i].x, positions[i].y + velocities[i].y);
            lines[i * 2 + 1].color = sf::Color::Green;
        }
        window->draw(lines);
    }

};
#endif // RENDER_SYSTEM_HPP