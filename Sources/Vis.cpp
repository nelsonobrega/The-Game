#include "Vis.hpp"
#include <cmath>
#include <random>

Vis::Vis(sf::Texture& sheet) : EnemyBase() {
    health = 25;
    scaleFactor = 2.5f;
    currentState = VisState::Moving;
    resetMovement();

    stateTimer = 0.f;
    animTimer = 0.f;
    animFrame = 0;
    showBeam = false;

    sprite.emplace(sheet);
    sprite->setScale(sf::Vector2f(scaleFactor, scaleFactor));

    // Frames
    framePreUniversal = sf::IntRect({ 530, 7 }, { 28, 30 });
    framesPreAttackDir = { sf::IntRect({ 561, 8 }, { 30, 30 }), sf::IntRect({ 594, 7 }, { 30, 31 }) };
    framesPreAttackDown = { sf::IntRect({ 529, 40 }, { 30, 30 }), sf::IntRect({ 560, 39 }, { 32, 31 }) };
    framesPreAttackUp = { sf::IntRect({ 529, 72 }, { 30, 30 }), sf::IntRect({ 560, 71 }, { 32, 31 }) };

    walkFrames[0] = sf::IntRect({ 530, 103 }, { 28, 30 }); walkFrames[1] = sf::IntRect({ 562, 103 }, { 28, 30 });
    walkFrames[2] = sf::IntRect({ 594, 104 }, { 28, 30 }); walkFrames[3] = sf::IntRect({ 626, 103 }, { 28, 30 });
    walkFrames[4] = sf::IntRect({ 530, 135 }, { 28, 30 }); walkFrames[5] = sf::IntRect({ 562, 135 }, { 28, 30 });
    walkFrames[6] = sf::IntRect({ 594, 136 }, { 28, 30 }); walkFrames[7] = sf::IntRect({ 626, 135 }, { 28, 30 });

    sprite->setTextureRect(walkFrames[0]);
    sprite->setOrigin(sf::Vector2f(14.f, 15.f));

    // Configuração visual do Laser (ConvexShape com 4 pontos = trapézio)
    beamCone.setPointCount(4);
    beamCone.setFillColor(sf::Color(255, 0, 0, 220));
    beamEnd.setFillColor(sf::Color(255, 0, 0, 220));
}

void Vis::resetMovement() {
    distanceWalked = 0.f;
    int r = std::rand() % 4;
    if (r == 0) { moveDir = { 1, 0 }; faceDir = FaceDir::Right; }
    else if (r == 1) { moveDir = { -1, 0 }; faceDir = FaceDir::Left; }
    else if (r == 2) { moveDir = { 0, 1 }; faceDir = FaceDir::Down; }
    else { moveDir = { 0, -1 }; faceDir = FaceDir::Up; }
}

