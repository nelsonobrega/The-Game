#include "Game.hpp"
#include "Utils.hpp"
#include "ConfigManager.hpp" 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm> // Necessário para std::min

// --- Definições de Texturas de Canto (3 Opções Aleatórias) ---
const sf::IntRect TEXTURE_OPTION_A = { {0, 0 }, { 234, 156 } };
const sf::IntRect TEXTURE_OPTION_B = { {0, 156 }, {234, 155} };
const sf::IntRect TEXTURE_OPTION_C = { {234, 0}, {234, 155} };

// --- Definições dos Recortes de Projéteis (Valores Hardcoded) ---
const sf::IntRect ISAAC_TEAR_RECT = { {8, 39}, {16, 16} };
// const sf::IntRect DEMON_TEAR_RECT = { {8, 103}, {16, 16} }; // Não é mais necessário aqui

// --- Função Auxiliar para Aleatorizar a Textura ---
sf::IntRect getRandomCornerTexture() {
    int choice = rand() % 3;
    if (choice == 0) return TEXTURE_OPTION_A;
    else if (choice == 1) return TEXTURE_OPTION_B;
    else return TEXTURE_OPTION_C;
}

// --- Implementação da Classe Game ---

void Game::loadGameAssets() {
    assets.loadAnimation("I_Down", "Front_Isaac", "F", 9, "V1.png");
    assets.loadAnimation("I_Up", "Back_Isaac", "B", 9, "V1.png");
    assets.loadAnimation("I_Left", "Left_Isaac", "L", 6, "V1.png");
    assets.loadAnimation("I_Right", "Right_Isaac", "R", 6, "V1.png");

    assets.loadTexture("TearAtlas", "Images/Tears/bulletatlas.png");

    assets.loadAnimation("D_Down", "Front_Demon", "F", 8, "D.png");
    assets.loadAnimation("D_Up", "Back_Demon", "B", 8, "D.png");
    assets.loadAnimation("D_Left", "Left_Demon", "L", 8, "D.png");
    assets.loadAnimation("D_Right", "Right_Demon", "R", 8, "D.png");

    assets.loadAnimation("Bishop", "Bishop", "B", 14, ".png");

    assets.loadTexture("Door", "Images/Background/Doors.png"); // NOVO: Textura da porta
    assets.loadTexture("HeartF", "Images/UI/Life/Full.png");
    assets.loadTexture("HeartH", "Images/UI/Life/Half.png");
    assets.loadTexture("HeartE", "Images/UI/Life/Empty.png");

    assets.loadTexture("BasementCorner", "Images/Background/Basement_sheet.png");
}

Game::Game()
    : window(sf::VideoMode({ 1920, 1080 }), "Isaac test"),
    currentState(GameState::menu),
    assets(AssetManager::getInstance())
{
    // PASSO CRÍTICO: CARREGAR A CONFIGURAÇÃO ANTES DE USÁ-LA
    try {
        ConfigManager::getInstance().loadConfig("config.json");
        std::cout << "Configuracao carregada com sucesso.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "ERRO FATAL AO CARREGAR CONFIG: " << e.what() << "\n";
        throw;
    }

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
        Isaac->setProjectileTextureRect(ISAAC_TEAR_RECT);
        // Coloca o Isaac no centro do ecrã antes da transição (será movido depois)
        Isaac->setPosition({ 1920.f / 2.f, 1080.f / 2.f });
    }

    // REMOVIDO: Enemy.emplace() e eBishop.emplace()

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
    float scaleX = 960.f / 234.f;
    float scaleY = 540.f / 156.f;

    // Canto Superior Esquerdo (TL)
    cornerTL->setTextureRect(getRandomCornerTexture());
    cornerTL->setPosition({ 0.f, 0.f });
    cornerTL->setScale({ scaleX, scaleY });

    // Canto Superior Direito (TR)
    cornerTR->setTextureRect(getRandomCornerTexture());
    cornerTR->setPosition({ 1920.f, 0.f });
    cornerTR->setScale({ -scaleX, scaleY });

    // Canto Inferior Esquerdo (BL)
    cornerBL->setTextureRect(getRandomCornerTexture());
    cornerBL->setPosition({ 0.f, 1080.f });
    cornerBL->setScale({ scaleX, -scaleY - 0.1f });

    // Canto Inferior Direito (BR)
    cornerBR->setTextureRect(getRandomCornerTexture());
    cornerBR->setPosition({ 1920.f, 1080.f });
    cornerBR->setScale({ -scaleX, -scaleY - 0.1f });

    // Define game bounds
    float left = 213.33f;
    float top = 179.80f;
    float width = 1493.34f;
    float height = 720.40f;
    gameBounds = sf::FloatRect({ left, top }, { width, height });

    // --- NOVO: CONFIGURAÇÃO DO ROOM MANAGER ---
    roomManager.emplace(assets, gameBounds);

    // Gera 5 salas (pode ser ajustado ou lido da config)
    roomManager->generateDungeon(5);
}

