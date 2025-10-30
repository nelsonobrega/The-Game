#include "enemy.hpp"
#include "ConfigManager.hpp" 
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cmath> 

// --- Funções Auxiliares ---
float calculateAngle(sf::Vector2f p1, sf::Vector2f p2) {
    sf::Vector2f diff = p2 - p1;
    // Usa Constants::PI_F
    return std::atan2f(-diff.y, diff.x) * 180.f / M_PI;
}


// --- Implementações de EnemyBase ---

EnemyBase::EnemyBase() {
    const PlayerConfig& player_cfg = ConfigManager::getInstance().getConfig().player;
    hitFlashDuration = sf::seconds(player_cfg.hit_flash_duration);
}

void EnemyBase::handleHealFlash() {
    if (!sprite) return;

    if (isHealed) {
        if (healFlashClock.getElapsedTime() >= healFlashDuration) {
            sprite->setColor(sf::Color::White);
            isHealed = false;
        }
        else {
            sprite->setColor(sf::Color::Green);
        }
    }
}

void EnemyBase::takeDamage(int amount) {
    if (health > 0) {
        health = std::max(0, health - amount);
        isHit = true;
        hitClock.restart();
    }
}

void EnemyBase::handleHitFlash() {
    if (!sprite) return;

    if (isHit) {
        if (hitClock.getElapsedTime() < hitFlashDuration) {
            sprite->setColor(sf::Color::Red);
        }
        else {
            if (!isHealed) {
                sprite->setColor(sf::Color::White);
            }
            isHit = false;
        }
    }
}

sf::FloatRect EnemyBase::getGlobalBounds() const {
    return sprite ? sprite->getGlobalBounds() : sf::FloatRect();
}

