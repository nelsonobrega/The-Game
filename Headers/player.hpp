#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <optional>
#include <cmath>

// NOVO: Incluir o ficheiro de utilidade para ter acesso às declarações
#include "Utils.hpp" 

struct Projectile {
    sf::Sprite sprite;
    sf::Vector2f direction;
    float distanceTraveled;
};

// Declarações de checkCollision e calculateAngle REMOVIDAS daqui
// (estão agora em Utils.hpp)

class Player_ALL {
public:
    Player_ALL(std::vector<sf::Texture>& walkDownTextures, sf::Texture& hitTexture);

    void update(float deltaTime, const sf::FloatRect& gameBounds);
    void draw(sf::RenderWindow& window);
    void takeDamage(int amount);

    sf::Vector2f getPosition() const;
    int getHealth() const;
    sf::FloatRect getGlobalBounds() const;
    std::vector<Projectile>& getProjectiles();

    std::vector<sf::Texture>* textures_walk_up = nullptr;
    std::vector<sf::Texture>* textures_walk_left = nullptr;
    std::vector<sf::Texture>* textures_walk_right = nullptr;

private:
    std::optional<sf::Sprite> Isaac;
    std::vector<Projectile> projectiles;

    std::vector<sf::Texture>* textures_walk_down = nullptr;
    sf::Texture* hitTexture = nullptr;
    std::vector<sf::Texture>* last_animation_set = nullptr;

    int health = 6;
    const float speed = 300.f;
    const float maxHitDistance = 600.f;

    int current_frame = 0;
    float animation_time = 0.0f;
    const float frame_duration = 0.100f;

    sf::Clock cooldownClock;
    sf::Time cooldownTime = sf::seconds(0.3f);
    const float isaacHitSpeed = 600.f;

    sf::Clock hitClock;
    const sf::Time hitFlashDuration = sf::milliseconds(100);
    bool isHit = false;

    void handleMovementAndAnimation(float deltaTime);
    void handleAttack();
    void handleHitFlash(float deltaTime);
    void updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds);
};

#endif // PLAYER_HPP