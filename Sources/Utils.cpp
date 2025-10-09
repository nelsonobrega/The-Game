#include "Utils.hpp"
#include <cmath>
#include <algorithm>

// DEFINI��ES �NICAS (N�O USAR 'static'!)
// Esta � a �nica vez que as fun��es s�o definidas. O linker n�o ter� conflitos.
bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b)
{
    bool x_overlap = a.position.x < b.position.x + b.size.x &&
        a.position.x + a.size.x > b.position.x;
    bool y_overlap = a.position.y < b.position.y + b.size.y &&
        a.position.y + a.size.y > b.position.y;
    return x_overlap && y_overlap;
}

float calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
    sf::Vector2f direction = p2 - p1;
    return static_cast<float>(std::atan2(direction.y, direction.x) * 180.0 / M_PI);
}