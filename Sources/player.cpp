#include "player.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "ConfigManager.hpp" 
#include <algorithm>
#include <iostream>
#include <cmath>

// --- FUNÇÃO DE COLISÃO MANUAL LOCAL ---
static bool checkCollision(const sf::FloatRect& rect1, const sf::FloatRect& rect2) {
    bool x_overlap = rect1.position.x < rect2.position.x + rect2.size.x &&
        rect1.position.x + rect1.size.x > rect2.position.x;

    bool y_overlap = rect1.position.y < rect2.position.y + rect2.size.y &&
        rect1.position.y + rect1.size.y > rect2.position.y;

    return x_overlap && y_overlap;
}

// --- CONSTRUTOR ATUALIZADO (Carrega Configs - Removendo as falhas) ---
Player_ALL::Player_ALL(
    std::vector<sf::Texture>& walkDownTextures,
    sf::Texture& hitTextureRef,
    std::vector<sf::Texture>& walkUpTextures,
    std::vector<sf::Texture>& walkLeftTextures,
    std::vector<sf::Texture>& walkRightTextures)
{
    // CARREGA CONFIGURAÇÕES DO PLAYER
    const PlayerConfig& cfg = ConfigManager::getInstance().getConfig().player;

    health = cfg.initial_health;
    speed = cfg.speed;
    isaacHitSpeed = cfg.projectile_speed;

    maxHitDistance = cfg.projectile_max_distance;

    // hitFlashDuration é a única Time/Duration que a config fornece
    hitFlashDuration = sf::seconds(cfg.hit_flash_duration);


    textures_walk_down = &walkDownTextures;
    textures_walk_up = &walkUpTextures;
    textures_walk_left = &walkLeftTextures;
    textures_walk_right = &walkRightTextures;
    hitTexture = &hitTextureRef;
    last_animation_set = textures_walk_down;

    if (textures_walk_down && !textures_walk_down->empty()) {
        Isaac.emplace(textures_walk_down->at(0));
        Isaac->setScale(sf::Vector2f(cfg.scale, cfg.scale));
        Isaac->setOrigin(sf::Vector2f(cfg.origin_x, cfg.origin_y));
        Isaac->setPosition({ cfg.start_position_x, cfg.start_position_y });
    }

    if (hitTexture) {
        projectileTextureRect = sf::IntRect({ 0, 0 }, (sf::Vector2i)hitTexture->getSize());
    }
}

void Player_ALL::setProjectileTextureRect(const sf::IntRect& rect) {
    projectileTextureRect = rect;
}

void Player_ALL::takeDamage(int amount) {
    if (health > 0) {
        health = std::max(0, health - amount);
        isHit = true;
        hitClock.restart();
    }
}

sf::Vector2f Player_ALL::getPosition() const {
    return Isaac ? Isaac->getPosition() : sf::Vector2f(0.f, 0.f);
}

int Player_ALL::getHealth() const {
    return health;
}

std::vector<Projectile>& Player_ALL::getProjectiles() {
    return projectiles;
}

sf::FloatRect Player_ALL::getGlobalBounds() const {
    return Isaac ? Isaac->getGlobalBounds() : sf::FloatRect();
}

void Player_ALL::handleMovementAndAnimation(float deltaTime) {
    if (!Isaac) return;

    sf::Vector2f move(0.f, 0.f);
    bool is_moving = false;
    std::vector<sf::Texture>* current_animation_set = nullptr;
    int current_total_frames = 0;

    // --- Lógica de Animação Direcional (Setas) ---
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
        current_animation_set = textures_walk_up;
        current_total_frames = frames_vertical; // Usa frames_vertical (const)
        is_moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        current_animation_set = textures_walk_down;
        current_total_frames = frames_vertical; // Usa frames_vertical (const)
        is_moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
        current_animation_set = textures_walk_left;
        current_total_frames = frames_horizontal; // Usa frames_horizontal (const)
        is_moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
        current_animation_set = textures_walk_right;
        current_total_frames = frames_horizontal; // Usa frames_horizontal (const)
        is_moving = true;
    }

    // --- Movimento do jogador (WASD) ---
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) move += {0.f, 1.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) move += {0.f, -1.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) move += {-1.f, 0.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) move += {1.f, 0.f};

    // --- Priorização de Animação de Movimento (se nenhuma seta foi pressionada) ---
    if (!is_moving) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
            current_animation_set = textures_walk_down;
            current_total_frames = frames_vertical;
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
            current_animation_set = textures_walk_up;
            current_total_frames = frames_vertical;
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
            current_animation_set = textures_walk_left;
            current_total_frames = frames_horizontal;
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
            current_animation_set = textures_walk_right;
            current_total_frames = frames_horizontal;
            is_moving = true;
        }
    }

    // Normaliza o movimento
    float length = std::sqrt(move.x * move.x + move.y * move.y);
    if (length > 1.0f) {
        move /= length;
    }
    Isaac->move(move * deltaTime * speed);

    // Lógica de animação
    if (is_moving && current_animation_set) {
        if (current_animation_set != last_animation_set) {
            current_frame = 0;
            animation_time = 0.0f;
            last_animation_set = current_animation_set;
        }

        animation_time += deltaTime;

        if (animation_time >= frame_duration) {
            animation_time -= frame_duration;
            current_frame = (current_frame + 1) % current_total_frames;

            if (current_animation_set && current_frame < current_animation_set->size())
                Isaac->setTexture(current_animation_set->at(current_frame));
        }
    }
    else {
        current_frame = 0;
        animation_time = 0.0f;
        if (last_animation_set && !last_animation_set->empty())
            Isaac->setTexture(last_animation_set->at(0));
    }
}

