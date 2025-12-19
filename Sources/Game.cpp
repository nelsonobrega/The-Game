#include "Game.hpp"
#include "Utils.hpp"
#include "ConfigManager.hpp" 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

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

// --- MODIFICADO: Agora lê o visual guardado na sala para manter a consistência ---
void Game::updateRoomVisuals() {
    Room* curr = roomManager->getCurrentRoom();
    if (!curr) return;

    // Obtém o retângulo que a sala sorteou na sua criação (no RoomsManager)
    sf::IntRect savedRect = curr->getCornerTextureRect();

    // Aplica o retângulo guardado aos 4 cantos
    cornerTL->setTextureRect(savedRect);
    cornerTR->setTextureRect(savedRect);
    cornerBL->setTextureRect(savedRect);
    cornerBR->setTextureRect(savedRect);
}

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(1920, 1080)), "The Game - Isaac Clone"),
    currentState(GameState::menu),
    assets(AssetManager::getInstance())
{
    try { ConfigManager::getInstance().loadConfig("config.json"); }
    catch (const std::exception& e) { throw; }

    const auto& config = ConfigManager::getInstance().getConfig();
    std::srand(static_cast<unsigned>(std::time(NULL)));
    loadGameAssets();

    Isaac.emplace(assets.getAnimationSet("I_Down"), assets.getTexture("TearAtlas"),
        assets.getAnimationSet("I_Up"), assets.getAnimationSet("I_Left"),
        assets.getAnimationSet("I_Right"));

    if (Isaac) {
        const auto& projConfig = config.projectile_textures.isaac_tear;
        Isaac->setProjectileTextureRect(sf::IntRect(sf::Vector2i(projConfig.x, projConfig.y), sf::Vector2i(projConfig.width, projConfig.height)));
        Isaac->setPosition({ (float)config.game.window_width / 2.f, (float)config.game.window_height / 2.f });
    }

    heartSpriteF.emplace(assets.getTexture("HeartF"));
    heartSpriteH.emplace(assets.getTexture("HeartH"));
    heartSpriteE.emplace(assets.getTexture("HeartE"));

    setupMenu();

    cornerTL.emplace(assets.getTexture("BasementCorner"));
    cornerTR.emplace(assets.getTexture("BasementCorner"));
    cornerBL.emplace(assets.getTexture("BasementCorner"));
    cornerBR.emplace(assets.getTexture("BasementCorner"));

    // Configuração de escala e posição dos cantos baseada na config
    float scaleX = (float)config.game.window_width / 2.f / (float)config.corners.option_a.width;
    float scaleY = (float)config.game.window_height / 2.f / (float)config.corners.option_a.height;
    cornerTL->setPosition({ 0, 0 }); cornerTL->setScale({ scaleX, scaleY });
    cornerTR->setPosition({ (float)config.game.window_width, 0 }); cornerTR->setScale({ -scaleX, scaleY });
    cornerBL->setPosition({ 0, (float)config.game.window_height }); cornerBL->setScale({ scaleX, -scaleY });
    cornerBR->setPosition({ (float)config.game.window_width, (float)config.game.window_height }); cornerBR->setScale({ -scaleX, -scaleY });

    gameBounds = sf::FloatRect({ config.game.bounds.left, config.game.bounds.top }, { config.game.bounds.width, config.game.bounds.height });
    roomManager.emplace(assets, gameBounds);

    int numRooms = config.game.dungeon.min_rooms + (std::rand() % (config.game.dungeon.max_rooms - config.game.dungeon.min_rooms + 1));
    roomManager->generateDungeon(numRooms);

    // Inicializa o visual da primeira sala
    updateRoomVisuals();
}

void Game::run() {
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        processEvents();
        if (currentState == GameState::menu) {
            if (playButton && isMouseOver(*playButton) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) currentState = GameState::playing;
            window.clear();
            if (menuGround) window.draw(*menuGround);
            if (playButton) window.draw(*playButton);
            if (exitButton) window.draw(*exitButton);
            window.display();
        }
        else if (currentState == GameState::playing) {
            update(deltaTime.asSeconds());
            render();
        }
    }
}

