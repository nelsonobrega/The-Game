#ifndef UTILS_HPP
#define UTILS_HPP

#include "SFML/Graphics.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// DECLARAÇÕES GLOBAIS
// Estas são as únicas cópias globais.
float calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2);

#endif // UTILS_HPP