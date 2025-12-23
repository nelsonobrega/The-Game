#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <string>

// Estrutura para os projéteis (Lágrimas/Pregos)
struct Projectile {
    sf::Sprite sprite;
    sf::Vector2f direction;
    float distanceTraveled;
    float damage;
    bool isNail; // Identifica se é um prego para lógica de hits
};

class Player_ALL {
public:
    Player_ALL(
        std::vector<sf::Texture>& walkDownTextures,
        sf::Texture& hitTextureRef,
        std::vector<sf::Texture>& walkUpTextures,
        std::vector<sf::Texture>& walkLeftTextures,
        std::vector<sf::Texture>& walkRightTextures
    );

    // Ciclo de Vida
    void update(float deltaTime, const sf::FloatRect& gameBounds);
    void draw(sf::RenderWindow& window);

    // Getters
    sf::Vector2f getPosition() const;
    sf::FloatRect getGlobalBounds() const;
    int getHealth() const;
    int getMaxHealthBonus() const;
    float getDamage() const;
    std::vector<Projectile>& getProjectiles();
    bool hasNailEffect() const { return hasEightInchNail; }

    // Setters e Comandos
    void setPosition(const sf::Vector2f& newPosition);
    void setSpeedMultiplier(float multiplier);
    void takeDamage(int amount);
    void setProjectileTextureRect(const sf::IntRect& rect);

    // --- MÉTODOS PARA ITENS ---
    void addSpeed(float amount);
    void addDamage(float amount);
    void heal(int amount);
    void increaseMaxHealth(int containers);
    void setTearTexture(const sf::Texture& texture, const sf::IntRect& rect);
    void setEightInchNail(bool active); // Ativa a mecânica de prego
    void incrementNailHits();           // Chamado pelo Game.cpp ao acertar inimigo
    void rage();

private:
    void handleMovementAndAnimation(float deltaTime);
    void handleAttack(float deltaTime); // Agora recebe deltaTime para o Charge
    void updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds);
    void handleHitFlash(float deltaTime);

    // Propriedades do Personagem
    std::optional<sf::Sprite> Isaac;
    int health;
    int maxHealthBonus;
    float speed;
    float speedMultiplier_;
    float damage;

    // Ataque e Projéteis
    float isaacHitSpeed;
    float maxHitDistance;
    sf::Clock cooldownClock;
    sf::Time cooldownTime;
    sf::IntRect projectileTextureRect;
    std::vector<Projectile> projectiles;
    sf::Texture* hitTexture;

    // --- MECÂNICA EIGHT INCH NAIL (CHARGE) ---
    bool hasEightInchNail = false;
    bool isCharging = false;
    float chargeTimer = 0.0f;
    int nailHits = 0;
    int currentChargeLevel = 0;
    sf::Vector2f lastChargeDir;
    float lastChargeRot;

    // Rects de evolução dos pregos
    std::vector<sf::IntRect> nailRects;
    std::vector<sf::IntRect> bloodNailRects;

    // Animação e Dano
    bool isHit = false;
    sf::Clock hitClock;
    sf::Time hitFlashDuration;
    sf::Time minDamageInterval;

    std::vector<sf::Texture>* textures_walk_down;
    std::vector<sf::Texture>* textures_walk_up;
    std::vector<sf::Texture>* textures_walk_left;
    std::vector<sf::Texture>* textures_walk_right;
    std::vector<sf::Texture>* last_animation_set;

    int current_frame = 0;
    float animation_time = 0.0f;
};

#endif