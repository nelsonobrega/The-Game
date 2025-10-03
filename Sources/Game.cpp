#include "Game.hpp"
#include <iostream>

void Game::loadGameAssets() {
    assets.loadAnimation("I_Down", "Front_Isaac", "F", 9, "V1.png");
    assets.loadAnimation("I_Up", "Back_Isaac", "B", 9, "V1.png");
    assets.loadAnimation("I_Left", "Left_Isaac", "L", 6, "V1.png");
    assets.loadAnimation("I_Right", "Right_Isaac", "R", 6, "V1.png");
    assets.loadTexture("Hit", "Images/Hit.png");

    assets.loadAnimation("D_Down", "Front_Demon", "F", 8, "D.png");
    assets.loadAnimation("D_Up", "Back_Demon", "B", 8, "D.png");
    assets.loadAnimation("D_Left", "Left_Demon", "L", 8, "D.png");
    assets.loadAnimation("D_Right", "Right_Demon", "R", 8, "D.png");
    assets.loadTexture("EnemyHit", "Images/Enemy_Knife.png");

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
    loadGameAssets();

    // Inicializa os objetos opcionais com emplace para evitar erros de construtor padrão e operador de atribuição
    Isaac.emplace(assets.getAnimationSet("I_Down"), assets.getTexture("Hit"));
    Enemy.emplace(assets.getAnimationSet("D_Down"), assets.getAnimationSet("D_Up"),
        assets.getAnimationSet("D_Left"), assets.getAnimationSet("D_Right"),
        assets.getTexture("EnemyHit"));
    eBishop.emplace(assets.getAnimationSet("Bishop"));

    // Inicializa sprites opcionais para corações
    heartSpriteF.emplace(assets.getTexture("HeartF"));
    heartSpriteH.emplace(assets.getTexture("HeartH"));
    heartSpriteE.emplace(assets.getTexture("HeartE"));

    setupMenu();

    // Inicializar sprites opcionais com las texturas cargadas
    cornerTL.emplace(assets.getTexture("BasementCorner"));
    cornerTR.emplace(assets.getTexture("BasementCorner"));
    cornerBL.emplace(assets.getTexture("BasementCorner"));
    cornerBR.emplace(assets.getTexture("BasementCorner"));

    // Setup basement corners
    float scaleX = 960.f / 234.f;
    float scaleY = 540.f / 156.f;

    cornerTL->setTextureRect(sf::IntRect({ 0, 156 }, { 234, 311 }));
    cornerTL->setPosition({ 0.f, 0.f });
    cornerTL->setScale({ scaleX, scaleY });

    cornerTR->setTextureRect(sf::IntRect({ 0, 156 }, { 234, 311 }));
    cornerTR->setPosition({ 1920.f, 0.f });
    cornerTR->setScale({ -scaleX, scaleY });

    cornerBL->setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
    cornerBL->setPosition({ 0.f, 1080.f });
    cornerBL->setScale({ scaleX, -scaleY - 0.1f });

    cornerBR->setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
    cornerBR->setPosition({ 1920.f, 1080.f });
    cornerBR->setScale({ -scaleX, -scaleY - 0.1f });

    // Define game bounds
    float left = 213.33f;
    float top = 179.80f;
    float width = 1493.34f;
    float height = 720.40f;
    gameBounds = sf::FloatRect({ left, top }, { width, height });

    // Set textures for Isaac animations (atualiza ponteiros para animações)
    Isaac->textures_walk_up = &assets.getAnimationSet("I_Up");
    Isaac->textures_walk_left = &assets.getAnimationSet("I_Left");
    Isaac->textures_walk_right = &assets.getAnimationSet("I_Right");
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
            Enemy->heal(2);
            eBishop->resetHealFlag();
        }
    }

    if (Isaac && Enemy) {
        auto& isaacProjectiles = Isaac->getProjectiles();
        for (auto it = isaacProjectiles.begin(); it != isaacProjectiles.end();) {
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();
            if (Enemy->getHealth() > 0 && checkCollision(projBounds, Enemy->getGlobalBounds())) {
                Enemy->takeDamage(1);
                it = isaacProjectiles.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    if (Isaac && Enemy) {
        auto& enemyProjectiles = Enemy->getProjectiles();
        for (auto it = enemyProjectiles.begin(); it != enemyProjectiles.end();) {
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();
            if (Isaac->getHealth() > 0 && checkCollision(projBounds, Isaac->getGlobalBounds())) {
                Isaac->takeDamage(2);
                it = enemyProjectiles.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    if ((Enemy && Enemy->getHealth() <= 0) || (Isaac && Isaac->getHealth() <= 0)) {
        window.close();
    }
}

void Game::render() {
    window.clear();

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