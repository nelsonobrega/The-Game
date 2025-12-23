#include "Chubby.hpp"
#include <cmath>
#include <random>
#include <iostream>

Chubby::Chubby(sf::Texture& sheet, sf::Texture& projSheet) : EnemyBase() {
    scaleFactor = 2.5f;

    sprite.emplace(sheet);
    sprite->setTextureRect({ {242, 24}, {28, 32} });
    sprite->setScale({ scaleFactor, scaleFactor });

    projectileSprite.emplace(projSheet);
    projectileSprite->setScale({ scaleFactor, scaleFactor });
    projectileSprite->setOrigin({ 12.f, 9.f });
    projectileSprite->setPosition({ -9999.f, -9999.f });
    projectileSprite->setTextureRect({ {458, 12}, {24, 18} });

    health = 15;
    state = ChubbyState::Idle;
    faceDir = FaceDir::Down;
    stateTimer = 0.f;
    animTimer = 0.f;
    animFrame = 0;
    boomerangActive = false;
    distanceWalked = 0.f;
}

// Implementação do Dano
void Chubby::takeDamage(int amount) {
    if (health > 0) {
        health -= amount;
        isHit = true;
        hitClock.restart();
        if (sprite) sprite->setColor(sf::Color::Red);
        std::cout << "Chubby levou dano! Vida atual: " << health << std::endl;
    }
}

// Implementação da Cura (Corrigida com healFlashClock)
void Chubby::heal(int amount) {
    if (health > 0) {
        health += amount;
        // Opcional: health = std::min(health, maxHealth);

        isHealing = true;             // Variável da EnemyBase
        healFlashClock.restart();     // Nome sincronizado com Enemy.hpp

        if (sprite) sprite->setColor(sf::Color::Green);
        std::cout << "Chubby curado! Vida atual: " << health << std::endl;
    }
}

void Chubby::setPosition(const sf::Vector2f& pos) {
    if (sprite) sprite->setPosition(pos);
}

sf::FloatRect Chubby::getGlobalBounds() const {
    return sprite ? sprite->getGlobalBounds() : sf::FloatRect();
}

sf::FloatRect Chubby::getBoomerangBounds() const {
    if (boomerangActive && projectileSprite) {
        return projectileSprite->getGlobalBounds();
    }
    return sf::FloatRect({ 0, 0 }, { 0, 0 });
}

void Chubby::draw(sf::RenderWindow& window) {
    if (sprite && health > 0) {
        window.draw(*sprite);
    }

    if (boomerangActive && projectileSprite) {
        window.draw(*projectileSprite);
    }
}

void Chubby::update(float deltaTime, sf::Vector2f playerPos, const sf::FloatRect& gameBounds) {
    if (health <= 0) return;

    stateTimer += deltaTime;

    if (boomerangActive) {
        updateBoomerang(deltaTime);
    }

    sf::Vector2f myPos = sprite->getPosition();
    sf::Vector2f myCenter = myPos;
    myCenter.x += (faceDir == FaceDir::Left ? -14.f : 14.f) * scaleFactor;
    myCenter.y += 16.f * scaleFactor;

    sf::Vector2f diff = playerPos - myCenter;
    float distToPlayer = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    switch (state) {
    case ChubbyState::Idle:
        sprite->setTextureRect({ {242, 24}, {28, 32} });
        if (stateTimer >= 0.4f) {
            state = ChubbyState::Moving;
            stateTimer = 0;
            distanceWalked = 0.f;
            int r = rand() % 4;
            if (r == 0) { moveDir = { 1, 0 }; faceDir = FaceDir::Right; }
            else if (r == 1) { moveDir = { -1, 0 }; faceDir = FaceDir::Left; }
            else if (r == 2) { moveDir = { 0, 1 }; faceDir = FaceDir::Down; }
            else { moveDir = { 0, -1 }; faceDir = FaceDir::Up; }
        }
        break;

    case ChubbyState::Moving:
        if (!boomerangActive && distToPlayer < 300.f && (std::abs(diff.x) < 150.f || std::abs(diff.y) < 150.f)) {
            if (std::abs(diff.x) > std::abs(diff.y)) faceDir = (diff.x > 0) ? FaceDir::Right : FaceDir::Left;
            else faceDir = (diff.y > 0) ? FaceDir::Down : FaceDir::Up;
            state = ChubbyState::Attacking;
            stateTimer = 0;
            break;
        }

        {
            sf::Vector2f movement = moveDir * 110.f * deltaTime;
            sf::Vector2f nextPos = myPos + movement;
            float spriteW = 28.f * scaleFactor;
            float spriteH = 32.f * scaleFactor;

            bool canMove = true;
            if (nextPos.x < gameBounds.position.x || (nextPos.x + spriteW) >(gameBounds.position.x + gameBounds.size.x)) canMove = false;
            if (nextPos.y < gameBounds.position.y || (nextPos.y + spriteH) >(gameBounds.position.y + gameBounds.size.y)) canMove = false;

            if (canMove) {
                sprite->move(movement);
                distanceWalked += std::sqrt(movement.x * movement.x + movement.y * movement.y);
            }
            else { distanceWalked = 201.f; }

            if (distanceWalked >= 200.f) {
                state = ChubbyState::Idle;
                stateTimer = 0;
            }
        }
        updateAnimation(deltaTime);
        break;

    case ChubbyState::Attacking:
        handleAttackSequence(deltaTime, playerPos);
        break;

    case ChubbyState::Recovering:
        if (faceDir == FaceDir::Right || faceDir == FaceDir::Left) sprite->setTextureRect({ {306, 26}, {30, 31} });
        else if (faceDir == FaceDir::Down) sprite->setTextureRect({ {272, 58}, {32, 31} });
        else if (faceDir == FaceDir::Up)   sprite->setTextureRect({ {272, 90}, {32, 31} });

        if (stateTimer >= 0.25f) {
            state = ChubbyState::Idle;
            stateTimer = 0;
        }
        break;
    }

    // Processamento vital dos flashes (Herdado da Base)
    handleHitFlash();
    handleHealFlash();
}

