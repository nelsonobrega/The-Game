#include "Monstro.hpp"
#include "ConfigManager.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Monstro::Monstro(sf::Texture& texture, sf::Texture& projectileTex, sf::Vector2f startPos) : EnemyBase() {
    projTex = &projectileTex;
    initFrames();
    sprite.emplace(texture);

    if (sprite) {
        sprite->setPosition(startPos);
        sprite->setScale(sf::Vector2f(baseScale, baseScale));
    }

    setFrame(1, 3);
    health = 80.f;
    maxHealth = 80.f;
    state = MonstroState::Idle;
    stateTimer = 0.f;
    animStep = 0;
    groundPos = startPos;
    lastActionWasAttack = false;

    shadow.setFillColor(sf::Color(0, 0, 0, 100));
    shadow.setRadius(25.f);
    shadow.setScale(sf::Vector2f(3.5f, 1.2f));
    shadow.setOrigin(sf::Vector2f(25.f, 25.f));
}

void Monstro::initFrames() {
    frames[1][1] = sf::IntRect({ 6, 66 }, { 67, 46 });
    frames[1][2] = sf::IntRect({ 86, 66 }, { 67, 46 });
    frames[1][3] = sf::IntRect({ 167, 60 }, { 67, 52 });
    frames[1][4] = sf::IntRect({ 246, 49 }, { 67, 63 });
    frames[1][5] = sf::IntRect({ 928, 66 }, { 66, 76 });
    frames[2][4] = sf::IntRect({ 247, 172 }, { 67, 52 });
}

void Monstro::setFrame(int coluna, int id) {
    if (sprite) {
        sf::IntRect r = frames[coluna][id];
        sprite->setTextureRect(r);
        sprite->setOrigin(sf::Vector2f((float)r.size.x / 2.f, (float)r.size.y));
    }
}

void Monstro::handleStates(float deltaTime, sf::Vector2f playerPos) {
    float flip = (playerPos.x > groundPos.x) ? -baseScale : baseScale;

    switch (state) {
    case MonstroState::Idle: {
        setFrame(1, 3);
        sprite->setScale(sf::Vector2f(flip, baseScale));
        if (stateTimer > 1.1f) {
            stateTimer = 0.0f;
            if (health < 40.f && (rand() % 100 < 40)) { // Mega Jump abaixo de 50% HP
                state = MonstroState::MegaJumping;
                groundPosTarget = playerPos;
            }
            else {
                if (lastActionWasAttack) {
                    state = MonstroState::Walking;
                    sf::Vector2f dir = playerPos - groundPos;
                    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    moveDir = (dist > 0) ? dir / dist : sf::Vector2f(0, 0);
                    lastActionWasAttack = false;
                }
                else {
                    state = MonstroState::Attacking;
                    groundPosTarget = playerPos;
                    lastActionWasAttack = true;
                }
            }
            animStep = 0;
        }
        break;
    }

    case MonstroState::Walking: {
        if (stateTimer < 0.75f) {
            setFrame(1, 2);
            groundPos += moveDir * 280.f * deltaTime;
            float jumpProgress = stateTimer / 0.75f;
            float jumpHeight = std::sin(jumpProgress * M_PI) * 150.f;
            sprite->setScale(sf::Vector2f(flip, baseScale));
            sprite->setPosition(sf::Vector2f(groundPos.x, groundPos.y - jumpHeight));
        }
        else {
            state = MonstroState::Cooldown;
            stateTimer = 0.0f;
        }
        break;
    }

    case MonstroState::Attacking: {
        if (stateTimer < 0.45f) {
            setFrame(1, 3);
            float charge = (stateTimer / 0.45f) * 0.08f;
            sprite->setScale(sf::Vector2f(flip * (1.0f + charge), baseScale * (1.0f - charge)));
        }
        else if (stateTimer < 1.4f) {
            setFrame(1, 4);
            sprite->setScale(sf::Vector2f(flip * 0.95f, baseScale * 1.1f));
            if (animStep == 0) { spawnTears(14, false); animStep = 1; }
        }
        else {
            state = MonstroState::Cooldown;
            stateTimer = 0.0f;
        }
        break;
    }

    case MonstroState::MegaJumping: {
        setFrame(1, 1);
        if (stateTimer < 0.4f) {
            sprite->move(sf::Vector2f(0.f, -2200.f * deltaTime));
        }
        else {
            state = MonstroState::Falling;
            stateTimer = 0.0f;
        }
        break;
    }

    case MonstroState::Falling: {
        if (stateTimer < 0.8f) { // Perseguição no ar
            sf::Vector2f dir = groundPosTarget - groundPos;
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist > 5.0f) groundPos += (dir / dist) * 850.f * deltaTime;
        }
        else if (stateTimer < 1.0f) { // Queda rápida
            float dropProgress = (stateTimer - 0.8f) / 0.2f;
            sprite->setPosition(sf::Vector2f(groundPos.x, (groundPos.y - 2000.f) + (dropProgress * 2000.f)));
            setFrame(1, 2);
        }
        else {
            sprite->setPosition(groundPos);
            spawnTears(40, true);
            state = MonstroState::Cooldown;
            stateTimer = 0.0f;
        }
        break;
    }

    case MonstroState::Cooldown: {
        setFrame(2, 4);
        sprite->setPosition(groundPos);
        if (stateTimer > 0.6f) {
            state = MonstroState::Idle;
            stateTimer = 0.0f;
        }
        break;
    }
    }
}

