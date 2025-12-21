#ifndef CHUBBY_HPP
#define CHUBBY_HPP
#include "Enemy.hpp"
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

enum class ChubbyState {
    Idle,
    Moving,
    Attacking,
    Recovering
};

enum class FaceDir {
    Up,
    Down,
    Left,
    Right
};

class Chubby : public EnemyBase {
public:
    Chubby(sf::Texture& sheet, sf::Texture& projSheet);

    // Métodos Base
    void update(float deltaTime, sf::Vector2f playerPos, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(const sf::Vector2f& pos) override;

    // Getters para Colisão e Dano
    sf::FloatRect getGlobalBounds() const;
    sf::FloatRect getBoomerangBounds() const;
    bool isBoomerangActive() const { return boomerangActive; }
    int getDamage() const { return 2; }

private:
    // Componentes Visuais
    std::optional<sf::Sprite> sprite;
    std::optional<sf::Sprite> projectileSprite;
    float scaleFactor = 2.5f;
    float projectileTraveled;

    // Estados e Timers
    ChubbyState state;
    FaceDir faceDir;
    float stateTimer;
    float animTimer;
    int animFrame;

    // Lógica de Movimento
    sf::Vector2f moveDir;
    float distanceWalked;

    // Lógica do Bumerangue
    bool boomerangActive;
    bool boomerangReturn;
    sf::Vector2f projectilePos;
    sf::Vector2f projectileVel;

    // Funções Auxiliares Internas
    void updateAnimation(float deltaTime);
    void handleAttackSequence(float deltaTime, sf::Vector2f playerPos);
    void launchBoomerang();
    void updateBoomerang(float deltaTime);
};

#endif