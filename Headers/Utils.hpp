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

#endif // UTILS_HPP