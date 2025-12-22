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
    float distanceTraveled = 0.f;
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
    int getMaxHealthBonus() const; // Adicionado aqui
    std::vector<Projectile>& getProjectiles();

    // Setters e Comandos
    void setPosition(const sf::Vector2f& newPosition);
    void setSpeedMultiplier(float multiplier);
    void takeDamage(int amount);
    void setProjectileTextureRect(const sf::IntRect& rect);

    // --- MÉTODOS PARA ITENS ---
    void addSpeed(float amount);
    void heal(int amount);
    void increaseMaxHealth(int containers); // Adicionado aqui
    void setTearTexture(const sf::Texture& texture, const sf::IntRect& rect);

private:
    // Lógicas internas
    void handleMovementAndAnimation(float deltaTime);
    void handleAttack();
    void updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds);
    void handleHitFlash(float deltaTime);

    // Propriedades do Personagem
    std::optional<sf::Sprite> Isaac;
    int health;
    int maxHealthBonus; // Variável para o Blood Bag
    float speed;
    float speedMultiplier_;

    // Ataque
    float isaacHitSpeed;
    float maxHitDistance;
    sf::Clock cooldownClock;
    sf::Time cooldownTime;
    sf::IntRect projectileTextureRect;
    std::vector<Projectile> projectiles;
    sf::Texture* hitTexture;

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