#ifndef ROOM_HPP
#define ROOM_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <memory>
#include <random>
#include <optional> 

//  INCLUSÃO CRÍTICA: Dá a definição completa das classes Demon_ALL e Bishop_ALL
// Isto resolve o erro de 'classe indefinida' com std::optional
#include "enemy.hpp" 

// Forward declarations (Agora desnecessárias, pois o enemy.hpp define as classes)
// class Demon_ALL; 
// class Bishop_ALL;

// Direções das portas
enum class DoorDirection {
    North,
    South,
    East,
    West,
    None
};

// Estrutura de uma porta
struct Door {
    DoorDirection direction;
    std::optional<sf::Sprite> sprite;

    sf::FloatRect bounds;
    bool isOpen;
    int leadsToRoomID;  // ID da sala conectada (-1 se não conectada)

    Door() : direction(DoorDirection::None), isOpen(false), leadsToRoomID(-1) {}
};

// Tipo de sala
enum class RoomType {
    SafeZone,       // Primeira sala, sem inimigos
    Normal,         // Sala com inimigos
    Boss,           // Sala de boss (futuro)
    Treasure        // Sala de tesouro (futuro)
};

// Classe Room
class Room {
public:
    Room(int id, RoomType type, const sf::FloatRect& gameBounds);

    // Setup
    void addDoor(DoorDirection direction, sf::Texture& doorTexture);
    void connectDoor(DoorDirection direction, int targetRoomID);

    // Enemy management
    void spawnEnemies(
        std::vector<sf::Texture>& demonWalkDown,
        std::vector<sf::Texture>& demonWalkUp,
        std::vector<sf::Texture>& demonWalkLeft,
        std::vector<sf::Texture>& demonWalkRight,
        sf::Texture& demonProjectileTexture,
        std::vector<sf::Texture>& bishopTextures
    );

    // Update & Draw
    void update(float deltaTime, sf::Vector2f playerPosition);
    void draw(sf::RenderWindow& window);

    // Getters
    int getID() const { return roomID; }
    RoomType getType() const { return type; }
    bool isCleared() const { return cleared; }
    bool hasDoor(DoorDirection direction) const;
    int getDoorLeadsTo(DoorDirection direction) const;
    const std::vector<Door>& getDoors() const { return doors; }

    // Enemy access (para colisões externas)
    std::optional<Demon_ALL>& getDemon() { return demon; }
    std::optional<Bishop_ALL>& getBishop() { return bishop; }
    sf::Vector2f getPlayerSpawnPosition(DoorDirection doorDirection) const;

    // Check room status
    void checkIfCleared();
    void openDoors();
    void closeDoors();

private:
    int roomID;
    RoomType type;
    sf::FloatRect gameBounds;


    std::vector<Door> doors;

    // Enemies (opcionais)
    std::optional<Demon_ALL> demon;
    std::optional<Bishop_ALL> bishop;

    bool cleared;
    bool doorsOpened;

    // Helper functions
    sf::Vector2f getDoorPosition(DoorDirection direction) const;
    float getDoorRotation(DoorDirection direction) const;
};

#endif // ROOM_HPP