void Chubby::launchBoomerang() {
    boomerangActive = true;
    boomerangReturn = false;

    sf::Vector2f spawnPos = sprite->getPosition();
    float visualCenterX = (faceDir == FaceDir::Left) ? -5.f * scaleFactor : 14.f * scaleFactor;
    if (faceDir == FaceDir::Up || faceDir == FaceDir::Down) visualCenterX += 10.f;

    spawnPos.x += visualCenterX;
    spawnPos.y += 18.f * scaleFactor;
    projectilePos = spawnPos;

    float s = 1000.f;
    projectileSprite->setScale({ (faceDir == FaceDir::Left ? -scaleFactor : scaleFactor), scaleFactor });

    if (faceDir == FaceDir::Right || faceDir == FaceDir::Left)
        projectileSprite->setTextureRect({ {458, 12}, {24, 18} });
    else if (faceDir == FaceDir::Down)
        projectileSprite->setTextureRect({ {397, 9}, {18, 24} });
    else if (faceDir == FaceDir::Up)
        projectileSprite->setTextureRect({ {397, 41}, {18, 24} });

    if (faceDir == FaceDir::Right) projectileVel = { s, 0 };
    else if (faceDir == FaceDir::Left) projectileVel = { -s, 0 };
    else if (faceDir == FaceDir::Up) projectileVel = { 0, -s };
    else projectileVel = { 0, s };

    projectileSprite->setPosition(projectilePos);
}

void Chubby::updateBoomerang(float deltaTime) {
    if (!boomerangReturn) {
        projectilePos += projectileVel * deltaTime;
        float speed = std::sqrt(projectileVel.x * projectileVel.x + projectileVel.y * projectileVel.y);
        if (speed < 150.f) boomerangReturn = true;
        else projectileVel -= (projectileVel * 2.2f * deltaTime);
    }
    else {
        sf::Vector2f target = sprite->getPosition();
        float visualCenterX = (faceDir == FaceDir::Left) ? -5.f * scaleFactor : 14.f * scaleFactor;
        if (faceDir == FaceDir::Up || faceDir == FaceDir::Down) visualCenterX += 10.f;
        target.x += visualCenterX;
        target.y += 18.f * scaleFactor;

        sf::Vector2f dir = target - projectilePos;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (dist < 65.f) {
            projectilePos = target;
            boomerangActive = false;
            projectileSprite->setPosition({ -9999.f, -9999.f });
            state = ChubbyState::Recovering;
            stateTimer = 0;
            return;
        }

        projectileVel = (dir / dist) * 1100.f;
        projectilePos += projectileVel * deltaTime;

        if (faceDir == FaceDir::Right || faceDir == FaceDir::Left)
            projectileSprite->setTextureRect({ {489, 13}, {27, 17} });
        else if (faceDir == FaceDir::Down)
            projectileSprite->setTextureRect({ {428, 10}, {20, 22} });
        else if (faceDir == FaceDir::Up)
            projectileSprite->setTextureRect({ {429, 42}, {19, 21} });
    }
    projectileSprite->setPosition(projectilePos);
}

void Chubby::handleAttackSequence(float deltaTime, sf::Vector2f playerPos) {
    float curScaleX = (faceDir == FaceDir::Left) ? -scaleFactor : scaleFactor;
    sprite->setScale({ curScaleX, scaleFactor });
    sprite->setOrigin({ (faceDir == FaceDir::Left ? 28.f : 0.f), 0.f });

    if (stateTimer < 0.1f) sprite->setTextureRect({ {242, 24}, {28, 32} });
    else if (stateTimer < 0.2f) {
        if (faceDir == FaceDir::Right || faceDir == FaceDir::Left) sprite->setTextureRect({ {273, 24}, {30, 33} });
        else if (faceDir == FaceDir::Down) sprite->setTextureRect({ {241, 56}, {30, 33} });
        else if (faceDir == FaceDir::Up)   sprite->setTextureRect({ {241, 90}, {30, 33} });
    }
    else if (stateTimer < 0.35f) {
        if (faceDir == FaceDir::Right || faceDir == FaceDir::Left) sprite->setTextureRect({ {306, 26}, {30, 31} });
        else if (faceDir == FaceDir::Down) sprite->setTextureRect({ {272, 58}, {32, 31} });
        else if (faceDir == FaceDir::Up)   sprite->setTextureRect({ {272, 90}, {32, 31} });
    }
    else {
        if (!boomerangActive) launchBoomerang();
    }
}

void Chubby::updateAnimation(float deltaTime) {
    animTimer += deltaTime;
    if (animTimer >= 0.12f) {
        animTimer = 0;
        animFrame = (animFrame + 1) % 8;
    }
    int row = (animFrame < 4) ? 125 : 157;
    int col = (animFrame % 4) * 32 + 242;
    sprite->setTextureRect({ {col, row}, {28, 32} });

    float curScaleX = (faceDir == FaceDir::Left) ? -scaleFactor : scaleFactor;
    sprite->setScale({ curScaleX, scaleFactor });
    sprite->setOrigin({ (faceDir == FaceDir::Left ? 28.f : 0.f), 0.f });
}