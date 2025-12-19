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

enum class DoorState {
	Open,
	Closing,
	Closed,
	Opening
};

struct Door {
	DoorDirection direction = DoorDirection::None; // Inicialização no local
	DoorType type = DoorType::Normal;             // Inicialização no local
	std::optional<sf::Sprite> sprite;
	std::optional<sf::Sprite> leftHalf;
	std::optional<sf::Sprite> rightHalf;
	std::optional<sf::Sprite> overlaySprite; // NOVO: Sprite para a moldura da porta (overlay)
	std::optional<sf::RectangleShape> collisionShape;
	sf::IntRect leftHalfOriginalRect;
	sf::IntRect rightHalfOriginalRect;
	sf::FloatRect bounds;
	bool isOpen = false;
	DoorState state = DoorState::Open;
	float animationProgress = 0.0f;
	int leadsToRoomID = -1;

	// Usa o construtor padrão do compilador (corrigindo o erro)
	Door() = default;
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
	void updateDoorAnimations(float deltaTime);
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
	void setCornerTextureRect(const sf::IntRect& rect) { cornerTextureRect = rect; }
	sf::IntRect getCornerTextureRect() const { return cornerTextureRect; }
private:
	void updateSingleDoorAnimation(Door& door, float deltaTime);
	void drawDoor(sf::RenderWindow& window, const Door& door) const;
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
	sf::IntRect cornerTextureRect;
};
#endif // ROOM_HPP
