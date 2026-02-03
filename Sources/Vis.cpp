#include "Vis.hpp"
#include <cmath>
#include <cstdlib>

const sf::Color BLOOD_RED(160, 0, 0, 245);
const sf::Color PURPLE_DARK(120, 0, 180, 245);

// Helper para configurar o laser
void setupBeam(sf::ConvexShape& cone, sf::CircleShape& end, FaceDir dir, sf::Vector2f pos, float timer, const sf::FloatRect& gb, float scale) {
    float length = 0.f;
    // Ajuste de offset central para o laser sair do meio do corpo (aprox 14px * scale)
    sf::Vector2f centerOffset = { (dir == FaceDir::Left ? -14.f : 14.f) * scale, 15.f * scale };
    sf::Vector2f beamOrigin = pos + centerOffset;

    if (dir == FaceDir::Right)      length = (gb.position.x + gb.size.x) - beamOrigin.x - (10.f * scale);
    else if (dir == FaceDir::Left)  length = beamOrigin.x - (gb.position.x + (10.f * scale));
    else if (dir == FaceDir::Down)  length = (gb.position.y + gb.size.y) - beamOrigin.y - (10.f * scale);
    else if (dir == FaceDir::Up)    length = beamOrigin.y - (gb.position.y + (10.f * scale));

    float pulse = std::sin(timer * 50.f) * 3.f;
    float sW = 2.0f;
    float eW = 50.f + pulse;

    if (dir == FaceDir::Right || dir == FaceDir::Left) {
        float s = (dir == FaceDir::Right) ? 1.f : -1.f;
        cone.setPoint(0, { 0.f, -sW / 2.f });
        cone.setPoint(1, { length * s, -eW / 2.f });
        cone.setPoint(2, { length * s, eW / 2.f });
        cone.setPoint(3, { 0.f, sW / 2.f });
        cone.setPosition(beamOrigin + sf::Vector2f(5.f * s, 0.f));
    }
    else {
        float s = (dir == FaceDir::Down) ? 1.f : -1.f;
        cone.setPoint(0, { -sW / 2.f, 0.f });
        cone.setPoint(1, { sW / 2.f, 0.f });
        cone.setPoint(2, { eW / 2.f, length * s });
        cone.setPoint(3, { -eW / 2.f, length * s });
        cone.setPosition(beamOrigin + sf::Vector2f(0.f, 5.f * s));
    }

    end.setRadius(eW / 2.f);
    end.setOrigin({ eW / 2.f, eW / 2.f });
    sf::Vector2f eP = cone.getPosition();
    if (dir == FaceDir::Right) eP.x += length;
    else if (dir == FaceDir::Left) eP.x -= length;
    else if (dir == FaceDir::Down) eP.y += length;
    else if (dir == FaceDir::Up) eP.y -= length;
    end.setPosition(eP);
}

Vis::Vis(sf::Texture& sheet) : EnemyBase() {
    health = 25;
    scaleFactor = 2.5f;
    currentState = VisState::Moving;
    stateTimer = animTimer = distanceWalked = 0.f;
    animFrame = 0;
    showBeam = false;

    sprite.emplace(sheet);
    sprite->setScale({ scaleFactor, scaleFactor });

    initRects(0);
    resetMovement();

    beamCone.setPointCount(4);
    beamCone.setFillColor(BLOOD_RED);
    beamEnd.setFillColor(BLOOD_RED);
}

void Vis::initRects(int offX) {
    framePreUniversal = { { 530 + offX, 7 }, { 28, 30 } };
    framesPreAttackDir = { sf::IntRect{{561 + offX, 8}, {30,30}}, sf::IntRect{{594 + offX, 7}, {30,31}} };
    framesPreAttackDown = { sf::IntRect{{529 + offX, 40}, {30,30}}, sf::IntRect{{560 + offX, 39}, {32,31}} };
    framesPreAttackUp = { sf::IntRect{{529 + offX, 72}, {30,30}}, sf::IntRect{{560 + offX, 71}, {32,31}} };

    // DIR (0-3)
    walkFrames[0] = { { 530 + offX, 103 }, { 28, 30 } };
    walkFrames[1] = { { 562 + offX, 103 }, { 28, 30 } };
    walkFrames[2] = { { 594 + offX, 104 }, { 28, 30 } };
    walkFrames[3] = { { 626 + offX, 103 }, { 28, 30 } };
    // ESQ (4-7)
    walkFrames[4] = { { 530 + offX, 135 }, { 28, 30 } };
    walkFrames[5] = { { 562 + offX, 135 }, { 28, 30 } };
    walkFrames[6] = { { 594 + offX, 136 }, { 28, 30 } };
    walkFrames[7] = { { 626 + offX, 135 }, { 28, 30 } };
}

