#include "player.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "ConfigManager.hpp" 
#include <algorithm>
#include <iostream>
#include <cmath>
#include <SFML/Graphics/RenderWindow.hpp>

static bool checkCollision(const sf::FloatRect& rect1, const sf::FloatRect& rect2) {
    bool x_overlap = rect1.position.x < rect2.position.x + rect2.size.x &&
        rect1.position.x + rect1.size.x > rect2.position.x;
    bool y_overlap = rect1.position.y < rect2.position.y + rect2.size.y &&
        rect1.position.y + rect1.size.y > rect2.position.y;
    return x_overlap && y_overlap;
}

// --- MÉTODOS DE STATUS E ITENS ---

void Player_ALL::addSpeed(float amount) {
    speed *= amount;
}

void Player_ALL::addDamage(float amount) {
    damage += amount;
}

void Player_ALL::setEightInchNail(bool active) {
    hasEightInchNail = active;
}

void Player_ALL::incrementNailHits() {
    nailHits++;
    if (nailHits == 100) std::cout << "BLOOD NAIL UNLOCKED!" << std::endl;
}

void Player_ALL::rage() {
    speed *= 1.5f;
    damage += 1.0f;
}

void Player_ALL::increaseMaxHealth(int containers) {
    maxHealthBonus += (containers * 2);
}

void Player_ALL::heal(int amount) {
    const auto& config = ConfigManager::getInstance().getConfig();
    int currentLimit = (config.game.ui.max_hearts * 2) + maxHealthBonus;
    health = std::min(currentLimit, health + amount);
}

void Player_ALL::setTearTexture(const sf::Texture& texture, const sf::IntRect& rect) {
    hitTexture = const_cast<sf::Texture*>(&texture);
    projectileTextureRect = rect;
}

// --- CONSTRUTOR ---

Player_ALL::Player_ALL(
    std::vector<sf::Texture>& walkDownTextures,
    sf::Texture& hitTextureRef,
    std::vector<sf::Texture>& walkUpTextures,
    std::vector<sf::Texture>& walkLeftTextures,
    std::vector<sf::Texture>& walkRightTextures)
    : speedMultiplier_(1.0f), maxHealthBonus(0), hasEightInchNail(false), isCharging(false), chargeTimer(0.0f), nailHits(0)
{
    const auto& config = ConfigManager::getInstance().getConfig();
    health = config.player.stats.initial_health;
    speed = config.player.stats.speed;
    damage = config.player.stats.damage;

    isaacHitSpeed = config.player.attack.projectile_speed;
    maxHitDistance = config.player.attack.projectile_max_distance;
    cooldownTime = sf::seconds(config.player.attack.cooldown);
    minDamageInterval = sf::seconds(1.0f);

    textures_walk_down = &walkDownTextures;
    textures_walk_up = &walkUpTextures;
    textures_walk_left = &walkLeftTextures;
    textures_walk_right = &walkRightTextures;
    hitTexture = &hitTextureRef;
    last_animation_set = textures_walk_down;

    // Inicializar Rects dos Pregos
    nailRects = {
        {{12,12}, {9,7}},   {{44,11}, {11,9}},  {{76,10}, {13,12}}, {{107,10}, {15,12}},
        {{137,10}, {17,12}}, {{168,10}, {19,12}}, {{199,9}, {20,14}},  {{230,9}, {22,14}},
        {{5,41}, {24,14}},   {{36,40}, {25,15}}, {{66,40}, {27,15}}, {{97,40}, {29,15}}, {{128,39}, {32,17}}
    };

    bloodNailRects = {
        {{12,76}, {9,7}},   {{44,75}, {11,9}},  {{76,74}, {13,12}}, {{107,74}, {15,12}},
        {{137,74}, {17,12}}, {{168,74}, {19,12}}, {{199,73}, {20,14}}, {{230,73}, {22,14}},
        {{5,105}, {24,14}},  {{36,104}, {25,15}}, {{66,104}, {27,15}}, {{97,104}, {29,15}}, {{128,103}, {32,17}}
    };

    if (textures_walk_down && !textures_walk_down->empty()) {
        Isaac.emplace(textures_walk_down->at(0));
        Isaac->setScale({ config.player.visual.scale, config.player.visual.scale });
        Isaac->setOrigin({ config.player.visual.origin_x, config.player.visual.origin_y });
        Isaac->setPosition({ config.player.spawn.start_position_x, config.player.spawn.start_position_y });
    }
}

