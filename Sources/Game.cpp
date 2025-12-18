#include "Game.hpp"
#include "Utils.hpp"
#include "ConfigManager.hpp" 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// --- Função Auxiliar para Aleatorizar a Textura ---
sf::IntRect getRandomCornerTexture(const GameConfig& config) {
    const auto& cornerConfig = config.corners;

    const sf::IntRect TEXTURE_OPTION_A(
        sf::Vector2i(cornerConfig.option_a.x, cornerConfig.option_a.y),
        sf::Vector2i(cornerConfig.option_a.width, cornerConfig.option_a.height)
    );
    const sf::IntRect TEXTURE_OPTION_B(
        sf::Vector2i(cornerConfig.option_b.x, cornerConfig.option_b.y),
        sf::Vector2i(cornerConfig.option_b.width, cornerConfig.option_b.height)
    );
    const sf::IntRect TEXTURE_OPTION_C(
        sf::Vector2i(cornerConfig.option_c.x, cornerConfig.option_c.y),
        sf::Vector2i(cornerConfig.option_c.width, cornerConfig.option_c.height)
    );

    int choice = rand() % 3;
    if (choice == 0) return TEXTURE_OPTION_A;
    else if (choice == 1) return TEXTURE_OPTION_B;
    else return TEXTURE_OPTION_C;
}

// --- Implementação da Classe Game ---

void Game::loadGameAssets() {
    assets.loadAnimation("I_Down", "Isaac/Front_Isaac", "F", 9, "V1.png");
    assets.loadAnimation("I_Up", "Isaac/Back_Isaac", "B", 9, "V1.png");
    assets.loadAnimation("I_Left", "Isaac/Left_Isaac", "L", 6, "V1.png");
    assets.loadAnimation("I_Right", "Isaac/Right_Isaac", "R", 6, "V1.png");

    assets.loadTexture("TearAtlas", "Images/Tears/bulletatlas.png");

    assets.loadAnimation("D_Down", "Demon/Front_Demon", "F", 8, "D.png");
    assets.loadAnimation("D_Up", "Demon/Back_Demon", "B", 8, "D.png");
    assets.loadAnimation("D_Left", "Demon/Left_Demon", "L", 8, "D.png");
    assets.loadAnimation("D_Right", "Demon/Right_Demon", "R", 8, "D.png");

    assets.loadAnimation("Bishop", "Bishop", "B", 14, ".png");

    assets.loadTexture("Door", "Images/Background/Doors.png");
    assets.loadTexture("HeartF", "Images/UI/Life/Full.png");
    assets.loadTexture("HeartH", "Images/UI/Life/Half.png");
    assets.loadTexture("HeartE", "Images/UI/Life/Empty.png");

    assets.loadTexture("BasementCorner", "Images/Background/Basement_sheet.png");
}

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(1920, 1080)), "The Game - Isaac Clone"),
    currentState(GameState::menu),
    assets(AssetManager::getInstance())
{
    // PASSO 1: CARREGAR CONFIG **PRIMEIRO**
    try {
        ConfigManager::getInstance().loadConfig("config.json");
        std::cout << "Configuracao carregada com sucesso.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "ERRO FATAL AO CARREGAR CONFIG: " << e.what() << "\n";
        throw;
    }

    // PASSO 2: Agora sim podemos usar o config
    const auto& config = ConfigManager::getInstance().getConfig();

    std::srand(std::time(NULL));

    loadGameAssets();

    // --- CONFIGURAÇÃO DO ISAAC ---
    Isaac.emplace(
        assets.getAnimationSet("I_Down"),
        assets.getTexture("TearAtlas"),
        assets.getAnimationSet("I_Up"),
        assets.getAnimationSet("I_Left"),
        assets.getAnimationSet("I_Right")
    );

    if (Isaac) {
        // Define o rect do projétil do Isaac
        const auto& projConfig = config.projectile_textures.isaac_tear;
        const sf::IntRect ISAAC_TEAR_RECT(
            sf::Vector2i(projConfig.x, projConfig.y),
            sf::Vector2i(projConfig.width, projConfig.height)
        );
        Isaac->setProjectileTextureRect(ISAAC_TEAR_RECT);
        Isaac->setPosition({ (float)config.game.window_width / 2.f, (float)config.game.window_height / 2.f });
    }

    // Inicializa sprites opcionais para corações
    heartSpriteF.emplace(assets.getTexture("HeartF"));
    heartSpriteH.emplace(assets.getTexture("HeartH"));
    heartSpriteE.emplace(assets.getTexture("HeartE"));

    setupMenu();

    // Inicializar sprites opcionais com as texturas carregadas (cantos)
    cornerTL.emplace(assets.getTexture("BasementCorner"));
    cornerTR.emplace(assets.getTexture("BasementCorner"));
    cornerBL.emplace(assets.getTexture("BasementCorner"));
    cornerBR.emplace(assets.getTexture("BasementCorner"));

    // Setup basement corners
    float scaleX = (float)config.game.window_width / 2.f / (float)config.corners.option_a.width;
    float scaleY = (float)config.game.window_height / 2.f / (float)config.corners.option_a.height;

    // Canto Superior Esquerdo (TL)
    cornerTL->setTextureRect(getRandomCornerTexture(config));
    cornerTL->setPosition({ 0.f, 0.f });
    cornerTL->setScale({ scaleX, scaleY });

    // Canto Superior Direito (TR)
    cornerTR->setTextureRect(getRandomCornerTexture(config));
    cornerTR->setPosition({ (float)config.game.window_width, 0.f });
    cornerTR->setScale({ -scaleX, scaleY });

    // Canto Inferior Esquerdo (BL)
    cornerBL->setTextureRect(getRandomCornerTexture(config));
    cornerBL->setPosition({ 0.f, (float)config.game.window_height });
    cornerBL->setScale({ scaleX, -scaleY });

    // Canto Inferior Direito (BR)
    cornerBR->setTextureRect(getRandomCornerTexture(config));
    cornerBR->setPosition({ (float)config.game.window_width, (float)config.game.window_height });
    cornerBR->setScale({ -scaleX, -scaleY });

    // Define game bounds
    gameBounds = sf::FloatRect({ config.game.bounds.left, config.game.bounds.top }, { config.game.bounds.width, config.game.bounds.height });

    // --- CONFIGURAÇÃO DO ROOM MANAGER ---
    roomManager.emplace(assets, gameBounds);

    // Gera número aleatório de salas entre min e max (lidos da config)
    int minRooms = config.game.dungeon.min_rooms;
    int maxRooms = config.game.dungeon.max_rooms;
    int numRooms = minRooms + (rand() % (maxRooms - minRooms + 1));

    std::cout << "Gerando dungeon com " << numRooms << " salas..." << std::endl;
    roomManager->generateDungeon(numRooms);
}

