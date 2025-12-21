#ifndef ROOM_HPP
#define ROOM_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>
#include <optional> 
#include <algorithm>
#include "enemy.hpp"
#include "Chubby.hpp"

enum class DoorDirection {
    North,
    South,
    East,
    West,
    None
};

enum class RoomType {
    SafeZone,
    Normal,
    Boss,
    Treasure
};

enum class DoorType {
    Normal,
    Boss,
    Treasure
};

enum class DoorState {
    Open,
    Closing,
    Closed,
    Opening
};

struct Door {
    DoorDirection direction = DoorDirection::None;
    DoorType type = DoorType::Normal;
    std::optional<sf::Sprite> sprite;
    std::optional<sf::Sprite> leftHalf;
    std::optional<sf::Sprite> rightHalf;
    std::optional<sf::Sprite> overlaySprite;
    std::optional<sf::RectangleShape> collisionShape;
    sf::IntRect leftHalfOriginalRect;
    sf::IntRect rightHalfOriginalRect;
    sf::FloatRect bounds;
    bool isOpen = false;
    DoorState state = DoorState::Open;
    float animationProgress = 0.0f;
    int leadsToRoomID = -1;

    Door() = default;
};

class Room {
public:
    Room(int id, RoomType type, const sf::FloatRect& gameBounds);

    // Gestão de Portas
    void addDoor(DoorDirection direction, DoorType doorType, sf::Texture& doorSpritesheet);
    void connectDoor(DoorDirection direction, int targetRoomID);

    // Spawn de Inimigos (Suporta múltiplos de cada tipo)
    void spawnEnemies(
        std::vector<sf::Texture>& demonWalkDown,
        std::vector<sf::Texture>& demonWalkUp,
        std::vector<sf::Texture>& demonWalkLeft,
        std::vector<sf::Texture>& demonWalkRight,
        sf::Texture& demonProjectileTexture,
        std::vector<sf::Texture>& bishopTextures,
        sf::Texture& chubbySheet,
        sf::Texture& chubbyProjSheet
    );

    // Ciclo de Vida
    void update(float deltaTime, sf::Vector2f playerPosition);
    void draw(sf::RenderWindow& window);

    // Getters Básicos
    int getID() const { return roomID; }
    RoomType getType() const { return type; }
    bool isCleared() const { return cleared; }
    bool hasDoor(DoorDirection direction) const;
    int getDoorLeadsTo(DoorDirection direction) const;
    const std::vector<Door>& getDoors() const { return doors; }
    DoorType getDoorType(DoorDirection direction) const;

    // NOVOS GETTERS (Essenciais para o Game.cpp funcionar com listas)
    std::vector<std::unique_ptr<Demon_ALL>>& getDemons() { return demons; }
    std::vector<std::unique_ptr<Bishop_ALL>>& getBishops() { return bishops; }
    std::vector<std::unique_ptr<Chubby>>& getChubbies() { return chubbies; }

    // Auxiliares de Gameplay
    sf::Vector2f getPlayerSpawnPosition(DoorDirection doorDirection) const;
    void checkIfCleared();
    void openDoors();
    void closeDoors();

    // Visual da Sala
    void setCornerTextureRect(const sf::IntRect& rect) { cornerTextureRect = rect; }
    sf::IntRect getCornerTextureRect() const { return cornerTextureRect; }

private:
    void updateDoorAnimations(float deltaTime);
    void updateSingleDoorAnimation(Door& door, float deltaTime);
    void drawDoor(sf::RenderWindow& window, const Door& door) const;
    sf::Vector2f getDoorPosition(DoorDirection direction) const;
    float getDoorRotation(DoorDirection direction) const;

    int roomID;
    RoomType type;
    sf::FloatRect gameBounds;
    std::vector<Door> doors;

    // LISTAS DE INIMIGOS (Permite múltiplos Chubbies/Demons por sala)
    std::vector<std::unique_ptr<Demon_ALL>> demons;
    std::vector<std::unique_ptr<Bishop_ALL>> bishops;
    std::vector<std::unique_ptr<Chubby>> chubbies;

    bool cleared;
    bool doorsOpened;
    sf::IntRect cornerTextureRect;
};

#endif // ROOM_HPP