#ifndef ROOMMANAGER_HPP
#define ROOMMANAGER_HPP

#include "Rooms.hpp"
#include "AssetManager.hpp"
#include "ConfigManager.hpp" 
#include <vector>
#include <map>
#include <set>
#include <random>
#include <SFML/Graphics.hpp>

// Comparador para usar sf::Vector2i como chave em std::map
struct Vector2iComparator {
    bool operator()(const sf::Vector2i& left, const sf::Vector2i& right) const {
        if (left.x != right.x) return left.x < right.x;
        return left.y < right.y;
    }
};

enum class TransitionState { None, FadingOut, Moving, FadingIn };

class RoomManager {
public:
    RoomManager(AssetManager& assetManager, const sf::FloatRect& gameBounds);

    void generateDungeon(int numRooms);

    void requestTransition(DoorDirection direction);
    void updateTransition(float deltaTime, sf::Vector2f& playerPosition);
    bool isTransitioning() const { return transitionState != TransitionState::None; }

    void update(float deltaTime, sf::Vector2f playerPosition);
    void draw(sf::RenderWindow& window);
    void drawTransitionOverlay(sf::RenderWindow& window);

    Room* getCurrentRoom() { return currentRoom; }
    int getCurrentRoomID() const { return currentRoomID; }

    DoorDirection checkPlayerAtDoor(const sf::FloatRect& playerBounds);

    void drawMiniMap(sf::RenderWindow& window);
    sf::Vector2i getCurrentRoomCoord() const;

private:
    AssetManager& assets;
    sf::FloatRect gameBounds;

    std::map<int, Room> rooms;
    Room* currentRoom;
    int currentRoomID;
    int nextRoomID;

    std::map<sf::Vector2i, int, Vector2iComparator> coordToRoomID;

    bool bossRoomGenerated = false;
    bool treasureRoomGenerated = false;
    std::set<int> visitedRooms;

    // Variáveis de Transição
    TransitionState transitionState;
    DoorDirection transitionDirection;
    float transitionProgress;
    float transitionDuration;

    sf::RectangleShape transitionOverlay;

    // Helpers de Geração
    void createRoom(int id, RoomType type);
    void connectRooms(int roomA, int roomB, DoorDirection directionFromA);
    DoorDirection getOppositeDirection(DoorDirection direction);

    std::mt19937 rng;
    std::random_device rd;
};

#endif