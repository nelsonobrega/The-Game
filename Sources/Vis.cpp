#include "Vis.hpp"
#include <cmath>
#include <cstdlib>

// Configurações Visuais do Laser
const sf::Color BLOOD_RED(160, 0, 0, 245);
const sf::Color PURPLE_DARK(120, 0, 180, 245);

void setupBeam(sf::ConvexShape& cone, sf::CircleShape& end, FaceDir dir, sf::Vector2f pos, float timer, const sf::FloatRect& gb, float scale) {
    float length = 0.f;
    if (dir == FaceDir::Right)      length = (gb.position.x + gb.size.x) - pos.x - (20.f * scale);
    else if (dir == FaceDir::Left)  length = pos.x - (gb.position.x + (20.f * scale));
    else if (dir == FaceDir::Down)  length = (gb.position.y + gb.size.y) - pos.y - (20.f * scale);
    else if (dir == FaceDir::Up)    length = pos.y - (gb.position.y + (20.f * scale));

    float pulse = std::sin(timer * 50.f) * 3.f;
    float sW = 1.0f;
    float eW = 55.f + pulse;

    if (dir == FaceDir::Right || dir == FaceDir::Left) {
        float s = (dir == FaceDir::Right) ? 1.f : -1.f;
        cone.setPoint(0, { 0.f, -sW / 2.f });
        cone.setPoint(1, { length * s, -eW / 2.f });
        cone.setPoint(2, { length * s, eW / 2.f });
        cone.setPoint(3, { 0.f, sW / 2.f });
        cone.setPosition(pos + sf::Vector2f({ 9.f * s, 2.f }));
    }
    else {
        float s = (dir == FaceDir::Down) ? 1.f : -1.f;
        cone.setPoint(0, { -sW / 2.f, 0.f });
        cone.setPoint(1, { sW / 2.f, 0.f });
        cone.setPoint(2, { eW / 2.f, length * s });
        cone.setPoint(3, { -eW / 2.f, length * s });
        cone.setPosition(pos + sf::Vector2f({ 5.f, 8.f * s }));
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

// --- VIS (NORMAL) ---

Vis::Vis(sf::Texture& sheet) : EnemyBase() {
    health = 25;
    scaleFactor = 2.5f;
    currentState = VisState::Moving;
    stateTimer = animTimer = 0.f;
    animFrame = 0;
    showBeam = false;

    sprite.emplace(sheet);
    sprite->setScale({ scaleFactor, scaleFactor });
    sprite->setOrigin({ 14.f, 15.f });
    sprite->setPosition({ -9999.f, -9999.f });

    initRects(0);
    sprite->setTextureRect(walkFrames[0]);
    resetMovement();

    beamCone.setPointCount(4);
    beamCone.setFillColor(BLOOD_RED);
    beamEnd.setFillColor(BLOOD_RED);
}

void Vis::initRects(int offX) {
    // PRE-ATTACK & ATTACK (Conforme as tuas coordenadas)
    framePreUniversal = { { 530 + offX, 7 }, { 28, 30 } };
    framesPreAttackDir[0] = { { 561 + offX, 8 }, { 30, 30 } };
    framesPreAttackDir[1] = { { 594 + offX, 7 }, { 30, 31 } };
    framesPreAttackDown[0] = { { 529 + offX, 40 }, { 30, 30 } };
    framesPreAttackDown[1] = { { 560 + offX, 39 }, { 32, 31 } };
    framesPreAttackUp[0] = { { 529 + offX, 72 }, { 30, 30 } };
    framesPreAttackUp[1] = { { 560 + offX, 71 }, { 32, 31 } };

    // WALK DIR (Y=103) - 4 Frames
    walkFrames[0] = { { 530 + offX, 103 }, { 28, 30 } };
    walkFrames[1] = { { 562 + offX, 103 }, { 28, 30 } };
    walkFrames[2] = { { 594 + offX, 104 }, { 28, 30 } };
    walkFrames[3] = { { 626 + offX, 103 }, { 28, 30 } };

    // WALK ESQ (Y=135) - 4 Frames
    walkFrames[4] = { { 530 + offX, 135 }, { 28, 30 } };
    walkFrames[5] = { { 562 + offX, 135 }, { 28, 30 } };
    walkFrames[6] = { { 594 + offX, 136 }, { 28, 30 } };
    walkFrames[7] = { { 626 + offX, 135 }, { 28, 30 } };
}

void Vis::update(float dt, sf::Vector2f pPos, const sf::FloatRect& gb) {
    if (health <= 0) return;
    stateTimer += dt;

    sf::Vector2f myPos = (currentState == VisState::PreAttack || currentState == VisState::Attacking) ? lastStablePos : sprite->getPosition();

    // Segurança de Bounds (Usando position e size)
    if (myPos.x < gb.position.x + 20.f) myPos.x = gb.position.x + 20.f;
    if (myPos.x > gb.position.x + gb.size.x - 20.f) myPos.x = gb.position.x + gb.size.x - 20.f;
    if (myPos.y < gb.position.y + 20.f) myPos.y = gb.position.y + 20.f;
    if (myPos.y > gb.position.y + gb.size.y - 20.f) myPos.y = gb.position.y + gb.size.y - 20.f;

    if (currentState == VisState::Moving) {
        sf::Vector2f diff = pPos - myPos;
        if (std::abs(diff.x) < 35.f || std::abs(diff.y) < 35.f) {
            attackDir = (std::abs(diff.x) < 35.f) ? (diff.y > 0 ? FaceDir::Down : FaceDir::Up) : (diff.x > 0 ? FaceDir::Right : FaceDir::Left);
            currentState = VisState::PreAttack;
            stateTimer = 0;
            lastStablePos = myPos;
        }
        else {
            sprite->move(moveDir * 95.f * dt);
            updateAnimation(dt);
            distanceWalked += 95.f * dt;
            if (distanceWalked > 180.f) { currentState = VisState::Idle; stateTimer = 0; }
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
            if (stateTimer >= 1.4f) { showBeam = false; currentState = VisState::Cooldown; stateTimer = 0; sprite->setPosition(lastStablePos); }
        }
        // SHAKE
        float shakeAmt = (currentState == VisState::Attacking) ? 3.5f : (stateTimer > 0.4f ? 2.5f : 1.0f);
        float ox = ((std::rand() % 100) / 100.f) * shakeAmt - (shakeAmt / 2.f);
        float oy = ((std::rand() % 100) / 100.f) * shakeAmt - (shakeAmt / 2.f);
        sprite->setPosition(lastStablePos + sf::Vector2f(ox, oy));
    }
    else if (stateTimer > 0.7f) {
        currentState = VisState::Moving;
        stateTimer = 0;
        resetMovement();
    }
    handleHitFlash();
}

void Vis::handleAttackSequence(float dt, const sf::FloatRect& gb) {
    if (stateTimer < 0.3f) {
        sprite->setTextureRect(framePreUniversal);
    }
    else if (stateTimer < 0.8f) {
        if (attackDir == FaceDir::Right || attackDir == FaceDir::Left) sprite->setTextureRect(framesPreAttackDir[0]);
        else if (attackDir == FaceDir::Down) sprite->setTextureRect(framesPreAttackDown[0]);
        else sprite->setTextureRect(framesPreAttackUp[0]);
        sprite->setScale({ (attackDir == FaceDir::Left ? -scaleFactor : scaleFactor), scaleFactor });
    }
    else {
        showBeam = true;
        currentState = VisState::Attacking;
        stateTimer = 0;
    }
}

void Vis::updateAnimation(float dt) {
    animTimer += dt;
    if (animTimer >= 0.12f) {
        animTimer = 0;
        animFrame = (animFrame + 1) % 4;
    }
    int baseIndex = (faceDir == FaceDir::Left) ? 4 : 0;
    sprite->setTextureRect(walkFrames[baseIndex + animFrame]);
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
    if (r == 0) { moveDir = { 1.f, 0.f };  faceDir = FaceDir::Right; }
    else if (r == 1) { moveDir = { -1.f, 0.f }; faceDir = FaceDir::Left; }
    else if (r == 2) { moveDir = { 0.f, 1.f };  faceDir = FaceDir::Down; }
    else { moveDir = { 0.f, -1.f }; faceDir = FaceDir::Up; }
    if (sprite) sprite->setScale({ (faceDir == FaceDir::Left ? -scaleFactor : scaleFactor), scaleFactor });
}

void Vis::takeDamage(int amount) { health -= amount; isHit = true; hitClock.restart(); }
void Vis::heal(int amount) { health += amount; isHealing = true; healFlashClock.restart(); }
void Vis::setPosition(const sf::Vector2f& pos) { if (sprite) { sprite->setPosition(pos); lastStablePos = pos; } }

sf::FloatRect Vis::getGlobalBounds() const {
    if (sprite) {
        sf::FloatRect b = sprite->getGlobalBounds();
        b.position.x = lastStablePos.x - sprite->getOrigin().x * scaleFactor;
        b.position.y = lastStablePos.y - sprite->getOrigin().y * scaleFactor;
        return b;
    }
    return sf::FloatRect({});
}

sf::FloatRect Vis::getLaserBounds() const { return showBeam ? beamCone.getGlobalBounds() : sf::FloatRect({}); }

// --- DOUBLE VIS ---

DoubleVis::DoubleVis(sf::Texture& sheet) : Vis(sheet) {
    health = 45;
    initRects(139);
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
    Vis::draw(window);
}