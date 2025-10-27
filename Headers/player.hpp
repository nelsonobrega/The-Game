#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <optional>
#include <iostream>
#include "ConfigManager.hpp" 
#include "Utils.hpp" // Incluir Utils para M_PI e outras funções

// *** NENHUMA DEFINIÇÃO DE CONSTANTE PI AQUI ***

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

    // VARIÁVEIS CARREGADAS DA CONFIGURAÇÃO (Sem problemas)
    int health = 0;
    float speed = 0.f;
    float isaacHitSpeed = 0.f;
    float maxHitDistance = 0.f;
    sf::Time hitFlashDuration;

    // VARIÁVEIS RESTAURADAS COMO CONSTANTES
    const float frame_duration = 0.1f;
    const sf::Time cooldownTime = sf::seconds(0.3f);
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