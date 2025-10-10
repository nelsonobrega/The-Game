#include "player.hpp"
#include "SFML/Window/Keyboard.hpp"  
#include <algorithm>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- FUNÇÃO DE COLISÃO MANUAL LOCAL (Resolve o erro 'checkCollision' e LNK2005) ---
// Usar 'static' impede o erro LNK2005 por garantir que esta função é interna a este ficheiro.
// Usa rect.position.x e rect.size.x (SFML 3.0)
static bool checkCollisionManual(const sf::FloatRect& rect1, const sf::FloatRect& rect2) {
    // Colisão no eixo X
    bool x_overlap = rect1.position.x < rect2.position.x + rect2.size.x &&
        rect1.position.x + rect1.size.x > rect2.position.x;

    // Colisão no eixo Y
    bool y_overlap = rect1.position.y < rect2.position.y + rect2.size.y &&
        rect1.position.y + rect1.size.y > rect2.position.y;

    return x_overlap && y_overlap;
}
// -------------------------------------------------------------------------------------

void Player_ALL::setProjectileTextureRect(const sf::IntRect& rect) {
    projectileTextureRect = rect;
}

Player_ALL::Player_ALL(
    std::vector<sf::Texture>& walkDownTextures,
    sf::Texture& hitTextureRef)
{
    textures_walk_down = &walkDownTextures;
    hitTexture = &hitTextureRef;
    last_animation_set = textures_walk_down;

    if (textures_walk_down && !textures_walk_down->empty()) {
        Isaac.emplace(textures_walk_down->at(0));
        // CORREÇÃO: Usa sf::Vector2f explícito para compatibilidade SFML 3.0
        Isaac->setScale(sf::Vector2f(3.f, 3.f));
        Isaac->setOrigin(sf::Vector2f(13.5f, 17.f));
        Isaac->setPosition({ 960.f, 540.f });
    }

    if (hitTexture) {
        projectileTextureRect = sf::IntRect({ 0, 0 }, (sf::Vector2i)hitTexture->getSize());
    }
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

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
        current_animation_set = textures_walk_up;
        current_total_frames = 9;
        is_moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        current_animation_set = textures_walk_down;
        current_total_frames = 9;
        is_moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
        current_animation_set = textures_walk_left;
        current_total_frames = 6;
        is_moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
        current_animation_set = textures_walk_right;
        current_total_frames = 6;
        is_moving = true;
    }

    // Movimento do jogador
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) move += {0.f, 1.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) move += {0.f, -1.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) move += {-1.f, 0.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) move += {1.f, 0.f};

    // Prioriza animação de movimento se nenhuma tecla de animação direcional foi pressionada
    if (!is_moving) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
            current_animation_set = textures_walk_down;
            current_total_frames = 9;
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
            current_animation_set = textures_walk_up;
            current_total_frames = 9;
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
            current_animation_set = textures_walk_left;
            current_total_frames = 6;
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
            current_animation_set = textures_walk_right;
            current_total_frames = 6;
            is_moving = true;
        }
    }

    // Normaliza o movimento se for diagonal para evitar velocidade maior
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

    if (cooldownClock.getElapsedTime() >= cooldownTime) {
        struct KeyDir { sf::Keyboard::Scancode key; sf::Vector2f dir; float rot; };
        KeyDir dirs[4] = {
            {sf::Keyboard::Scancode::Up,    {0.f,-1.f}, -180.f},
            {sf::Keyboard::Scancode::Down,  {0.f,1.f},    0.f},
            {sf::Keyboard::Scancode::Left,  {-1.f,0.f},  90.f},
            {sf::Keyboard::Scancode::Right, {1.f,0.f},  -90.f}
        };

        for (auto& d : dirs) {
            if (sf::Keyboard::isKeyPressed(d.key)) {
                Projectile p = { sf::Sprite(*hitTexture), d.dir, 0.f };

                p.sprite.setTextureRect(projectileTextureRect);

                // Usa sf::Vector2f explícito para compatibilidade SFML 3.0
                p.sprite.setScale(sf::Vector2f(1.5f, 1.5f));
                p.sprite.setOrigin(sf::Vector2f(7.5f, 12.f));

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

        // SUBSTITUIÇÃO: Usa a função auxiliar checkCollisionManual()
        if (it->distanceTraveled >= maxHitDistance || !checkCollisionManual(projBounds, gameBounds))
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

    // Lógica de fixação de limites do jogador (com sintaxe SFML 3.0)
    newPos.x = std::min(std::max(newPos.x, gameBounds.position.x + isaacBounds.size.x / 2.f), gameBounds.position.x + gameBounds.size.x - isaacBounds.size.x / 2.f);
    newPos.y = std::min(std::max(newPos.y, gameBounds.position.y + isaacBounds.size.y / 2.f), gameBounds.position.y + gameBounds.size.y - isaacBounds.size.y / 2.f);
    Isaac->setPosition(newPos);
}


void Player_ALL::draw(sf::RenderWindow& window) {
    if (!Isaac || health <= 0) return;

    for (auto& p : projectiles) window.draw(p.sprite);

    window.draw(*Isaac);
}