void Game::update(float deltaTime) {
    if (!Isaac || !roomManager) return;
    const auto& config = ConfigManager::getInstance().getConfig();
    sf::Vector2f playerPosition = Isaac->getPosition();

    // --- LÓGICA DE TRANSIÇÃO (Sincronizada com RoomsManager) ---
    if (roomManager->isTransitioning()) {
        Room* roomBefore = roomManager->getCurrentRoom();

        roomManager->updateTransition(deltaTime, playerPosition);
        Isaac->setPosition(playerPosition);
        Isaac->setSpeedMultiplier(0.f); // Bloqueia movimento na transição

        // Se o ponteiro da sala mudou DURANTE a transição (ecrã preto)
        if (roomManager->getCurrentRoom() != roomBefore) {
            updateRoomVisuals(); // Aplica o visual guardado na nova sala
        }
        return;
    }
    Isaac->setSpeedMultiplier(1.f);

    Isaac->update(deltaTime, gameBounds);
    roomManager->update(deltaTime, Isaac->getPosition());

    DoorDirection doorHit = roomManager->checkPlayerAtDoor(Isaac->getGlobalBounds());
    if (doorHit != DoorDirection::None) {
        roomManager->requestTransition(doorHit);
        return;
    }

    // Colisões com inimigos
    Room* currentRoom = roomManager->getCurrentRoom();
    if (currentRoom) {
        auto& isaacProjectiles = Isaac->getProjectiles();
        auto& demon = currentRoom->getDemon();
        auto& bishop = currentRoom->getBishop();
        for (auto it = isaacProjectiles.begin(); it != isaacProjectiles.end();) {
            bool hit = false;
            if (demon && demon->getHealth() > 0 && checkCollision(it->sprite.getGlobalBounds(), demon->getGlobalBounds())) {
                demon->takeDamage(config.player.stats.damage); hit = true;
            }
            if (bishop && bishop->getHealth() > 0 && checkCollision(it->sprite.getGlobalBounds(), bishop->getGlobalBounds())) {
                bishop->takeDamage(config.player.stats.damage); hit = true;
            }
            if (hit) it = isaacProjectiles.erase(it); else ++it;
        }
    }

    if (Isaac->getHealth() <= 0) window.close();
}

void Game::render() {
    window.clear();
    const auto& config = ConfigManager::getInstance().getConfig();

    // Desenha o fundo (cantos)
    if (cornerTL) window.draw(*cornerTL);
    if (cornerTR) window.draw(*cornerTR);
    if (cornerBL) window.draw(*cornerBL);
    if (cornerBR) window.draw(*cornerBR);

    // Desenha as portas e inimigos da sala
    if (roomManager) roomManager->draw(window);

    // Desenha o Isaac
    if (Isaac) Isaac->draw(window);

    // UI de HP (Corações)
    if (Isaac && heartSpriteF) {
        int hp = Isaac->getHealth();
        float x = config.game.ui.heart_ui_x;
        for (int i = 0; i < config.game.ui.max_hearts; ++i) {
            sf::Sprite* s = (hp >= 2) ? &*heartSpriteF : (hp == 1 ? &*heartSpriteH : &*heartSpriteE);
            s->setPosition({ x, config.game.ui.heart_ui_y });
            s->setScale({ config.game.ui.heart_scale, config.game.ui.heart_scale });
            window.draw(*s);
            x += config.game.ui.heart_spacing; hp -= 2;
        }
    }

    // Minimapa e Overlay de Transição (Desenha por cima de tudo)
    if (roomManager) {
        roomManager->drawMiniMap(window);
        roomManager->drawTransitionOverlay(window);
    }
    window.display();
}

void Game::processEvents() {
    while (std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();
    }
}

void Game::setupMenu() {
    const auto& config = ConfigManager::getInstance().getConfig();
    if (playButtonTexture.loadFromFile("Images/UI/Menu/playButton.png")) {
        playButton.emplace(playButtonTexture);
        playButton->setPosition({ config.game.menu.play_button.position_x, config.game.menu.play_button.position_y });
        playButton->setScale({ config.game.menu.play_button.scale_x, config.game.menu.play_button.scale_y });
    }
    if (exitButtonTexture.loadFromFile("Images/UI/Menu/exitButton.png")) {
        exitButton.emplace(exitButtonTexture);
        exitButton->setPosition({ config.game.menu.exit_button.position_x, config.game.menu.exit_button.position_y });
        exitButton->setScale({ config.game.menu.exit_button.scale_x, config.game.menu.exit_button.scale_y });
    }
    if (menuTexture.loadFromFile("Images/UI/Menu/backgroundMenu.png")) {
        menuGround.emplace(menuTexture);
        menuGround->setScale({ config.game.menu.background_scale_x, config.game.menu.background_scale_y });
    }
}

bool Game::isMouseOver(const sf::Sprite& sprite) {
    auto mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
    return sprite.getGlobalBounds().contains(mousePos);
}