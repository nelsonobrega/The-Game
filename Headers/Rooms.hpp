#ifndef ROOM_HPP
#define ROOM_HPP
#include "SFML/Graphics.hpp"
#include <vector>
#include <memory>
#include <random>
#include <optional> 
#include "enemy.hpp" 

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

struct Door {
    DoorDirection direction;
    DoorType type;
    std::optional<sf::Sprite> sprite;
    sf::FloatRect bounds;
    bool isOpen;
    int leadsToRoomID;
    Door() : direction(DoorDirection::None), type(DoorType::Normal), isOpen(false), leadsToRoomID(-1) {}
};

class Room {
public:
    Room(int id, RoomType type, const sf::FloatRect& gameBounds);

    // MODIFICADO: Aceita a textura do spritesheet (todas as portas estão na mesma)
    void addDoor(DoorDirection direction, DoorType doorType, sf::Texture& doorSpritesheet);

    void connectDoor(DoorDirection direction, int targetRoomID);
    void spawnEnemies(
        std::vector<sf::Texture>& demonWalkDown,
        std::vector<sf::Texture>& demonWalkUp,
        std::vector<sf::Texture>& demonWalkLeft,
        std::vector<sf::Texture>& demonWalkRight,
        sf::Texture& demonProjectileTexture,
        std::vector<sf::Texture>& bishopTextures
    );
    void update(float deltaTime, sf::Vector2f playerPosition);
    void draw(sf::RenderWindow& window);
    int getID() const { return roomID; }
    RoomType getType() const { return type; }
    bool isCleared() const { return cleared; }
    bool hasDoor(DoorDirection direction) const;
    int getDoorLeadsTo(DoorDirection direction) const;
    const std::vector<Door>& getDoors() const { return doors; }
    DoorType getDoorType(DoorDirection direction) const;
    std::optional<Demon_ALL>& getDemon() { return demon; }
    std::optional<Bishop_ALL>& getBishop() { return bishop; }
    sf::Vector2f getPlayerSpawnPosition(DoorDirection doorDirection) const;
    void checkIfCleared();
    void openDoors();
    void closeDoors();
private:
    int roomID;
    RoomType type;
    sf::FloatRect gameBounds;
    std::vector<Door> doors;
    std::optional<Demon_ALL> demon;
    std::optional<Bishop_ALL> bishop;
    bool cleared;
    bool doorsOpened;
    sf::Vector2f getDoorPosition(DoorDirection direction) const;
    float getDoorRotation(DoorDirection direction) const;
};
#endif // ROOM_HPP