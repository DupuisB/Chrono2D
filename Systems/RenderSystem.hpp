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
            // Draw all Rects
            if (ecs->hasComponent<RenderablePolygon>(entity)) {
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
        }
        window->display();
    }

};
#endif // RENDER_SYSTEM_HPP