void Monstro::update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) {
    if (health <= 0) return;
    stateTimer += deltaTime;
    handleStates(deltaTime, playerPosition);

    float maxRange = 769.f; // RANGE DE 980px

    for (auto it = projectiles.begin(); it != projectiles.end();) {
        float speed = 480.f;
        sf::Vector2f movement = it->direction * speed * deltaTime;
        it->sprite.move(movement);

        // Acumula a distância percorrida
        it->distanceTraveled += std::sqrt(movement.x * movement.x + movement.y * movement.y);

        // Remove se sair da tela OU se passar de 980px
        if (!gameBounds.contains(it->sprite.getPosition()) || it->distanceTraveled >= maxRange) {
            it = projectiles.erase(it);
        }
        else {
            ++it;
        }
    }

    handleHitFlash();
    shadow.setPosition(groundPos);
}

void Monstro::draw(sf::RenderWindow& window) {
    if (health > 0) {
        window.draw(shadow);
        if (sprite) window.draw(*sprite);
        for (auto& p : projectiles) window.draw(p.sprite);

        // Barra de vida do Boss
        float currentH = static_cast<float>(health);
        float hRatio = std::max<float>(0.0f, currentH) / maxHealth;
        sf::RectangleShape back(sf::Vector2f(600.f, 15.f));
        back.setFillColor(sf::Color(30, 30, 30));
        back.setPosition(sf::Vector2f(static_cast<float>(window.getSize().x) / 2.f - 300.f, 30.f));
        sf::RectangleShape front(sf::Vector2f(600.f * hRatio, 15.f));
        front.setFillColor(sf::Color(255, 0, 0));
        front.setPosition(back.getPosition());
        window.draw(back);
        window.draw(front);
    }
}

void Monstro::spawnTears(int count, bool circular) {
    if (!sprite || !projTex) return;
    const auto& dConfigTear = ConfigManager::getInstance().getConfig().projectile_textures.demon_tear;
    sf::IntRect demonTearRect({ dConfigTear.x, dConfigTear.y }, { dConfigTear.width, dConfigTear.height });

    if (circular) {
        for (int i = 0; i < count; ++i) {
            float angle = (360.f / count) * i;
            float rad = angle * (M_PI / 180.f);
            float speedMult = 0.7f + (static_cast<float>(rand() % 60) / 100.f);

            // Inicializando distanceTraveled como 0.f
            EnemyProjectile p = { sf::Sprite(*projTex), { std::cos(rad) * speedMult, std::sin(rad) * speedMult }, 0.f };
            p.sprite.setTextureRect(demonTearRect);
            p.sprite.setOrigin(sf::Vector2f(dConfigTear.width / 2.f, dConfigTear.height / 2.f));
            p.sprite.setPosition(sprite->getPosition());
            p.sprite.setScale(sf::Vector2f(2.0f, 2.0f));
            projectiles.push_back(p);
        }
    }
    else {
        sf::Vector2f baseDir = groundPosTarget - sprite->getPosition();
        float baseAngle = std::atan2(baseDir.y, baseDir.x) * (180.f / M_PI);

        for (int i = 0; i < count; ++i) {
            float spread = (rand() % 60 - 30);
            float rad = (baseAngle + spread) * (M_PI / 180.f);
            float speedMult = 0.8f + (static_cast<float>(rand() % 40) / 100.f);

            EnemyProjectile p = { sf::Sprite(*projTex), { std::cos(rad) * speedMult, std::sin(rad) * speedMult }, 0.f };
            p.sprite.setTextureRect(demonTearRect);
            p.sprite.setOrigin(sf::Vector2f(dConfigTear.width / 2.f, dConfigTear.height / 2.f));
            p.sprite.setPosition(sprite->getPosition() - sf::Vector2f(0.f, 40.f));
            p.sprite.setScale(sf::Vector2f(2.3f, 2.3f));
            projectiles.push_back(p);
        }
    }
}

void Monstro::setPosition(sf::Vector2f pos) {
    groundPos = pos;
    if (sprite) sprite->setPosition(pos);
}