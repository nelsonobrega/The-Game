#ifndef VIS_HPP
#define VIS_HPP

#include "Enemy.hpp"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

enum class VisState { Idle, Moving, PreAttack, Attacking, Cooldown };

class Vis : public EnemyBase {
public:
    Vis(sf::Texture& sheet);
    void update(float deltaTime, sf::Vector2f playerPos, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;

    void takeDamage(int amount) override;
    void heal(int amount) override;
    void setPosition(const sf::Vector2f& pos) override;
    sf::FloatRect getGlobalBounds() const override;

    virtual std::vector<sf::FloatRect> getHazardBounds() const override {
        if (showBeam) return { beamCone.getGlobalBounds() };
        return {};
    }

protected:
    void initRects(int offsetX);
    void updateAnimation(float deltaTime);
    void handleAttackSequence(float deltaTime, const sf::FloatRect& gameBounds);
    void resetMovement();
    void applyBounds(sf::Vector2f& pos, const sf::FloatRect& gb);

    VisState currentState;
    FaceDir faceDir;
    FaceDir attackDir;
    float stateTimer, animTimer, scaleFactor, distanceWalked;
    int animFrame;
    sf::Vector2f moveDir;
    sf::Vector2f lastStablePos; // Posição onde ele "tranca" para disparar

    sf::ConvexShape beamCone;
    sf::CircleShape beamEnd;
    bool showBeam;

    sf::IntRect framePreUniversal;
    std::array<sf::IntRect, 2> framesPreAttackDir, framesPreAttackDown, framesPreAttackUp;
    std::array<sf::IntRect, 8> walkFrames;
};

class DoubleVis : public Vis {
public:
    DoubleVis(sf::Texture& sheet);
    void update(float deltaTime, sf::Vector2f playerPos, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;

    std::vector<sf::FloatRect> getHazardBounds() const override {
        if (showBeam) return { beamCone.getGlobalBounds(), beamConeTras.getGlobalBounds() };
        return {};
    }

private:
    sf::ConvexShape beamConeTras;
    sf::CircleShape beamEndTras;
};

#endif