void Player_ALL::handleAttack(float deltaTime) {
    if (!Isaac || !hitTexture) return;

    sf::Vector2f dir(0.f, 0.f);
    float rot = 0.f;
    bool isPressingAttack = false;

    // Mapeamento correto baseado na sprite virada para a direita
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) { dir = { 1.f, 0.f }; rot = 0.f; isPressingAttack = true; }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) { dir = { 0.f, 1.f }; rot = 90.f; isPressingAttack = true; }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) { dir = { -1.f, 0.f }; rot = 180.f; isPressingAttack = true; }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) { dir = { 0.f, -1.f }; rot = 270.f; isPressingAttack = true; }

    if (hasEightInchNail) {
        if (isPressingAttack) {
            isCharging = true;
            lastChargeDir = dir;
            lastChargeRot = rot;
            chargeTimer += deltaTime;
            currentChargeLevel = std::min((int)(chargeTimer * 10.0f), 12);
        }
        else if (isCharging) {
            Projectile p = { sf::Sprite(*hitTexture), lastChargeDir, 0.f, 0.f, true };

            float chargeMultiplier = 1.0f + (currentChargeLevel * 0.2f);
            p.damage = this->damage * chargeMultiplier;
            if (nailHits >= 100) p.damage *= 1.5f;

            sf::IntRect rect = (nailHits >= 100) ? bloodNailRects[currentChargeLevel] : nailRects[currentChargeLevel];
            p.sprite.setTextureRect(rect);

            // Escala dos pregos (podes ajustar o 2.0f se ainda estiver grande)
            p.sprite.setScale({ 2.0f, 2.0f });

            sf::FloatRect lb = p.sprite.getLocalBounds();
            p.sprite.setOrigin({ lb.size.x / 2.f, lb.size.y / 2.f });
            p.sprite.setPosition(Isaac->getPosition());
            p.sprite.setRotation(sf::degrees(lastChargeRot));

            projectiles.push_back(p);

            isCharging = false;
            chargeTimer = 0.0f;
            currentChargeLevel = 0;
        }
    }
    else if (isPressingAttack && cooldownClock.getElapsedTime() >= cooldownTime) {
        Projectile p = { sf::Sprite(*hitTexture), dir, 0.f, this->damage, false };
        p.sprite.setTextureRect(projectileTextureRect);

        // ESCALA NORMAL (Configurada via JSON ou um valor menor como 1.0f)
        const auto& pVis = ConfigManager::getInstance().getConfig().player.projectile_visual;
        p.sprite.setScale({ pVis.scale, pVis.scale });

        sf::FloatRect lb = p.sprite.getLocalBounds();
        p.sprite.setOrigin({ lb.size.x / 2.f, lb.size.y / 2.f });
        p.sprite.setPosition(Isaac->getPosition());
        p.sprite.setRotation(sf::degrees(rot));

        projectiles.push_back(p);
        cooldownClock.restart();
    }
}

void Player_ALL::updateProjectiles(float deltaTime, const sf::FloatRect& gameBounds) {
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->sprite.move(it->direction * isaacHitSpeed * deltaTime);
        it->distanceTraveled += isaacHitSpeed * deltaTime;

        if (it->distanceTraveled >= maxHitDistance || !checkCollision(it->sprite.getGlobalBounds(), gameBounds))
            it = projectiles.erase(it);
        else
            ++it;
    }
}

void Player_ALL::update(float deltaTime, const sf::FloatRect& gameBounds) {
    if (!Isaac || health <= 0) return;

    handleMovementAndAnimation(deltaTime);
    handleAttack(deltaTime);
    handleHitFlash(deltaTime);
    updateProjectiles(deltaTime, gameBounds);

    // Clamp position
    sf::Vector2f pos = Isaac->getPosition();
    sf::FloatRect bounds = Isaac->getGlobalBounds();
    pos.x = std::clamp(pos.x, gameBounds.position.x + bounds.size.x / 2.f, gameBounds.position.x + gameBounds.size.x - bounds.size.x / 2.f);
    pos.y = std::clamp(pos.y, gameBounds.position.y + bounds.size.y / 2.f, gameBounds.position.y + gameBounds.size.y - bounds.size.y / 2.f);
    Isaac->setPosition(pos);
}

