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
            if (ecs->hasComponent<Renderable>(entity)) {
                Renderable& renderable = ecs->getData<Renderable>(entity);
                if (renderable.type == ShapeType::RECT) {
                    sf::RectangleShape rect;
                    rect.setPosition(sf::Vector2f(renderable.pointA.x, renderable.pointA.y));
                    rect.setSize(sf::Vector2f(renderable.pointB.x - renderable.pointA.x, renderable.pointB.y - renderable.pointA.y));
                    rect.setFillColor(sf::Color::Green);
                    rect.setOutlineColor(sf::Color::Red);
                    window->draw(rect);
                }
            }
        }
        window->display();
    }

};
#endif // RENDER_SYSTEM_HPP