void Game::setupMenu() {
    if (!playButtonTexture.loadFromFile("Images/playButton.png")) {
        throw std::runtime_error("Failed to load playButton.png");
    }
    playButton.emplace(playButtonTexture);
    playButton->setPosition({ 150.0f, 170.0f });
    playButton->setScale(sf::Vector2f(0.5f, 0.5f));

    if (!exitButtonTexture.loadFromFile("Images/exitButton.png")) {
        throw std::runtime_error("Failed to load exitButton.png");
    }
    exitButton.emplace(exitButtonTexture);
    exitButton->setPosition({ 150.0f, 750.0f });
    exitButton->setScale(sf::Vector2f(0.4900965f, 0.537958f));

    if (!menuTexture.loadFromFile("Images/backgroundMenu.png")) {
        throw std::runtime_error("Failed to load backgroundMenu.png");
    }
    menuGround.emplace(menuTexture);
    menuGround->setScale({ 1.25f, 1.05571847f });
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

void Game::processEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
    }
}

void Game::update(float deltaTime) {
    if (!Isaac || !roomManager) return;

    sf::Vector2f playerPosition = Isaac->getPosition();

    // 1. Lógica de Transição (TEM PRIORIDADE)
    if (roomManager->isTransitioning()) {
        roomManager->updateTransition(deltaTime, playerPosition);
        Isaac->setPosition(playerPosition); // Atualiza Isaac com o movimento da transição
        Isaac->setSpeedMultiplier(0.f); // Pára o player durante o fade
        return;
    }
    Isaac->setSpeedMultiplier(1.f); // Restaura velocidade

    // 2. Atualização do Player (Movimento, Tiro)
    Isaac->update(deltaTime, gameBounds);

    // 3. Atualização da Sala (Inimigos, Cura, Checagem de Limpeza)
    roomManager->update(deltaTime, Isaac->getPosition());

    // 4. Checagem de Porta
    DoorDirection doorHit = roomManager->checkPlayerAtDoor(Isaac->getGlobalBounds());
    if (doorHit != DoorDirection::None) {
        roomManager->requestTransition(doorHit);
        return; // Pula o resto da lógica (colisão) para iniciar o fade
    }

    // --- Lógica de Colisão de Projéteis ---

    auto& isaacProjectiles = Isaac->getProjectiles();
    Room* currentRoom = roomManager->getCurrentRoom();

    if (currentRoom) {
        // ⚠️ CORREÇÃO: Usar auto& para obter a REFERÊNCIA do std::optional,
        // garantindo que as chamadas a takeDamage alteram o inimigo original.
        auto& demon = currentRoom->getDemon();
        auto& bishop = currentRoom->getBishop();
        // ------------------------------------------------------------------

        // Colisão: Projéteis do Isaac vs. Inimigos
        for (auto it = isaacProjectiles.begin(); it != isaacProjectiles.end();) {
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();
            bool projectile_hit = false;

            // 1. COLISÃO COM O DEMON
            if (demon.has_value() && demon->getHealth() > 0 && checkCollision(projBounds, demon->getGlobalBounds())) {
                demon->takeDamage(1);
                projectile_hit = true;
            }

            // 2. COLISÃO COM O BISHOP
            if (bishop.has_value() && bishop->getHealth() > 0 && checkCollision(projBounds, bishop->getGlobalBounds())) {
                bishop->takeDamage(1);
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
                    Isaac->takeDamage(1);
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
    float xPosition = 200.f;
    float yPosition = 90.f;
    const int maxHearts = 3;
    const float heartSpacing = 67.f;

    if (!heartSpriteF || !heartSpriteH || !heartSpriteE) return;

    // Garante que a escala é definida
    heartSpriteF->setScale({ 3.f, 3.f });
    heartSpriteH->setScale({ 3.f, 3.f });
    heartSpriteE->setScale({ 3.f, 3.f });

    for (int i = 0; i < maxHearts; ++i) {
        const sf::Sprite* spriteToDraw;
        if (currentHealth >= 2) spriteToDraw = &*heartSpriteF;
        else if (currentHealth == 1) spriteToDraw = &*heartSpriteH;
        else spriteToDraw = &*heartSpriteE;

        // É necessário const_cast ou garantir que o objeto não é const para chamar setPosition
        const_cast<sf::Sprite*>(spriteToDraw)->setPosition({ xPosition, yPosition });
        window.draw(*spriteToDraw);
        xPosition += heartSpacing;
        currentHealth -= 2;
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