void Vis::update(float dt, sf::Vector2f pPos, const sf::FloatRect& gb) {
    if (health <= 0) return;
    stateTimer += dt;

    sf::Vector2f myPos = sprite->getPosition();

    if (currentState == VisState::Moving) {
        sf::Vector2f diff = pPos - myPos;
        // Trigger de Ataque se estiver alinhado
        if (std::abs(diff.x) < 45.f || std::abs(diff.y) < 45.f) {
            attackDir = (std::abs(diff.x) < 45.f) ? (diff.y > 0 ? FaceDir::Down : FaceDir::Up) : (diff.x > 0 ? FaceDir::Right : FaceDir::Left);
            currentState = VisState::PreAttack;
            stateTimer = 0;
            lastStablePos = myPos;
        }
        else {
            // Lógica de Movimento Igual ao Chubby
            sf::Vector2f movement = moveDir * 105.f * dt;
            sf::Vector2f nextPos = myPos + movement;
            sf::Vector2f size(28.f * scaleFactor, 30.f * scaleFactor);

            bool canMove = true;
            if (nextPos.x < gb.position.x || (nextPos.x + size.x) >(gb.position.x + gb.size.x)) canMove = false;
            if (nextPos.y < gb.position.y || (nextPos.y + size.y) >(gb.position.y + gb.size.y)) canMove = false;

            if (canMove) {
                sprite->move(movement);
                distanceWalked += std::sqrt(movement.x * movement.x + movement.y * movement.y);
            }
            else {
                distanceWalked = 201.f; // Força nova direção
            }

            updateAnimation(dt);

            if (distanceWalked >= 200.f) {
                currentState = VisState::Idle;
                stateTimer = 0;
            }
        }
    }
    else if (currentState == VisState::PreAttack || currentState == VisState::Attacking) {
        if (currentState == VisState::PreAttack) {
            handleAttackSequence(dt, gb);
        }
        else {
            if (attackDir == FaceDir::Right || attackDir == FaceDir::Left) sprite->setTextureRect(framesPreAttackDir[1]);
            else if (attackDir == FaceDir::Down) sprite->setTextureRect(framesPreAttackDown[1]);
            else sprite->setTextureRect(framesPreAttackUp[1]);

            setupBeam(beamCone, beamEnd, attackDir, lastStablePos, stateTimer, gb, scaleFactor);

            if (stateTimer >= 1.3f) {
                showBeam = false;
                currentState = VisState::Cooldown;
                stateTimer = 0;
                sprite->setPosition(lastStablePos);
            }
        }
        // Shake
        float shake = (currentState == VisState::Attacking) ? 4.0f : 1.5f;
        sprite->setPosition(lastStablePos + sf::Vector2f((std::rand() % 10 - 5) / 5.f * shake, (std::rand() % 10 - 5) / 5.f * shake));
    }
    else if (currentState == VisState::Idle || currentState == VisState::Cooldown) {
        if ((currentState == VisState::Idle && stateTimer > 0.5f) || (currentState == VisState::Cooldown && stateTimer > 0.8f)) {
            resetMovement();
            currentState = VisState::Moving;
            stateTimer = 0;
        }
    }

    handleHitFlash();
}

void Vis::updateAnimation(float dt) {
    animTimer += dt;
    if (animTimer >= 0.12f) {
        animTimer = 0;
        animFrame = (animFrame + 1) % 8; // Ciclo DIR 1234 -> ESQ 1234
    }

    sprite->setTextureRect(walkFrames[animFrame]);

    // Flip Lógica Chubby
    float curScaleX = (faceDir == FaceDir::Left) ? -scaleFactor : scaleFactor;
    sprite->setScale({ curScaleX, scaleFactor });
    sprite->setOrigin({ (faceDir == FaceDir::Left ? 28.f : 0.f), 0.f });
}