void Player_ALL::handleAttack() {
    if (!Isaac || !hitTexture) return;

    // Usa cooldownTime (constante da classe)
    if (cooldownClock.getElapsedTime() >= cooldownTime) {
        struct KeyDir { sf::Keyboard::Scancode key; sf::Vector2f dir; float rot; };
        KeyDir dirs[4] = {
            {sf::Keyboard::Scancode::Up,    {0.f,-1.f}, -180.f},
            {sf::Keyboard::Scancode::Down,  {0.f,1.f},    0.f},
            {sf::Keyboard::Scancode::Left,  {-1.f,0.f},  90.f},
            {sf::Keyboard::Scancode::Right, {1.f,0.f},  -90.f}
        };

        // Carrega configurações de projétil
        const float projectileScale = ConfigManager::getInstance().getConfig().player.projectile_scale;
        const float origin_x = ConfigManager::getInstance().getConfig().player.projectile_origin_x;
        const float origin_y = ConfigManager::getInstance().getConfig().player.projectile_origin_y;


        for (auto& d : dirs) {
            if (sf::Keyboard::isKeyPressed(d.key)) {
                Projectile p = { sf::Sprite(*hitTexture), d.dir, 0.f };

                p.sprite.setTextureRect(projectileTextureRect);

                p.sprite.setScale(sf::Vector2f(projectileScale, projectileScale));
                p.sprite.setOrigin(sf::Vector2f(origin_x, origin_y));

                p.sprite.setPosition(Isaac->getPosition());
                p.sprite.setRotation(sf::degrees(d.rot));
                projectiles.push_back(p);
                cooldownClock.restart();
                break;
            }
        }
    }
}

void Player_ALL::updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds) {
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->sprite.move(it->direction * isaacHitSpeed * deltaTime);
        it->distanceTraveled += isaacHitSpeed * deltaTime;
        sf::FloatRect projBounds = it->sprite.getGlobalBounds();

        if (it->distanceTraveled >= maxHitDistance || !checkCollision(projBounds, gameBounds))
            it = projectiles.erase(it);
        else
            ++it;
    }
}

void Player_ALL::handleHitFlash(float deltaTime) {
    if (!Isaac) return;

    if (isHit) {
        if (hitClock.getElapsedTime() < hitFlashDuration) {
            Isaac->setColor(sf::Color::Red);
        }
        else {
            Isaac->setColor(sf::Color::White);
            isHit = false;
        }
    }
}

void Player_ALL::update(float deltaTime, const sf::FloatRect& gameBounds) {
    if (!Isaac || health <= 0) return;

    handleMovementAndAnimation(deltaTime);
    handleAttack();
    handleHitFlash(deltaTime);
    updateProjectiles(deltaTime, gameBounds);

    sf::Vector2f newPos = Isaac->getPosition();
    sf::FloatRect isaacBounds = Isaac->getGlobalBounds();

    newPos.x = std::min(std::max(newPos.x, gameBounds.position.x + isaacBounds.size.x / 2.f), gameBounds.position.x + gameBounds.size.x - isaacBounds.size.x / 2.f);
    newPos.y = std::min(std::max(newPos.y, gameBounds.position.y + isaacBounds.size.y / 2.f), gameBounds.position.y + gameBounds.size.y - isaacBounds.size.y / 2.f);
    Isaac->setPosition(newPos);
}


void Player_ALL::draw(sf::RenderWindow& window) {
    if (!Isaac || health <= 0) return;

    for (auto& p : projectiles) window.draw(p.sprite);

    window.draw(*Isaac);
}