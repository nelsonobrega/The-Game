#include "enemy.hpp"
#include <algorithm>
#include <stdexcept> 
    

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// DEFINIÇÕES de checkCollision e calculateAngle REMOVIDAS DAQUI!
// Elas agora estão APENAS em Utils.cpp.

// --- Base Class Implementations ---
void EnemyBase::takeDamage(int amount) {
    if (health > 0) {
        health = std::max(0, health - amount);
        isHit = true;
        hitClock.restart();
    }
}

sf::FloatRect EnemyBase::getGlobalBounds() const {
    return sprite ? sprite->getGlobalBounds() : sf::FloatRect();
}

sf::Sprite& EnemyBase::getSprite() {
    if (!sprite) {
        throw std::runtime_error("Erro: Tentativa de obter sprite não inicializada!");
    }
    return *sprite;
}

bool EnemyBase::isHitFlashActive() const {
    return isHit;
}

bool EnemyBase::isHealFlashActive() const {
    return isHealed;
}

std::vector<EnemyProjectile>& EnemyBase::getProjectiles() {
    return projectiles;
}

void EnemyBase::handleHitFlash() {
    if (!sprite) return;

    if (isHit) {
        if (hitClock.getElapsedTime() < hitFlashDuration) {
            sprite->setColor(sf::Color::Red);
        }
        else {
            isHit = false;
            if (!isHealed) {
                sprite->setColor(sf::Color::White);
            }
        }
    }
}

void EnemyBase::updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds) {
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->sprite.move(it->direction * enemyHitSpeed * deltaTime);
        it->distanceTraveled += enemyHitSpeed * deltaTime;
        sf::FloatRect projBounds = it->sprite.getGlobalBounds();

        sf::Vector2f center = projBounds.position + projBounds.size / 2.f;

        bool is_outside_bounds = center.x < gameBounds.position.x ||
            center.x > gameBounds.position.x + gameBounds.size.x ||
            center.y < gameBounds.position.y ||
            center.y > gameBounds.position.y + gameBounds.size.y;


        if (it->distanceTraveled >= maxHitDistance || is_outside_bounds)
            it = projectiles.erase(it);
        else
            ++it;
    }
}

void EnemyBase::draw(sf::RenderWindow& window) {
    if (!sprite || health <= 0) return;
    for (auto& p : projectiles) window.draw(p.sprite);
    window.draw(*sprite);
}

// --- Demon_ALL Implementations ---
Demon_ALL::Demon_ALL(
    std::vector<sf::Texture>& walkDown,
    std::vector<sf::Texture>& walkUp,
    std::vector<sf::Texture>& walkLeft,
    std::vector<sf::Texture>& walkRight,
    sf::Texture& projectileTextureRef)
{
    health = 10;
    speed = 100.f;

    textures_walk_down = &walkDown;
    textures_walk_up = &walkUp;
    textures_walk_left = &walkLeft;
    textures_walk_right = &walkRight;
    projectileTexture = &projectileTextureRef;

    if (textures_walk_down && !textures_walk_down->empty()) {
        sprite.emplace(textures_walk_down->at(0));
        sprite->setScale({ 3.f, 3.f });
        sprite->setPosition({ 960.f, 540.f });
        sprite->setOrigin({ 14.f, 16.f });
    }
}

void Demon_ALL::takeDamage(int amount) {
    EnemyBase::takeDamage(amount);
    isHealed = false;
    if (sprite) sprite->setColor(sf::Color::Red);
}

void Demon_ALL::heal(int amount) {
    health = std::min(10, health + amount);
    isHealed = true;
    healClock.restart();
    isHit = false;
}

void Demon_ALL::handleHealFlash() {
    if (!sprite) return;

    if (isHealed) {
        if (healClock.getElapsedTime() < healFlashDuration) {
            sprite->setColor(sf::Color::Green);
        }
        else {
            isHealed = false;
            sprite->setColor(sf::Color::White);
        }
    }
}

