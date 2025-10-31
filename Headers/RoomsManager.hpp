#ifndef ROOMMANAGER_HPP
#define ROOMMANAGER_HPP

#include "Rooms.hpp"
#include "AssetManager.hpp"
#include <vector>
#include <map>
#include <set>
#include <SFML/System/Vector2.hpp>

// 🚨 CORREÇÃO ROBUSTA: Comparador personalizado para sf::Vector2i
// Isto resolve o erro do compilador forçando o uso de um método de comparação.
struct Vector2iComparator {
    bool operator()(const sf::Vector2i& left, const sf::Vector2i& right) const {
        // Compara o X
        if (left.x != right.x) {
            return left.x < right.x;
        }
        // Se X for igual, compara o Y
        return left.y < right.y;
    }
};


// Estado da transição entre salas
enum class TransitionState {
    None,
    FadingOut,
    Moving,
    FadingIn
};

// Classe que gerencia o labirinto de salas
class RoomManager {
public:
    RoomManager(AssetManager& assetManager, const sf::FloatRect& gameBounds);

    // Gera o labirinto
    void generateDungeon(int numRooms);

    // Transição entre salas
    void requestTransition(DoorDirection direction);
    void updateTransition(float deltaTime, sf::Vector2f& playerPosition);
    bool isTransitioning() const { return transitionState != TransitionState::None; }

    // Update e Draw
    void update(float deltaTime, sf::Vector2f playerPosition);
    void draw(sf::RenderWindow& window);
    void drawTransitionOverlay(sf::RenderWindow& window);

    // Getters
    Room* getCurrentRoom() { return currentRoom; }
    int getCurrentRoomID() const { return currentRoomID; }

    // Verifica se player está numa porta
    DoorDirection checkPlayerAtDoor(const sf::FloatRect& playerBounds);

    // NOVO: Métodos para o minimapa
    void drawMiniMap(sf::RenderWindow& window);
    sf::Vector2i getCurrentRoomCoord() const;

private:
    AssetManager& assets;
    sf::FloatRect gameBounds;

    std::map<int, Room> rooms;
    Room* currentRoom;
    int currentRoomID;
    int nextRoomID;

    // NOVO: Usa o comparador personalizado Vector2iComparator
    std::map<sf::Vector2i, int, Vector2iComparator> coordToRoomID;

    // NOVO: Rastreamento de salas especiais
    bool bossRoomGenerated = false;
    bool treasureRoomGenerated = false;

    // NOVO: Rastreamento de salas visitadas para o minimapa
    std::set<int> visitedRooms;

    // Transição
    TransitionState transitionState;
    DoorDirection transitionDirection;
    float transitionProgress;
    const float transitionDuration = 0.5f;  // 0.5 segundos

    sf::RectangleShape transitionOverlay;

    // Geração do labirinto
    void createRoom(int id, RoomType type);
    void connectRooms(int roomA, int roomB, DoorDirection directionFromA);
    DoorDirection getOppositeDirection(DoorDirection direction);

    // Helper
    sf::Vector2f getTransitionOffset(DoorDirection direction, float progress);

    std::mt19937 rng; // Motor de geração
    std::random_device rd; // Seed para inicializar o motor
};

#endif // ROOMMANAGER_HPP
