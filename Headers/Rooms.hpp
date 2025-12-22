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
#include "Monstro.hpp"
#include "Items.hpp" 

enum class DoorDirection { North, South, East, West, None };
enum class RoomType { SafeZone, Normal, Boss, Treasure };
enum class DoorType { Normal, Boss, Treasure };
enum class DoorState { Open, Closing, Closed, Opening };

struct Door {
    DoorDirection direction = DoorDirection::None;
    DoorType type = DoorType::Normal;
    std::optional<sf::Sprite> sprite;
    std::optional<sf::Sprite> leftHalf;
    std::optional<sf::Sprite> rightHalf;
    std::optional<sf::Sprite> overlaySprite;
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

    void addDoor(DoorDirection direction, DoorType doorType, sf::Texture& doorSpritesheet);
    void connectDoor(DoorDirection direction, int targetRoomID);

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

    void update(float deltaTime, sf::Vector2f playerPosition);
    void draw(sf::RenderWindow& window);

    int getID() const { return roomID; }
    RoomType getType() const { return type; }
    bool isCleared() const { return cleared; }
    bool hasDoor(DoorDirection direction) const;
    int getDoorLeadsTo(DoorDirection direction) const;
    const std::vector<Door>& getDoors() const { return doors; }

    std::vector<std::unique_ptr<Demon_ALL>>& getDemons() { return demons; }
    std::vector<std::unique_ptr<Bishop_ALL>>& getBishops() { return bishops; }
    std::vector<std::unique_ptr<Chubby>>& getChubbies() { return chubbies; }
    std::vector<std::unique_ptr<Monstro>>& getMonstros() { return monstros; }

    // --- CORREÇÃO AQUI: Agora aceita itemTex E altarTex ---
    void setRoomItem(ItemType type, sf::Vector2f pos, const sf::Texture& itemTex, const sf::Texture& altarTex) {
        roomItem.emplace(type, pos, itemTex, altarTex);
    }

    std::optional<Item>& getRoomItem() { return roomItem; }

    sf::Vector2f getPlayerSpawnPosition(DoorDirection doorDirection) const;
    void checkIfCleared();
    void openDoors();
    void closeDoors();

    void setCornerTextureRect(const sf::IntRect& rect) { cornerTextureRect = rect; }
    sf::IntRect getCornerTextureRect() const { return cornerTextureRect; }

private:
    void updateDoorAnimations(float deltaTime);
    void drawDoor(sf::RenderWindow& window, const Door& door) const;
    sf::Vector2f getDoorPosition(DoorDirection direction) const;
    float getDoorRotation(DoorDirection direction) const;

    int roomID;
    RoomType type;
    sf::FloatRect gameBounds;
    std::vector<Door> doors;

    std::vector<std::unique_ptr<Demon_ALL>> demons;
    std::vector<std::unique_ptr<Bishop_ALL>> bishops;
    std::vector<std::unique_ptr<Chubby>> chubbies;
    std::vector<std::unique_ptr<Monstro>> monstros;

    std::optional<Item> roomItem;

    bool cleared;
    bool doorsOpened;
    sf::IntRect cornerTextureRect;
};

#endif