std::vector<EnemyProjectile>& EnemyBase::getProjectiles() {
    return projectiles;
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


// --- Implementações de Demon_ALL ---

Demon_ALL::Demon_ALL(
    std::vector<sf::Texture>& walkDown,
    std::vector<sf::Texture>& walkUp,
    std::vector<sf::Texture>& walkLeft,
    std::vector<sf::Texture>& walkRight,
    sf::Texture& projectileTextureRef)
    : EnemyBase()
{
    const DemonConfig& cfg = ConfigManager::getInstance().getConfig().demon;

    health = cfg.initial_health;
    speed = cfg.speed;
    frame_duration = cfg.frame_duration;

    cooldownTime = sf::seconds(cfg.fire_cooldown);
    attackDelayTime = sf::seconds(cfg.attack_delay);

    enemyHitSpeed = cfg.projectile_speed;
    maxHitDistance = cfg.projectile_max_distance;

    projectileTexture = &projectileTextureRef;
    textures_walk_down = &walkDown;
    textures_walk_up = &walkUp;
    textures_walk_left = &walkLeft;
    textures_walk_right = &walkRight;
    last_animation_set = textures_walk_down;

    if (textures_walk_down && !textures_walk_down->empty()) {
        sprite.emplace(textures_walk_down->at(0));
        sprite->setScale(sf::Vector2f(cfg.scale, cfg.scale));
        sprite->setPosition({ cfg.start_position_x, cfg.start_position_y });
        sprite->setOrigin(sf::Vector2f(cfg.origin_x, cfg.origin_y));
    }

    if (projectileTexture) {
        projectileTextureRect = sf::IntRect({ 0, 0 }, (sf::Vector2i)projectileTexture->getSize());
    }
}

void Demon_ALL::setProjectileTextureRect(const sf::IntRect& rect) {
    projectileTextureRect = rect;
}

void Demon_ALL::heal(int amount) {
    const int MAX_HEALTH = ConfigManager::getInstance().getConfig().demon.max_health;
    health = std::min(MAX_HEALTH, health + amount);

    isHealed = true;
    healFlashClock.restart();
}

void Demon_ALL::handleMovementAndAnimation(float deltaTime, sf::Vector2f playerPosition, bool isAttacking) {
    if (!sprite) return;

    const int current_total_frames = ConfigManager::getInstance().getConfig().demon.animation_frames;

    sf::Vector2f currentPos = sprite->getPosition();
    sf::Vector2f move = playerPosition - currentPos;

    float length = std::sqrtf(move.x * move.x + move.y * move.y);
    if (length > 0.0f) {
        move /= length;
    }

    if (!isAttacking && length > 100.f) {
        sprite->move(move * deltaTime * speed);
    }

    std::vector<sf::Texture>* current_animation_set = textures_walk_down;
    bool isAnimating = length > 10.f || isAttacking;

    if (length > 10.f) {
        if (std::abs(move.x) > std::abs(move.y)) {
            if (move.x > 0) current_animation_set = textures_walk_right;
            else current_animation_set = textures_walk_left;
        }
        else {
            if (move.y > 0) current_animation_set = textures_walk_down;
            else current_animation_set = textures_walk_up;
        }
    }
    else {
        current_animation_set = last_animation_set;
    }

    if (current_animation_set && current_animation_set != last_animation_set) {
        current_frame = 0;
        animation_time = 0.0f;
        last_animation_set = current_animation_set;
    }

    if (isAnimating) {
        animation_time += deltaTime;

        if (animation_time >= frame_duration) {
            animation_time -= frame_duration;
            current_frame = (current_frame + 1) % current_total_frames;
        }
    }
    else {
        current_frame = 0;
        animation_time = 0.0f;
    }

    if (current_animation_set && current_animation_set->size() > 0 && current_frame < current_animation_set->size()) {
        sprite->setTexture(current_animation_set->at(current_frame));
    }
}

void Demon_ALL::handleAttack(sf::Vector2f playerPosition) {
    if (!sprite || !projectileTexture) return;

    const DemonConfig& cfg = ConfigManager::getInstance().getConfig().demon;

    if (!isPreparingAttack) {
        if (cooldownClock.getElapsedTime() >= cooldownTime) {
            isPreparingAttack = true;
            attackDelayClock.restart();
            targetPositionAtStartOfAttack = playerPosition;
        }
    }

    if (isPreparingAttack) {
        if (attackDelayClock.getElapsedTime() >= attackDelayTime) {

            float baseAngle = calculateAngle(sprite->getPosition(), targetPositionAtStartOfAttack);

            const int numProjectiles = cfg.projectile_count;
            const float spreadAngle = cfg.projectile_spread;
            const float projectileScale = cfg.projectile_scale;

            float startOffset = -(spreadAngle / 2.0f);
            float step = (numProjectiles > 1) ? spreadAngle / (numProjectiles - 1) : 0.0f;


            for (int i = 0; i < numProjectiles; ++i) {
                float angleOffset = startOffset + i * step;
                float finalAngle = baseAngle + angleOffset;
                // Usa Constants::PI_F
                float angleRad = finalAngle * (M_PI / 180.f);

                sf::Vector2f direction = { std::cosf(angleRad), -std::sinf(angleRad) };

                EnemyProjectile p = { sf::Sprite(*projectileTexture), direction, 0.f };
                p.sprite.setScale(sf::Vector2f(projectileScale, projectileScale));
                p.sprite.setTextureRect(projectileTextureRect);
                p.sprite.setOrigin(sf::Vector2f(8.f, 8.f));
                p.sprite.setPosition(sprite->getPosition());
                p.sprite.setRotation(sf::degrees(finalAngle - 90.f));
                projectiles.push_back(p);
            }

            isPreparingAttack = false;
            cooldownClock.restart();
        }
    }
}

void Demon_ALL::update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) {
    if (health <= 0) return;

    handleMovementAndAnimation(deltaTime, playerPosition, isPreparingAttack);

    handleAttack(playerPosition);
    updateProjectiles(deltaTime, gameBounds);

    handleHitFlash();
    handleHealFlash();

    sf::Vector2f newPos = sprite->getPosition();
    sf::FloatRect demonBounds = sprite->getGlobalBounds();

    newPos.x = std::min(std::max(newPos.x, gameBounds.position.x + demonBounds.size.x / 2.f), gameBounds.position.x + gameBounds.size.x - demonBounds.size.x / 2.f);
    newPos.y = std::min(std::max(newPos.y, gameBounds.position.y + demonBounds.size.y / 2.f), gameBounds.position.y + gameBounds.size.y - demonBounds.size.y / 2.f);
    sprite->setPosition(newPos);
}


// --- Implementações de Bishop_ALL ---

