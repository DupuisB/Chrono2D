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
            if (ecs->hasComponent<RenderableRect>(entity)) {
                RenderableRect& renderData = ecs->getData<RenderableRect>(entity);
                RigidRect& rigidRect = ecs->getData<RigidRect>(entity);

                std::array<Vec2f, 4> positions = rigidRect.getPositions();

                sf::VertexArray lines(sf::PrimitiveType::LineStrip, 5);
                for (int i = 0; i < 4; i++) {
                    lines[i].position = sf::Vector2f(positions[i].x, positions[i].y);
                    lines[i].color = renderData.color;
                }
                lines[4].position = sf::Vector2f(positions[0].x, positions[0].y);
                lines[4].color = renderData.color;
                window->draw(lines);
            }
        }
        window->display();
    }

};
#endif // RENDER_SYSTEM_HPP