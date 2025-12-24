#ifndef VIS_HPP
#define VIS_HPP
#include "Enemy.hpp"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

// Estados de comportamento do Vis
enum class VisState {
    Idle,
    Moving,
    PreAttack,
    Attacking,
    Cooldown
};

class Vis : public EnemyBase {
public:
    Vis(sf::Texture& sheet);

    // Métodos principais de ciclo de vida
    void update(float deltaTime, sf::Vector2f playerPos, const sf::FloatRect& gameBounds) override;
    void draw(sf::RenderWindow& window) override;

    // Métodos de estado e combate
    void takeDamage(int amount) override;
    void heal(int amount) override;

    // Transformações e Colisões
    sf::FloatRect getGlobalBounds() const override;
    void setPosition(const sf::Vector2f& pos) override;

    // Lógica do Laser (Brimstone)
    bool isLaserActive() const { return currentState == VisState::Attacking; }
    sf::FloatRect getLaserBounds() const;

private:
    // Funções auxiliares de lógica interna
    void updateAnimation(float deltaTime);
    void handleAttackSequence(float deltaTime, const sf::FloatRect& gameBounds);
    void resetMovement();

    // Máquina de estados e direções
    VisState currentState;
    FaceDir faceDir;   // Direção para onde está a olhar ao andar
    FaceDir attackDir; // Direção fixa durante o disparo do laser

    // Timers e Controlo de Animação
    float stateTimer;
    float animTimer;
    int animFrame;
    float scaleFactor;
    float distanceWalked;
    sf::Vector2f moveDir;

    // Componentes Visuais do Laser
    sf::ConvexShape beamCone;  // Cone trapezóide para o laser
    sf::CircleShape beamEnd;   // Círculo no fim
    bool showBeam;

    // Contentores de Frames (Spritesheet)
    sf::IntRect framePreUniversal;                             // Sprite 1
    std::array<sf::IntRect, 2> framesPreAttackDir;            // Sprites 2, 3
    std::array<sf::IntRect, 2> framesPreAttackDown;           // Sprites 4, 5
    std::array<sf::IntRect, 2> framesPreAttackUp;             // Sprites 6, 7
    std::array<sf::IntRect, 8> walkFrames;                    // Sprites 8 ao 15
};

#endif