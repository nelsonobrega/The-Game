#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <optional>
#include <iostream>
#include <cmath>

bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b);
float calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2);

struct EnemyProjectile {
    sf::Sprite sprite;
    sf::Vector2f direction;
    float distanceTraveled;
};

class EnemyBase {
public:
    virtual void update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void takeDamage(int amount);

    sf::FloatRect getGlobalBounds() const;
    int getHealth() const { return health; }
    sf::Vector2f getPosition() const { return sprite ? sprite->getPosition() : sf::Vector2f(); }
    bool isHitFlashActive() const;
    bool isHealFlashActive() const;
    std::vector<EnemyProjectile>& getProjectiles();

    sf::Sprite& getSprite();

protected:
    std::optional<sf::Sprite> sprite;
    int health;
    float speed;

    sf::Clock hitClock;
    const sf::Time hitFlashDuration = sf::milliseconds(100);
    bool isHit = false;

    sf::Clock healClock;
    const sf::Time healFlashDuration = sf::seconds(2.0f);
    bool isHealed = false;

    std::vector<EnemyProjectile> projectiles;
    const float maxHitDistance = 600.f;
    const float enemyHitSpeed = 950.f;

    void handleHitFlash();
    void updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds);
};

class Demon_ALL : public EnemyBase {
public:
    Demon_ALL(
        std::vector<sf::Texture>& walkDown,
        std::vector<sf::Texture>& walkUp,
        std::vector<sf::Texture>& walkLeft,
        std::vector<sf::Texture>& walkRight,
        sf::Texture& projectileTexture);

    void update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;

    void takeDamage(int amount) override;
    void heal(int amount);

    sf::Sprite& getSprite() { return EnemyBase::getSprite(); }


private:
    std::vector<sf::Texture>* textures_walk_down = nullptr;
    std::vector<sf::Texture>* textures_walk_up = nullptr;
    std::vector<sf::Texture>* textures_walk_left = nullptr;
    std::vector<sf::Texture>* textures_walk_right = nullptr;
    sf::Texture* projectileTexture = nullptr;

    int current_frame = 0;
    float animation_time = 0.0f;
    const float frame_duration = 0.12f;

    sf::Clock cooldownClock;
    sf::Time cooldownTime = sf::seconds(3.5f);

    void handleMovementAndAnimation(float deltaTime, sf::Vector2f dirToPlayer);
    void handleAttack(sf::Vector2f dirToPlayer, sf::Vector2f playerPosition);
    void handleHealFlash();

};

class Bishop_ALL : public EnemyBase {
public:
    Bishop_ALL(std::vector<sf::Texture>& animationTextures);

    void update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;

    bool shouldHealDemon() const;
    void resetHealFlag();

    sf::Sprite& getSprite() { return EnemyBase::getSprite(); }


private:
    std::vector<sf::Texture>* textures_animation = nullptr;

    int current_frame = 0;
    float animation_time = 0.0f;
    const float frame_duration = 0.08f;
    const int TOTAL_FRAMES = 14;

    sf::Clock cooldownClock;
    const sf::Time cooldown_time = sf::seconds(7.0f);
    bool is_animating = false;

    const float long_pause_duration = 1.0f;
    float long_pause_time = 0.0f;
    bool healDemonFlag = false;
};
#endif // ENEMY_HPP