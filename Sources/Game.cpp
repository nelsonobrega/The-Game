#include "Game.hpp"
#include "Utils.hpp"
#include "ConfigManager.hpp" // Adicionado para carregar a configuração
#include <iostream>
#include <cstdlib> 
#include <ctime>   

// --- Definições de Texturas de Canto (3 Opções Aleatórias) ---
// Estas constantes são locais e não causam redefinição.
const sf::IntRect TEXTURE_OPTION_A = { {0, 0 }, { 234, 156 } };
const sf::IntRect TEXTURE_OPTION_B = { {0, 156 }, {234, 155} };
const sf::IntRect TEXTURE_OPTION_C = { {234, 0}, {234, 155} };

// --- Definições dos Recortes de Projéteis (Valores Hardcoded) ---
const sf::IntRect ISAAC_TEAR_RECT = { {8, 39}, {16, 16} };
const sf::IntRect DEMON_TEAR_RECT = { {8, 103}, {16, 16} };

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
        // ATENÇÃO: Verifique o caminho correto do seu ficheiro JSON
        ConfigManager::getInstance().loadConfig("config.json");
        std::cout << "Configuracao carregada com sucesso.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "ERRO FATAL AO CARREGAR CONFIG: " << e.what() << "\n";
        // Lança exceção para garantir que o programa pare se a config falhar
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
    }

    // --- CONFIGURAÇÃO DO DEMON ---
    Enemy.emplace(
        assets.getAnimationSet("D_Down"), assets.getAnimationSet("D_Up"),
        assets.getAnimationSet("D_Left"), assets.getAnimationSet("D_Right"),
        assets.getTexture("TearAtlas")
    );

    if (Enemy) {
        Enemy->setProjectileTextureRect(DEMON_TEAR_RECT);
        std::cout << "Demon inicializado com novo recorte de projetil.\n";
    }

    // --- CONFIGURAÇÃO DO BISHOP ---
    eBishop.emplace(assets.getAnimationSet("Bishop"));

    // Inicializa sprites opcionais para corações
    heartSpriteF.emplace(assets.getTexture("HeartF"));
    heartSpriteH.emplace(assets.getTexture("HeartH"));
    heartSpriteE.emplace(assets.getTexture("HeartE"));

    setupMenu();

    // Inicializar sprites opcionais com as texturas carregadas
    cornerTL.emplace(assets.getTexture("BasementCorner"));
    cornerTR.emplace(assets.getTexture("BasementCorner"));
    cornerBL.emplace(assets.getTexture("BasementCorner"));
    cornerBR.emplace(assets.getTexture("BasementCorner"));

    // Setup basement corners (COM ALEATORIEDADE E 3 OPÇÕES)
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
    if (Isaac) Isaac->update(deltaTime, gameBounds);
    if (Enemy) Enemy->update(deltaTime, Isaac->getPosition(), gameBounds);
    if (eBishop) eBishop->update(deltaTime, Isaac->getPosition(), gameBounds);

    if (eBishop && eBishop->shouldHealDemon()) {
        if (Enemy && Enemy->getHealth() > 0) {
            Enemy->heal(3);
            eBishop->resetHealFlag();
        }
    }

    if (Isaac) {
        auto& isaacProjectiles = Isaac->getProjectiles();

        for (auto it = isaacProjectiles.begin(); it != isaacProjectiles.end();) {
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();
            bool projectile_hit = false;

            // 1. COLISÃO COM O DEMON (Enemy) - USANDO FUNÇÃO GLOBAL checkCollision (de Utils.hpp)
            if (Enemy && Enemy->getHealth() > 0 && checkCollision(projBounds, Enemy->getGlobalBounds())) {
                Enemy->takeDamage(1);
                projectile_hit = true;
            }

            // 2. COLISÃO COM O BISHOP (eBishop) - USANDO FUNÇÃO GLOBAL checkCollision (de Utils.hpp)
            if (eBishop && eBishop->getHealth() > 0 && checkCollision(projBounds, eBishop->getGlobalBounds())) {
                eBishop->takeDamage(1);
                projectile_hit = true;
            }

            if (projectile_hit) {
                it = isaacProjectiles.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // Colisão dos projéteis inimigos (Demon) com o Player
    if (Isaac && Enemy) {
        auto& enemyProjectiles = Enemy->getProjectiles();
        for (auto it = enemyProjectiles.begin(); it != enemyProjectiles.end();) {
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();
            // USANDO FUNÇÃO GLOBAL checkCollision (de Utils.hpp)
            if (Isaac->getHealth() > 0 && checkCollision(projBounds, Isaac->getGlobalBounds())) {
                Isaac->takeDamage(1);
                it = enemyProjectiles.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    if ((Enemy && Enemy->getHealth() <= 0) && (eBishop && eBishop->getHealth() <= 0)) {
        // Lógica de fim de jogo/sala
    }

    if (Isaac && Isaac->getHealth() <= 0) {
        window.close();
    }
}

void Game::render() {
    window.clear();

    // Desenho dos Cantos da Sala
    if (cornerTL) window.draw(*cornerTL);
    if (cornerTR) window.draw(*cornerTR);
    if (cornerBL) window.draw(*cornerBL);
    if (cornerBR) window.draw(*cornerBR);


    if (!Isaac) return;

    int currentHealth = Isaac->getHealth();
    float xPosition = 200.f;
    float yPosition = 90.f;
    const int maxHearts = 3;
    const float heartSpacing = 67.f;

    if (!heartSpriteF || !heartSpriteH || !heartSpriteE) return;

    heartSpriteF->setScale({ 3.f, 3.f });
    heartSpriteH->setScale({ 3.f, 3.f });
    heartSpriteE->setScale({ 3.f, 3.f });

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

    if (Enemy) Enemy->draw(window);
    Isaac->draw(window);
    if (eBishop) eBishop->draw(window);

    window.display();
}

bool Game::isMouseOver(const sf::Sprite& sprite) {
    auto mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
    return sprite.getGlobalBounds().contains(mousePos);
}