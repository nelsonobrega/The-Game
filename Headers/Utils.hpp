#ifndef UTILS_HPP
#define UTILS_HPP

#include "SFML/Graphics.hpp"
#include <cmath>

#ifndef M_PI
// Define M_PI globalmente para todos que incluírem este header
#define M_PI 3.14159265358979323846
#endif

// DECLARAÇÕES GLOBAIS
bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b);

// NOVO: Sobrecarga do operador == para sf::Vector2i
inline bool operator==(const sf::Vector2i& left, const sf::Vector2i& right) {
    return left.x == right.x && left.y == right.y;
}

#endif // UTILS_HPP