Bishop_ALL::Bishop_ALL(std::vector<sf::Texture>& walkTextures)
    : EnemyBase()
{
    const BishopConfig& cfg = ConfigManager::getInstance().getConfig().bishop;

    health = cfg.initial_health;
    speed = cfg.speed;

    frame_duration = cfg.frame_duration;
    FRAME_B7_INDEX = cfg.heal_trigger_frame;
    healCooldown = sf::seconds(cfg.heal_cooldown);

    center_pull_weight = cfg.center_pull_weight;
    lateral_bias_frequency = cfg.lateral_bias_frequency;
    lateral_bias_strength = cfg.lateral_bias_strength;

    textures_idle = &walkTextures;

    if (textures_idle && !textures_idle->empty()) {
        sprite.emplace(textures_idle->at(0));
        sprite->setScale(sf::Vector2f(cfg.scale, cfg.scale));
        sprite->setPosition({ cfg.start_position_x, cfg.start_position_y });
        sprite->setOrigin(sf::Vector2f(cfg.origin_x, cfg.origin_y));
    }
}


void Bishop_ALL::handleAnimation(float deltaTime) {
    if (!sprite) return;

    const int current_total_frames = ConfigManager::getInstance().getConfig().bishop.animation_frames;

    if (isChanting) {
        animation_time += deltaTime;

        if (animation_time >= frame_duration) {
            animation_time -= frame_duration;
            current_frame = (current_frame + 1);

            if (current_frame == FRAME_B7_INDEX) {
                if (healClock.getElapsedTime().asSeconds() >= healCooldown.asSeconds()) {
                    canHealDemon = true;
                }
            }

            if (current_frame >= current_total_frames) {
                isChanting = false;
                current_frame = 0;
                animation_time = 0.0f;
            }

            if (textures_idle && current_frame < textures_idle->size())
                sprite->setTexture(textures_idle->at(current_frame));
        }
    }
    else {
        current_frame = 0;
        animation_time = 0.0f;
        if (sprite && textures_idle && !textures_idle->empty()) {
            sprite->setTexture(textures_idle->at(0));
        }

        if (healClock.getElapsedTime().asSeconds() >= healCooldown.asSeconds() && !canHealDemon) {
            isChanting = true;
        }
    }
}

bool Bishop_ALL::shouldHealDemon() const {
    return canHealDemon;
}

void Bishop_ALL::resetHealFlag() {
    canHealDemon = false;
    healClock.restart();
}


void Bishop_ALL::update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) {
    if (health <= 0) return;

    if (!sprite) return;

    sf::Vector2f currentPos = sprite->getPosition();
    sf::Vector2f diff = playerPosition - currentPos;
    sf::Vector2f flee = -diff;
    float length = std::sqrtf(flee.x * flee.x + flee.y * flee.y);
    if (length > 0.0f) {
        flee /= length;
    }

    float center_Y = gameBounds.position.y + gameBounds.size.y / 2.f;
    sf::Vector2f centerPull = { 0.f, center_Y - currentPos.y };

    float pullLength = std::sqrtf(centerPull.x * centerPull.x + centerPull.y * centerPull.y);
    if (pullLength > 0.0f) {
        centerPull /= pullLength;
        centerPull *= center_pull_weight;
    }

    sf::Vector2f lateralMove = { -flee.y, flee.x };
    float lateralBias = std::sin(healClock.getElapsedTime().asSeconds() * lateral_bias_frequency) * lateral_bias_strength;

    sf::Vector2f finalMove = flee + (lateralMove * lateralBias) + centerPull;

    float finalLength = std::sqrtf(finalMove.x * finalMove.x + finalMove.y * finalMove.y);
    if (finalLength > 0.0f) {
        finalMove /= finalLength;
    }

    sprite->move(finalMove * deltaTime * speed);


    handleAnimation(deltaTime);

    handleHitFlash();

    sf::Vector2f newPos = sprite->getPosition();
    sf::FloatRect bounds = sprite->getGlobalBounds();

    newPos.x = std::min(
        std::max(newPos.x, gameBounds.position.x + (bounds.size.x / 2.f) - 112),
        gameBounds.position.x + gameBounds.size.x - (bounds.size.x / 2.f)
    );

    newPos.y = std::min(
        std::max(newPos.y, gameBounds.position.y + (bounds.size.y / 2.f) - 85.f),
        gameBounds.position.y + gameBounds.size.y - (bounds.size.y / 2.f) - 50.f
    );

    sprite->setPosition(newPos);
}