void Vis::handleAttackSequence(float dt, const sf::FloatRect& gb) {
    // Manter o flip correto durante o carregamento do ataque
    float curScaleX = (attackDir == FaceDir::Left) ? -scaleFactor : scaleFactor;
    sprite->setScale({ curScaleX, scaleFactor });
    sprite->setOrigin({ (attackDir == FaceDir::Left ? 28.f : 0.f), 0.f });

    if (stateTimer < 0.3f) {
        sprite->setTextureRect(framePreUniversal);
    }
    else if (stateTimer < 0.7f) {
        if (attackDir == FaceDir::Right || attackDir == FaceDir::Left) sprite->setTextureRect(framesPreAttackDir[0]);
        else if (attackDir == FaceDir::Down) sprite->setTextureRect(framesPreAttackDown[0]);
        else sprite->setTextureRect(framesPreAttackUp[0]);
    }
    else {
        showBeam = true;
        currentState = VisState::Attacking;
        stateTimer = 0;
    }
}

void Vis::draw(sf::RenderWindow& window) {
    if (health <= 0) return;
    if (showBeam && attackDir == FaceDir::Up) { window.draw(beamCone); window.draw(beamEnd); }
    if (sprite) window.draw(*sprite);
    if (showBeam && attackDir != FaceDir::Up) { window.draw(beamCone); window.draw(beamEnd); }
}

void Vis::resetMovement() {
    distanceWalked = 0.f;
    int r = std::rand() % 4;
    faceDir = static_cast<FaceDir>(r);

    if (faceDir == FaceDir::Right)      moveDir = { 1.f, 0.f };
    else if (faceDir == FaceDir::Left) moveDir = { -1.f, 0.f };
    else if (faceDir == FaceDir::Down) moveDir = { 0.f, 1.f };
    else                               moveDir = { 0.f, -1.f };

    // Update inicial de escala/origem
    float curScaleX = (faceDir == FaceDir::Left) ? -scaleFactor : scaleFactor;
    sprite->setScale({ curScaleX, scaleFactor });
    sprite->setOrigin({ (faceDir == FaceDir::Left ? 28.f : 0.f), 0.f });
}

sf::FloatRect Vis::getGlobalBounds() const {
    if (!sprite) return {};
    sf::FloatRect b = sprite->getGlobalBounds();
    sf::Vector2f p = (currentState == VisState::Attacking || currentState == VisState::PreAttack) ? lastStablePos : sprite->getPosition();
    // Ajuste de hitbox baseado na origem dinâmica
    float offsetX = (faceDir == FaceDir::Left) ? -28.f * scaleFactor : 0.f;
    b.position = p + sf::Vector2f(offsetX, 0.f);
    return b;
}

void Vis::takeDamage(int amount) { health -= amount; isHit = true; hitClock.restart(); }
void Vis::heal(int amount) { health += amount; isHealing = true; healFlashClock.restart(); }
void Vis::setPosition(const sf::Vector2f& pos) { if (sprite) { sprite->setPosition(pos); lastStablePos = pos; } }

// --- DOUBLE VIS ---

DoubleVis::DoubleVis(sf::Texture& sheet) : Vis(sheet) {
    health = 50;
    initRects(138);
    beamConeTras.setPointCount(4);
    beamConeTras.setFillColor(PURPLE_DARK);
    beamEndTras.setFillColor(PURPLE_DARK);
    beamCone.setFillColor(PURPLE_DARK);
    beamEnd.setFillColor(PURPLE_DARK);
}

void DoubleVis::update(float dt, sf::Vector2f playerPos, const sf::FloatRect& gb) {
    Vis::update(dt, playerPos, gb);
    if (showBeam) {
        FaceDir opp = (attackDir == FaceDir::Up) ? FaceDir::Down : (attackDir == FaceDir::Down ? FaceDir::Up : (attackDir == FaceDir::Left ? FaceDir::Right : FaceDir::Left));
        setupBeam(beamConeTras, beamEndTras, opp, lastStablePos, stateTimer, gb, scaleFactor);
    }
}

void DoubleVis::draw(sf::RenderWindow& window) {
    if (health <= 0) return;
    if (showBeam) {
        window.draw(beamCone);
        window.draw(beamConeTras);
        window.draw(beamEnd);
        window.draw(beamEndTras);
    }
    if (sprite) window.draw(*sprite);
}