#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <optional>
#include <iostream>
#include "ConfigManager.hpp" 
#include "Utils.hpp" 

struct Projectile {
    sf::Sprite sprite;
    sf::Vector2f direction;
    float distanceTraveled;
};

class Player_ALL {
public:
    Player_ALL(
        std::vector<sf::Texture>& walkDownTextures,
        sf::Texture& hitTextureRef,
        std::vector<sf::Texture>& walkUpTextures,
        std::vector<sf::Texture>& walkLeftTextures,
        std::vector<sf::Texture>& walkRightTextures);

    // --- NOVOS MÉTODOS PARA O ROOMMANAGER ---
    void setPosition(const sf::Vector2f& newPosition);
    void setSpeedMultiplier(float multiplier);
    // ----------------------------------------

    void setProjectileTextureRect(const sf::IntRect& rect);

    void takeDamage(int amount);
    void update(float deltaTime, const sf::FloatRect& gameBounds);
    void draw(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    int getHealth() const;
    std::vector<Projectile>& getProjectiles();
    sf::FloatRect getGlobalBounds() const;

    std::vector<sf::Texture>* textures_walk_up = nullptr;
    std::vector<sf::Texture>* textures_walk_left = nullptr;
    std::vector<sf::Texture>* textures_walk_right = nullptr;

private:
    std::optional<sf::Sprite> Isaac;

    // VARIÁVEIS CARREGADAS DA CONFIGURAÇÃO
    int health = 0;
    float speed = 0.f; // Velocidade base
    float isaacHitSpeed = 0.f;
    float maxHitDistance = 0.f;
    sf::Time hitFlashDuration;

    // Intervalo mínimo entre aplicações de dano (permite múltiplos projéteis)
    sf::Time minDamageInterval = sf::seconds(0.02f);

    // NOVO: Multiplicador de velocidade (1.0 = normal, 0.0 = parado)
    float speedMultiplier_ = 1.0f;

    // VARIÁVEIS RESTAURADAS COMO CONSTANTES
    const float frame_duration = 0.1f;
    sf::Time cooldownTime;
    const int frames_vertical = 9;
    const int frames_horizontal = 6;


    std::vector<sf::Texture>* textures_walk_down = nullptr;
    sf::Texture* hitTexture = nullptr;
    sf::IntRect projectileTextureRect;

    std::vector<sf::Texture>* last_animation_set = nullptr;
    int current_frame = 0;
    float animation_time = 0.0f;

    sf::Clock cooldownClock;
    sf::Clock hitClock;
    bool isHit = false;

    std::vector<Projectile> projectiles;

    void handleMovementAndAnimation(float deltaTime);
    void handleAttack();
    void updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds);
    void handleHitFlash(float deltaTime);
};

#endif // PLAYER_HPP