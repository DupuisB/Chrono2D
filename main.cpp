// Define this before including cmath if needed (often required on Windows/MSVC)
#define _USE_MATH_DEFINES
#include <cmath> // For std::cos, std::sin, std::hypot, M_PI


#include <SFML/Graphics.hpp>
#include <vector>
#include <numeric> // For std::copysign
#include <iostream>
#include <optional> // For SFML 3 event handling
#include <algorithm> // For std::min/max
#include <limits>    // For numeric_limits

int main() {
    sf::Window window;
    window.create(sf::VideoMode({900, 600}), "Game", sf::Style::Default);

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
    }

    return 0;
}