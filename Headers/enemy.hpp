#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <optional>
#include <cmath>
#include "ConfigManager.hpp"
#include "Utils.hpp" // Incluir Utils para acesso a checkCollision, calculateAngle e M_PI

// *** REMOVIDO: O bloco namespace Constants::PI_F foi removido para evitar redefinição.
// A constante M_PI já está disponível via Utils.hpp.

// Estrutura para os projéteis do inimigo
struct EnemyProjectile {
    sf::Sprite sprite;
    sf::Vector2f direction;
    float distanceTraveled;
};

// --- CLASSE BASE ---
class EnemyBase {
public:
    virtual ~EnemyBase() = default;

    void takeDamage(int amount);
    int getHealth() const { return health; }
    sf::FloatRect getGlobalBounds() const;
    std::vector<EnemyProjectile>& getProjectiles();

    virtual void update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) = 0;
    virtual void draw(sf::RenderWindow& window);

protected:
    EnemyBase();

    std::optional<sf::Sprite> sprite;
    int health = 1;
    float speed = 0.f;

    sf::Texture* projectileTexture = nullptr;
    sf::IntRect projectileTextureRect;

    float enemyHitSpeed = 0.f;
    float maxHitDistance = 0.f;

    std::vector<EnemyProjectile> projectiles;

    sf::Clock hitClock;
    sf::Time hitFlashDuration;
    bool isHit = false;

    sf::Clock healFlashClock;
    const sf::Time healFlashDuration = sf::seconds(2.0f);
    bool isHealed = false;
    void handleHealFlash();

    void updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds);
    void handleHitFlash();
};

// --- CLASSE DEMON ---
class Demon_ALL : public EnemyBase {
public:
    Demon_ALL(
        std::vector<sf::Texture>& walkDown,
        std::vector<sf::Texture>& walkUp,
        std::vector<sf::Texture>& walkLeft,
        std::vector<sf::Texture>& walkRight,
        sf::Texture& projectileTextureRef);

    void update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) override;

    void heal(int amount);
    void setProjectileTextureRect(const sf::IntRect& rect);

private:
    std::vector<sf::Texture>* textures_walk_down = nullptr;
    std::vector<sf::Texture>* textures_walk_up = nullptr;
    std::vector<sf::Texture>* textures_walk_left = nullptr;
    std::vector<sf::Texture>* textures_walk_right = nullptr;

    std::vector<sf::Texture>* last_animation_set = nullptr;
    int current_frame = 0;
    float animation_time = 0.0f;

    float frame_duration = 0.f;

    sf::Clock cooldownClock;
    sf::Time cooldownTime;
    sf::Clock attackDelayClock;
    sf::Time attackDelayTime;
    bool isPreparingAttack = false;
    sf::Vector2f targetPositionAtStartOfAttack;

    void handleMovementAndAnimation(float deltaTime, sf::Vector2f playerPosition, bool isAttacking);
    void handleAttack(sf::Vector2f playerPosition);
};

// --- CLASSE BISHOP (Suporte/Healer) ---
class Bishop_ALL : public EnemyBase {
public:
    Bishop_ALL(std::vector<sf::Texture>& walkTextures);

    void update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) override;

    bool shouldHealDemon() const;
    void resetHealFlag();

private:
    std::vector<sf::Texture>* textures_idle = nullptr;
    int current_frame = 0;
    float animation_time = 0.0f;

    float frame_duration = 0.f;
    int FRAME_B7_INDEX = 0;

    sf::Clock healClock;
    sf::Time healCooldown;
    bool canHealDemon = false;
    bool isChanting = false;

    float center_pull_weight = 0.0f;
    float lateral_bias_frequency = 0.0f;
    float lateral_bias_strength = 0.0f;

    void handleAnimation(float deltaTime);
};

// *** REMOVIDO: A declaração de calculateAngle foi removida, pois está em Utils.hpp.

#endif // ENEMY_HPP