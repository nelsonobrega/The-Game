#ifndef CHUBBY_HPP
#define CHUBBY_HPP

#include "Enemy.hpp"

enum class ChubbyState { Idle, Moving, Attacking, Recovering };

class Chubby : public EnemyBase {
public:
    Chubby(sf::Texture& sheet, sf::Texture& projSheet);

    void update(float deltaTime, sf::Vector2f playerPos, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(const sf::Vector2f& pos) override;
    sf::FloatRect getGlobalBounds() const override;

    void takeDamage(int amount) override;

    // Método de cura adicionado
    void heal(int amount) override;

    bool BoomerangActive() const { return boomerangActive; }
    sf::FloatRect getBoomerangBounds() const;

private:
    ChubbyState state;
    FaceDir faceDir;
    sf::Vector2f moveDir;
    float stateTimer;
    float animTimer;
    int animFrame;
    float scaleFactor;

    // Bumerangue
    std::optional<sf::Sprite> projectileSprite;
    sf::Vector2f projectilePos;
    sf::Vector2f projectileVel;
    bool boomerangActive;
    bool boomerangReturn;
    float distanceWalked;

    void updateAnimation(float deltaTime);
    void handleAttackSequence(float deltaTime, sf::Vector2f playerPos);
    void launchBoomerang();
    void updateBoomerang(float deltaTime);
};

#endif