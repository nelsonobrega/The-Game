#ifndef MONSTRO_HPP
#define MONSTRO_HPP

#include "enemy.hpp"
#include <map>
#include <vector>

enum class MonstroState {
    Idle,
    Walking,
    Attacking,
    MegaJumping,
    Falling,
    Cooldown
};

class Monstro : public EnemyBase {
public:
    Monstro(sf::Texture& texture, sf::Texture& projectileTex, sf::Vector2f startPos);
    void update(float deltaTime, sf::Vector2f playerPosition, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos);

    // Adicionado para o Game.cpp conseguir ler o estado
    MonstroState getState() const { return state; }

private:
    MonstroState state;
    float stateTimer;
    int animStep;
    float baseScale = 3.1f;
    bool lastActionWasAttack = false;
    float maxHealth;

    sf::Vector2f groundPos;
    sf::Vector2f groundPosTarget;
    sf::Vector2f moveDir;

    sf::CircleShape shadow;
    sf::Texture* projTex;

    std::map<int, std::map<int, sf::IntRect>> frames;

    void initFrames();
    void setFrame(int coluna, int id);
    void handleStates(float deltaTime, sf::Vector2f playerPos);
    void spawnTears(int count, bool circular);
};

#endif