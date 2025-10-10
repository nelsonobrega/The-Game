#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <optional>
#include <cmath>

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
    std::optional<sf::Sprite> sprite;
    int health = 1;
    float speed = 0.f;

    sf::Texture* projectileTexture = nullptr;
    sf::IntRect projectileTextureRect;
    const float enemyHitSpeed = 700.f;
    const float maxHitDistance = 800.f;

    std::vector<EnemyProjectile> projectiles;

    // --- VARIÁVEIS DE HIT FLASH (Vermelho) ---
    sf::Clock hitClock;
    const sf::Time hitFlashDuration = sf::milliseconds(100);
    bool isHit = false;

    // --- VARIÁVEIS DE HEAL FLASH (Verde) ---
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
    const float frame_duration = 0.12f;

    sf::Clock cooldownClock;
    sf::Time cooldownTime = sf::seconds(2.0f);
    sf::Clock attackDelayClock;
    sf::Time attackDelayTime = sf::seconds(0.5f);
    bool isPreparingAttack = false;
    sf::Vector2f targetPositionAtStartOfAttack;

    // Função de movimento e animação que recebe o estado de ataque
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
    const float frame_duration = 0.08f;
    const int FRAME_B7_INDEX = 6;

    sf::Clock healClock;
    sf::Time healCooldown = sf::seconds(7.0f);
    bool canHealDemon = false;
    bool isChanting = false;

    void handleAnimation(float deltaTime);
};

// Funções utilitárias
float calculateAngle(sf::Vector2f p1, sf::Vector2f p2);

#endif // ENEMY_HPP