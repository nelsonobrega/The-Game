#ifndef GAME_HPP
#define GAME_HPP

#include "SFML/Graphics.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "AssetManager.hpp"
#include "RoomsManager.hpp" // NOVO: Incluir o RoomsManager
#include <optional>

enum class GameState {
    menu,
    playing,
    exiting,
};

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void updateRoomVisuals();
    void render();

    void loadGameAssets();
    void setupMenu();

    sf::RenderWindow window;
    GameState currentState;

    // Textures para menu e UI
    sf::Texture playButtonTexture;
    sf::Texture exitButtonTexture;
    sf::Texture menuTexture;
    sf::Texture basementCornerTexture;

    // Sprites opcionais para menu e UI
    std::optional<sf::Sprite> playButton;
    std::optional<sf::Sprite> exitButton;
    std::optional<sf::Sprite> menuGround;

    // Sprites opcionais para cantos do basement
    std::optional<sf::Sprite> cornerTL;
    std::optional<sf::Sprite> cornerTR;
    std::optional<sf::Sprite> cornerBL;
    std::optional<sf::Sprite> cornerBR;

    // NOVO: O RoomManager agora gere todas as salas e inimigos
    std::optional<RoomManager> roomManager;

    // Game object opcional para o player
    std::optional<Player_ALL> Isaac;

    // REMOVIDO: Demon e Bishop (agora geridos pelo RoomManager)

    // Sprites opcionais para corações de vida
    std::optional<sf::Sprite> heartSpriteF;
    std::optional<sf::Sprite> heartSpriteH;
    std::optional<sf::Sprite> heartSpriteE;

    sf::FloatRect gameBounds;

    sf::Clock clock;

    AssetManager& assets;

    bool isMouseOver(const sf::Sprite& sprite);
};

#endif // GAME_HPP