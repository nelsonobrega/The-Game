#ifndef ROOM_HPP
#define ROOM_HPP

#include "SFML/Graphics.hpp"
#include "AssetManager.hpp"
#include <vector>
#include <memory>

enum class RoomType {
    START,
    NORMAL,
    TREASURE,
    BOSS
};

class Room {
public:
    // Tamanho lógico da grelha da sala
    static constexpr int TILES_X = 30;
    static constexpr int TILES_Y = 15;

    // Tamanho efetivo do tile no ecrã (ajustar isto se necessário)
    static constexpr float TILE_EFFECTIVE_SIZE_X = 64.0f;
    static constexpr float TILE_EFFECTIVE_SIZE_Y = 64.0f;

    int mapX, mapY;

    Room(RoomType type, int x, int y);

    void update(sf::Time deltaTime);
    void draw(sf::RenderWindow& window);

private:
    RoomType type;
    AssetManager& assets;
    std::vector<sf::Sprite> floorTiles;
    std::vector<sf::Sprite> wallTiles;

    void buildRoomLayout();
    // Função auxiliar que carrega a textura e aplica o rect
    void addTile(std::vector<sf::Sprite>& tileVector, const sf::IntRect& rect, float x, float y);
};

#endif // ROOM_HPP