void Vis::update(float deltaTime, sf::Vector2f playerPos, const sf::FloatRect& gameBounds) {
    if (health <= 0) return;
    stateTimer += deltaTime;

    sf::Vector2f myPos = sprite->getPosition();
    sf::Vector2f diff = playerPos - myPos;

    switch (currentState) {
    case VisState::Idle:
        if (stateTimer >= 0.4f) {
            currentState = VisState::Moving;
            stateTimer = 0.f;
            resetMovement();
        }
        break;

    case VisState::Moving:
        if (std::abs(diff.x) < 35.f || std::abs(diff.y) < 35.f) {
            if (std::abs(diff.x) < 35.f) attackDir = (diff.y > 0) ? FaceDir::Down : FaceDir::Up;
            else attackDir = (diff.x > 0) ? FaceDir::Right : FaceDir::Left;
            currentState = VisState::PreAttack;
            stateTimer = 0;
            break;
        }

        {
            sf::Vector2f movement = moveDir * 95.f * deltaTime;
            sf::Vector2f nextPos = myPos + movement;
            bool canMove = true;
            if (nextPos.x < gameBounds.position.x + 50.f || nextPos.x > gameBounds.position.x + gameBounds.size.x - 50.f) canMove = false;
            if (nextPos.y < gameBounds.position.y + 50.f || nextPos.y > gameBounds.position.y + gameBounds.size.y - 50.f) canMove = false;

            if (canMove) {
                sprite->move(movement);
                distanceWalked += std::sqrt(movement.x * movement.x + movement.y * movement.y);
            }
            else { distanceWalked = 201.f; }

            if (distanceWalked >= 180.f) {
                currentState = VisState::Idle;
                stateTimer = 0;
            }
        }
        updateAnimation(deltaTime);
        break;

    case VisState::PreAttack:
        handleAttackSequence(deltaTime, gameBounds);
        break;

    case VisState::Attacking:
    {
        sf::Vector2f bPos = sprite->getPosition();

        float length = 0.f;
        if (attackDir == FaceDir::Right) length = (gameBounds.position.x + gameBounds.size.x) - bPos.x - 40.f;
        else if (attackDir == FaceDir::Left) length = bPos.x - (gameBounds.position.x + 40.f);
        else if (attackDir == FaceDir::Down) length = (gameBounds.position.y + gameBounds.size.y) - bPos.y - 40.f;
        else if (attackDir == FaceDir::Up) length = bPos.y - (gameBounds.position.y + 40.f);

        // Pulsação
        float pulse = std::sin(stateTimer * 45.f) * 4.f;
        float startWidth = 8.f + pulse;   // Fino no Vis
        float endWidth = 55.f + pulse;    // Largo no fim

        // Construir trapézio com ConvexShape (4 pontos)
        // Direção horizontal (Right ou Left)
        if (attackDir == FaceDir::Right) {
            beamCone.setPoint(0, sf::Vector2f(0.f, -startWidth / 2.f));        // Topo esquerdo (fino)
            beamCone.setPoint(1, sf::Vector2f(length, -endWidth / 2.f));       // Topo direito (largo)
            beamCone.setPoint(2, sf::Vector2f(length, endWidth / 2.f));        // Base direita (largo)
            beamCone.setPoint(3, sf::Vector2f(0.f, startWidth / 2.f));         // Base esquerda (fino)
        }
        else if (attackDir == FaceDir::Left) {
            beamCone.setPoint(0, sf::Vector2f(0.f, -startWidth / 2.f));        // Topo direito (fino)
            beamCone.setPoint(1, sf::Vector2f(-length, -endWidth / 2.f));      // Topo esquerdo (largo)
            beamCone.setPoint(2, sf::Vector2f(-length, endWidth / 2.f));       // Base esquerda (largo)
            beamCone.setPoint(3, sf::Vector2f(0.f, startWidth / 2.f));         // Base direita (fino)
        }
        // Direção vertical (Down ou Up)
        else if (attackDir == FaceDir::Down) {
            beamCone.setPoint(0, sf::Vector2f(-startWidth / 2.f, 0.f));        // Esquerda topo (fino)
            beamCone.setPoint(1, sf::Vector2f(startWidth / 2.f, 0.f));         // Direita topo (fino)
            beamCone.setPoint(2, sf::Vector2f(endWidth / 2.f, length));        // Direita base (largo)
            beamCone.setPoint(3, sf::Vector2f(-endWidth / 2.f, length));       // Esquerda base (largo)
        }
        else if (attackDir == FaceDir::Up) {
            beamCone.setPoint(0, sf::Vector2f(-startWidth / 2.f, 0.f));        // Esquerda base (fino)
            beamCone.setPoint(1, sf::Vector2f(startWidth / 2.f, 0.f));         // Direita base (fino)
            beamCone.setPoint(2, sf::Vector2f(endWidth / 2.f, -length));       // Direita topo (largo)
            beamCone.setPoint(3, sf::Vector2f(-endWidth / 2.f, -length));      // Esquerda topo (largo)
        }

        // Ajustar posição do cone com offset para cada direção
        sf::Vector2f conePos = bPos;
        if (attackDir == FaceDir::Right) {
            conePos.x += 9.f;  // Mais para frente
            conePos.y += 2.f;  // Um pouco para baixo
        }
        else if (attackDir == FaceDir::Left) {
            conePos.x -= 9.f;  // Mais para frente
            conePos.y += 2.f;  // Um pouco para baixo
        }
        else if (attackDir == FaceDir::Down) {
            conePos.x += 5.f;  // Mais para a direita
            conePos.y += 8.f;  // Mais para frente
        }
        else if (attackDir == FaceDir::Up) {
            conePos.x += 5.f;  // Mais para a direita
            conePos.y -= 8.f;  // Mais para frente
        }

        beamCone.setPosition(conePos);

        // Círculo no final
        beamEnd.setRadius(endWidth / 2.f);
        beamEnd.setOrigin({ endWidth / 2.f, endWidth / 2.f });

        sf::Vector2f endPos = conePos;
        if (attackDir == FaceDir::Right) endPos.x += length;
        else if (attackDir == FaceDir::Left) endPos.x -= length;
        else if (attackDir == FaceDir::Down) endPos.y += length;
        else if (attackDir == FaceDir::Up) endPos.y -= length;

        beamEnd.setPosition(endPos);

        if (stateTimer >= 1.4f) {
            showBeam = false;
            currentState = VisState::Cooldown;
            stateTimer = 0;
        }
    }
    break;

    case VisState::Cooldown:
        if (stateTimer >= 0.7f) {
            currentState = VisState::Moving;
            stateTimer = 0;
            resetMovement();
        }
        break;
    }
    handleHitFlash();
    handleHealFlash();
}

