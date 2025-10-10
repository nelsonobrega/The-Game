#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <optional>
#include <iostream>

struct Projectile {
    sf::Sprite sprite;
    sf::Vector2f direction;
    float distanceTraveled;
};

class Player_ALL {
public:
    Player_ALL(
        std::vector<sf::Texture>& walkDownTextures,
        sf::Texture& hitTextureRef);

    void setProjectileTextureRect(const sf::IntRect& rect);

    void takeDamage(int amount);
    void update(float deltaTime, const sf::FloatRect& gameBounds);
    void draw(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    int getHealth() const;
    std::vector<Projectile>& getProjectiles();
    sf::FloatRect getGlobalBounds() const;

    // Ponteiros para as texturas (definidos na Game)
    std::vector<sf::Texture>* textures_walk_up = nullptr;
    std::vector<sf::Texture>* textures_walk_left = nullptr;
    std::vector<sf::Texture>* textures_walk_right = nullptr;

private:
    std::optional<sf::Sprite> Isaac;
    int health = 6;
    float speed = 350.f;
    const float isaacHitSpeed = 750.f;
    const float maxHitDistance = 900.f;

    std::vector<sf::Texture>* textures_walk_down = nullptr;
    sf::Texture* hitTexture = nullptr;
    sf::IntRect projectileTextureRect;

    std::vector<sf::Texture>* last_animation_set = nullptr;
    int current_frame = 0;
    float animation_time = 0.0f;
    const float frame_duration = 0.1f;

    sf::Clock cooldownClock;
    sf::Time cooldownTime = sf::seconds(0.3f);

    sf::Clock hitClock;
    const sf::Time hitFlashDuration = sf::milliseconds(100);
    bool isHit = false;

    std::vector<Projectile> projectiles;

    void handleMovementAndAnimation(float deltaTime);
    void handleAttack();
    void updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds);
    void handleHitFlash(float deltaTime);
};

#endif // PLAYER_HPP