void Player_ALL::draw(sf::RenderWindow& window) {

    //Desenha o nail
    if (isCharging && hasEightInchNail) {
        sf::Sprite preview(*hitTexture);
        sf::IntRect rect = (nailHits >= 100) ? bloodNailRects[currentChargeLevel] : nailRects[currentChargeLevel];
        preview.setTextureRect(rect);
        preview.setRotation(sf::degrees(lastChargeRot));
        preview.setScale({ 2.0f, 2.0f });
        preview.setPosition(Isaac->getPosition());
        preview.setColor(sf::Color(255, 255, 255, 180));
        window.draw(preview);
    }

    //Desenha os projéteis
    for (auto& p : projectiles) {
        window.draw(p.sprite);
    }
    if (!Isaac || health <= 0) return;

    //Desenha o Isaac
    window.draw(*Isaac);
}

// --- RESTO DOS MÉTODOS (Getters/Setters/Animação) ---

void Player_ALL::takeDamage(int amount) {
    if (isHit && hitClock.getElapsedTime() < minDamageInterval) return;
    if (health > 0) {
        health = std::max(0, health - amount);
        isHit = true;
        hitClock.restart();
    }
}

void Player_ALL::handleMovementAndAnimation(float deltaTime) {
    if (!Isaac) return;
    sf::Vector2f move(0.f, 0.f);
    bool is_moving = false;
    std::vector<sf::Texture>* current_animation_set = nullptr;
    const auto& animConfig = ConfigManager::getInstance().getConfig().player.visual.animation;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) move += {0.f, 1.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) move += {0.f, -1.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) move += {-1.f, 0.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) move += {1.f, 0.f};

    if (move.y > 0) { current_animation_set = textures_walk_down; is_moving = true; }
    else if (move.y < 0) { current_animation_set = textures_walk_up; is_moving = true; }
    else if (move.x < 0) { current_animation_set = textures_walk_left; is_moving = true; }
    else if (move.x > 0) { current_animation_set = textures_walk_right; is_moving = true; }

    float length = std::sqrt(move.x * move.x + move.y * move.y);
    if (length > 1.0f) move /= length;
    if (speedMultiplier_ > 0.f) Isaac->move(move * deltaTime * speed * speedMultiplier_);

    if (is_moving && current_animation_set) {
        if (current_animation_set != last_animation_set) {
            current_frame = 0; animation_time = 0.0f; last_animation_set = current_animation_set;
        }
        animation_time += deltaTime;
        if (animation_time >= animConfig.frame_duration) {
            animation_time = 0;
            int total = (move.x != 0) ? animConfig.frames_horizontal : animConfig.frames_vertical;
            current_frame = (current_frame + 1) % total;
            if (current_frame < (int)current_animation_set->size())
                Isaac->setTexture(current_animation_set->at(current_frame));
        }
    }
    else if (last_animation_set && !last_animation_set->empty()) {
        Isaac->setTexture(last_animation_set->at(0));
    }
}

void Player_ALL::handleHitFlash(float deltaTime) {
    if (!Isaac || !isHit) return;
    if (hitClock.getElapsedTime() < minDamageInterval) {
        if (static_cast<int>(hitClock.getElapsedTime().asMilliseconds() / 100) % 2 == 0)
            Isaac->setColor(sf::Color::Red);
        else
            Isaac->setColor(sf::Color(255, 255, 255, 150));
    }
    else {
        Isaac->setColor(sf::Color::White);
        isHit = false;
    }
}

// Getters padrão
float Player_ALL::getDamage() const { return damage; }
sf::Vector2f Player_ALL::getPosition() const { return Isaac ? Isaac->getPosition() : sf::Vector2f(0.f, 0.f); }
int Player_ALL::getHealth() const { return health; }
int Player_ALL::getMaxHealthBonus() const { return maxHealthBonus; }
std::vector<Projectile>& Player_ALL::getProjectiles() { return projectiles; }
sf::FloatRect Player_ALL::getGlobalBounds() const { return Isaac ? Isaac->getGlobalBounds() : sf::FloatRect(); }
void Player_ALL::setPosition(const sf::Vector2f& newPos) { if (Isaac) Isaac->setPosition(newPos); }
void Player_ALL::setSpeedMultiplier(float m) { speedMultiplier_ = std::max(0.0f, m); }
void Player_ALL::setProjectileTextureRect(const sf::IntRect& r) { projectileTextureRect = r; }