void Vis::handleAttackSequence(float deltaTime, const sf::FloatRect& gameBounds) {
    if (stateTimer < 0.25f) {
        sprite->setTextureRect(framePreUniversal);
    }
    else if (stateTimer < 0.75f) {
        if (attackDir == FaceDir::Right || attackDir == FaceDir::Left) sprite->setTextureRect(framesPreAttackDir[0]);
        else if (attackDir == FaceDir::Down) sprite->setTextureRect(framesPreAttackDown[0]);
        else sprite->setTextureRect(framesPreAttackUp[0]);

        float sX = (attackDir == FaceDir::Left) ? -scaleFactor : scaleFactor;
        sprite->setScale({ sX, scaleFactor });
    }
    else {
        if (attackDir == FaceDir::Right || attackDir == FaceDir::Left) sprite->setTextureRect(framesPreAttackDir[1]);
        else if (attackDir == FaceDir::Down) sprite->setTextureRect(framesPreAttackDown[1]);
        else if (attackDir == FaceDir::Up) sprite->setTextureRect(framesPreAttackUp[1]);

        showBeam = true;
        currentState = VisState::Attacking;
        stateTimer = 0;
    }
}

void Vis::updateAnimation(float dt) {
    animTimer += dt;
    if (animTimer >= 0.12f) { animTimer = 0; animFrame = (animFrame + 1) % 8; }
    sprite->setTextureRect(walkFrames[animFrame]);
    float sX = (animFrame >= 4) ? -scaleFactor : scaleFactor;
    sprite->setScale({ sX, scaleFactor });
}

void Vis::draw(sf::RenderWindow& window) {
    if (health <= 0) return;

    // Z-Index: laser para cima vai atrás
    if (showBeam && attackDir == FaceDir::Up) {
        window.draw(beamCone);
        window.draw(beamEnd);
    }

    if (sprite) window.draw(*sprite);

    if (showBeam && attackDir != FaceDir::Up) {
        window.draw(beamCone);
        window.draw(beamEnd);
    }
}

void Vis::takeDamage(int amount) { health -= amount; isHit = true; hitClock.restart(); }
void Vis::heal(int amount) { health += amount; isHealing = true; healFlashClock.restart(); }
void Vis::setPosition(const sf::Vector2f& pos) { if (sprite) sprite->setPosition(pos); }
sf::FloatRect Vis::getGlobalBounds() const { return sprite ? sprite->getGlobalBounds() : sf::FloatRect(); }
sf::FloatRect Vis::getLaserBounds() const { return isLaserActive() ? beamCone.getGlobalBounds() : sf::FloatRect(); }