void Demon_ALL::handleMovementAndAnimation(float deltaTime, sf::Vector2f dirToPlayer, const sf::FloatRect& gameBounds) {
    std::vector<sf::Texture>* current_animation_set = textures_walk_down;
    int current_total_frames = 8;

    if (std::abs(dirToPlayer.x) > std::abs(dirToPlayer.y)) {
        current_animation_set = (dirToPlayer.x > 0) ? textures_walk_right : textures_walk_left;
    }
    else {
        current_animation_set = (dirToPlayer.y > 0) ? textures_walk_down : textures_walk_up;
    }

    if (!current_animation_set) return;

    animation_time += deltaTime;
    if (animation_time >= frame_duration) {
        animation_time -= frame_duration;
        current_frame = (current_frame + 1) % current_total_frames;

        if (current_frame < current_animation_set->size())
            sprite->setTexture(current_animation_set->at(current_frame));
    }

    sf::Vector2f movement = dirToPlayer * speed * deltaTime;

    sf::FloatRect currentBounds = sprite->getGlobalBounds();
    sf::FloatRect tentativeBounds = currentBounds;

    tentativeBounds.position.x += movement.x;
    tentativeBounds.position.y += movement.y;

    bool is_within_bounds = tentativeBounds.position.x >= gameBounds.position.x &&
        tentativeBounds.position.y >= gameBounds.position.y &&
        tentativeBounds.position.x + tentativeBounds.size.x <= gameBounds.position.x + gameBounds.size.x &&
        tentativeBounds.position.y + tentativeBounds.size.y <= gameBounds.position.y + gameBounds.size.y;

    if (is_within_bounds) {
        sprite->move(movement);
    }
}

void Demon_ALL::handleAttack(sf::Vector2f dirToPlayer, sf::Vector2f playerPosition) {
    if (!sprite || !projectileTexture) return;

    if (cooldownClock.getElapsedTime() >= cooldownTime) {
        EnemyProjectile p = { sf::Sprite(*projectileTexture), dirToPlayer, 0.f };
        p.sprite.setOrigin({ 4.f / 2.f, 74.f / 2.f });
        p.sprite.setPosition(sprite->getPosition());
        // calculateAngle agora chama a versão global do Utils.cpp
        float angle = calculateAngle(sprite->getPosition(), playerPosition);
        p.sprite.setRotation(sf::degrees(angle - 90.f));
        projectiles.push_back(p);
        cooldownClock.restart();
    }
}

void Demon_ALL::update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) {
    if (!sprite || health <= 0) return;

    sf::Vector2f demonPos = sprite->getPosition();
    sf::Vector2f dirToPlayer = playerPosition - demonPos;
    float distance = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);

    if (distance > 0.f) {
        dirToPlayer /= distance;
    }
    else {
        dirToPlayer = { 0.f, 0.f };
    }

    handleMovementAndAnimation(deltaTime, dirToPlayer, gameBounds);
    handleAttack(dirToPlayer, playerPosition);
    updateProjectiles(deltaTime, gameBounds);

    handleHitFlash();
    handleHealFlash();
}

void Demon_ALL::draw(sf::RenderWindow& window) {
    EnemyBase::draw(window);
}

// --- Bishop_ALL Implementations ---
Bishop_ALL::Bishop_ALL(std::vector<sf::Texture>& animationTextures, int initialHealth)
{
    health = initialHealth;
    speed = 0.f;

    textures_animation = &animationTextures;

    if (textures_animation && !textures_animation->empty()) {
        sprite.emplace(textures_animation->at(0));
        sprite->setScale({ 3.f, 3.f });
        sprite->setPosition({ 1500.f, 540.f });
        sprite->setOrigin({ 14.f, 16.f });
    }
    cooldownClock.restart();
}

void Bishop_ALL::update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) {
    if (!sprite || health <= 0 || !textures_animation) return;

    healDemonFlag = false;

    if (!is_animating) {
        if (cooldownClock.getElapsedTime() >= cooldown_time) {
            is_animating = true;
            current_frame = 1;
            if (current_frame < TOTAL_FRAMES) sprite->setTexture(textures_animation->at(current_frame));
        }
    }
    else {
        if (current_frame == 8) {
            long_pause_time += deltaTime;
            if (long_pause_time >= long_pause_duration) {
                current_frame++;
                long_pause_time = 0.0f;
            }
        }
        else {
            animation_time += deltaTime;
            if (animation_time >= frame_duration) {
                animation_time -= frame_duration;
                current_frame++;

                if (current_frame == 8) {
                    healDemonFlag = true;
                }

                if (current_frame >= TOTAL_FRAMES) {
                    is_animating = false;
                    current_frame = 0;
                    cooldownClock.restart();
                }

                if (current_frame < TOTAL_FRAMES) {
                    sprite->setTexture(textures_animation->at(current_frame));
                }
            }
        }
    }

    handleHitFlash();
}

void Bishop_ALL::draw(sf::RenderWindow& window) {
    EnemyBase::draw(window);
}

bool Bishop_ALL::shouldHealDemon() const {
    return healDemonFlag;
}

void Bishop_ALL::resetHealFlag() {
    healDemonFlag = false;
}