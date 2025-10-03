#ifndef GAME_HPP
#define GAME_HPP

#include "SFML/Graphics.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "AssetManager.hpp"
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

    // Game objects opcionais para evitar erros de construtor padrão e operador de atribuição
    std::optional<Player_ALL> Isaac;
    std::optional<Demon_ALL> Enemy;
    std::optional<Bishop_ALL> eBishop;

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