void Game::setupMenu() {
    const auto& config = ConfigManager::getInstance().getConfig();

    if (!playButtonTexture.loadFromFile("Images/UI/Menu/playButton.png")) {
        throw std::runtime_error("Failed to load playButton.png");
    }
    playButton.emplace(playButtonTexture);
    playButton->setPosition({ config.game.menu.play_button.position_x, config.game.menu.play_button.position_y });
    playButton->setScale(sf::Vector2f(config.game.menu.play_button.scale_x, config.game.menu.play_button.scale_y));

    if (!exitButtonTexture.loadFromFile("Images/UI/Menu/exitButton.png")) {
        throw std::runtime_error("Failed to load exitButton.png");
    }
    exitButton.emplace(exitButtonTexture);
    exitButton->setPosition({ config.game.menu.exit_button.position_x, config.game.menu.exit_button.position_y });
    exitButton->setScale(sf::Vector2f(config.game.menu.exit_button.scale_x, config.game.menu.exit_button.scale_y));

    if (!menuTexture.loadFromFile("Images/UI/Menu/backgroundMenu.png")) {
        throw std::runtime_error("Failed to load backgroundMenu.png");
    }
    menuGround.emplace(menuTexture);
    menuGround->setScale({ config.game.menu.background_scale_x, config.game.menu.background_scale_y });
}

void Game::run() {
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();

        processEvents();

        if (currentState == GameState::menu) {
            if (playButton && isMouseOver(*playButton) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                currentState = GameState::playing;
            }
            else if (exitButton && isMouseOver(*exitButton) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                currentState = GameState::exiting;
            }

            window.clear();
            if (menuGround) window.draw(*menuGround);
            if (playButton) window.draw(*playButton);
            if (exitButton) window.draw(*exitButton);
            window.display();

            if (currentState == GameState::exiting) {
                window.close();
            }
        }
        else if (currentState == GameState::playing) {
            update(deltaTime.asSeconds());
            render();
        }
        else if (currentState == GameState::exiting) {
            window.close();
        }
    }
}

// CORRIGIDO: Eventos no SFML 3.0
void Game::processEvents() {
    while (std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
    }
}

void Game::update(float deltaTime) {
    if (!Isaac || !roomManager) return;

    const auto& config = ConfigManager::getInstance().getConfig();
    sf::Vector2f playerPosition = Isaac->getPosition();

    // 1. Lógica de Transição (TEM PRIORIDADE)
    if (roomManager->isTransitioning()) {
        roomManager->updateTransition(deltaTime, playerPosition);
        Isaac->setPosition(playerPosition);
        Isaac->setSpeedMultiplier(0.f);
        return;
    }
    Isaac->setSpeedMultiplier(1.f);

    // 2. Atualização do Player (Movimento, Tiro)
    Isaac->update(deltaTime, gameBounds);

    // 3. Atualização da Sala (Inimigos, Cura, Checagem de Limpeza)
    roomManager->update(deltaTime, Isaac->getPosition());

    // 4. Checagem de Porta
    DoorDirection doorHit = roomManager->checkPlayerAtDoor(Isaac->getGlobalBounds());
    if (doorHit != DoorDirection::None) {
        roomManager->requestTransition(doorHit);
        return;
    }

    // --- Lógica de Colisão de Projéteis ---

    auto& isaacProjectiles = Isaac->getProjectiles();
    Room* currentRoom = roomManager->getCurrentRoom();

    if (currentRoom) {
        auto& demon = currentRoom->getDemon();
        auto& bishop = currentRoom->getBishop();

        // Colisão: Projéteis do Isaac vs. Inimigos
        for (auto it = isaacProjectiles.begin(); it != isaacProjectiles.end();) {
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();
            bool projectile_hit = false;

            int playerDamage = config.player.stats.damage;

            // 1. COLISÃO COM O DEMON
            if (demon.has_value() && demon->getHealth() > 0 && checkCollision(projBounds, demon->getGlobalBounds())) {
                demon->takeDamage(playerDamage);
                projectile_hit = true;
            }

            // 2. COLISÃO COM O BISHOP
            if (bishop.has_value() && bishop->getHealth() > 0 && checkCollision(projBounds, bishop->getGlobalBounds())) {
                bishop->takeDamage(playerDamage);
                projectile_hit = true;
            }

            if (projectile_hit) {
                it = isaacProjectiles.erase(it);
            }
            else {
                ++it;
            }
        }

        // Colisão: Projéteis Inimigos (Demon) vs. Player
        if (demon.has_value() && demon->getHealth() > 0) {
            auto& enemyProjectiles = demon->getProjectiles();
            for (auto it = enemyProjectiles.begin(); it != enemyProjectiles.end();) {
                sf::FloatRect projBounds = it->sprite.getGlobalBounds();

                if (Isaac->getHealth() > 0 && checkCollision(projBounds, Isaac->getGlobalBounds())) {
                    Isaac->takeDamage(config.demon.stats.damage);
                    it = enemyProjectiles.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
    }

    // Lógica de morte do Isaac
    if (Isaac && Isaac->getHealth() <= 0) {
        window.close();
    }
}

void Game::render() {
    window.clear();

    const auto& config = ConfigManager::getInstance().getConfig();

    // Desenho dos Cantos da Sala (Background)
    if (cornerTL) window.draw(*cornerTL);
    if (cornerTR) window.draw(*cornerTR);
    if (cornerBL) window.draw(*cornerBL);
    if (cornerBR) window.draw(*cornerBR);

    // --- Desenho da Sala ---
    if (roomManager) {
        roomManager->draw(window);
    }

    // O Player deve ser desenhado sempre acima dos inimigos
    if (Isaac) Isaac->draw(window);

    // --- Desenho da UI (Saúde) ---
    if (!Isaac) return;

    int currentHealth = Isaac->getHealth();
    const auto& uiConfig = config.game.ui;
    float xPosition = uiConfig.heart_ui_x;
    float yPosition = uiConfig.heart_ui_y;
    const int maxHearts = uiConfig.max_hearts;
    const float heartSpacing = uiConfig.heart_spacing;

    if (!heartSpriteF || !heartSpriteH || !heartSpriteE) return;

    heartSpriteF->setScale({ uiConfig.heart_scale, uiConfig.heart_scale });
    heartSpriteH->setScale({ uiConfig.heart_scale, uiConfig.heart_scale });
    heartSpriteE->setScale({ uiConfig.heart_scale, uiConfig.heart_scale });

    for (int i = 0; i < maxHearts; ++i) {
        const sf::Sprite* spriteToDraw;
        if (currentHealth >= 2) spriteToDraw = &*heartSpriteF;
        else if (currentHealth == 1) spriteToDraw = &*heartSpriteH;
        else spriteToDraw = &*heartSpriteE;

        const_cast<sf::Sprite*>(spriteToDraw)->setPosition({ xPosition, yPosition });
        window.draw(*spriteToDraw);
        xPosition += heartSpacing;
        currentHealth -= 2;
    }

    // --- Desenho do Minimapa ---
    if (roomManager) {
        roomManager->drawMiniMap(window);
    }

    // --- Desenho do Overlay de Transição (DEVE SER O ÚLTIMO) ---
    if (roomManager) {
        roomManager->drawTransitionOverlay(window);
    }

    window.display();
}

bool Game::isMouseOver(const sf::Sprite& sprite) {
    auto mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
    return sprite.getGlobalBounds().contains(mousePos);
}