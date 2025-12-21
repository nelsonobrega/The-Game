#include "Game.hpp"
#include "Utils.hpp"
#include "ConfigManager.hpp"
#include "Chubby.hpp"
#include "Monstro.hpp"
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

    assets.loadTexture("ChubbySheet", "Images/Chubby/Chubby.png");
    assets.loadTexture("Door", "Images/Background/Doors.png");
    assets.loadTexture("HeartF", "Images/UI/Life/Full.png");
    assets.loadTexture("HeartH", "Images/UI/Life/Half.png");
    assets.loadTexture("HeartE", "Images/UI/Life/Empty.png");
    assets.loadTexture("BasementCorner", "Images/Background/Basement_sheet.png");
    assets.loadTexture("MonstroSheet", "Images/Monstro(BOSS)/Monstro.png");
}

void Game::updateRoomVisuals() {
    Room* curr = roomManager->getCurrentRoom();
    if (!curr) return;

    sf::IntRect savedRect = curr->getCornerTextureRect();
    if (cornerTL) cornerTL->setTextureRect(savedRect);
    if (cornerTR) cornerTR->setTextureRect(savedRect);
    if (cornerBL) cornerBL->setTextureRect(savedRect);
    if (cornerBR) cornerBR->setTextureRect(savedRect);

    if (curr->getType() == RoomType::Boss && !curr->isCleared()) {
        showBossTitle = true;
        bossTitleTimer = 0.0f;

        // Configuração do Retângulo Preto para o Freeze
        bossIntroBackground.setSize({ 1920.f, 1080.f });
        bossIntroBackground.setFillColor(sf::Color::Black);

        bossNameSprite.emplace(assets.getTexture("MonstroSheet"));
        bossNameSprite->setTextureRect(sf::IntRect({ 173, 237 }, { 149, 45 }));
        bossNameSprite->setOrigin({ 149 / 2.f, 45 / 2.f });
        bossNameSprite->setPosition({ 1920 / 2.f, 1080 / 2.f });
        bossNameSprite->setScale({ 4.0f, 4.0f });
    }
}

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(1920, 1080)), "The Game - Isaac Clone"),
    currentState(GameState::menu),
    assets(AssetManager::getInstance()),
    showBossTitle(false),
    bossTitleTimer(0.f)
{
    try { ConfigManager::getInstance().loadConfig("config.json"); }
    catch (const std::exception& e) { std::cerr << "Config Error: " << e.what() << std::endl; }

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

    float scaleX = (float)config.game.window_width / 2.f / (float)config.corners.option_a.width;
    float scaleY = (float)config.game.window_height / 2.f / (float)config.corners.option_a.height;

    cornerTL->setPosition({ 0, 0 }); cornerTL->setScale({ scaleX, scaleY });
    cornerTR->setPosition({ (float)config.game.window_width, 0 }); cornerTR->setScale({ -scaleX, scaleY });
    cornerBL->setPosition({ 0, (float)config.game.window_height }); cornerBL->setScale({ scaleX, -scaleY });
    cornerBR->setPosition({ (float)config.game.window_width, (float)config.game.window_height }); cornerBR->setScale({ -scaleX, -scaleY });

    gameBounds = sf::FloatRect({ (float)config.game.bounds.left, (float)config.game.bounds.top }, { (float)config.game.bounds.width, (float)config.game.bounds.height });
    roomManager.emplace(assets, gameBounds);

    int numRooms = config.game.dungeon.min_rooms + (std::rand() % (config.game.dungeon.max_rooms - config.game.dungeon.min_rooms + 1));
    roomManager->generateDungeon(numRooms);

    updateRoomVisuals();
}

void Game::run() {
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        processEvents();

        if (currentState == GameState::menu) {
            if (playButton && isMouseOver(*playButton) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                currentState = GameState::playing;
            }
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

    // --- LÓGICA DE INTRO DO BOSS (FREEZE) ---
    if (showBossTitle) {
        bossTitleTimer += deltaTime;
        if (bossTitleTimer > 4.5f) { // 2s preto + 2.5s texto
            showBossTitle = false;
        }
        else {
            return; // Bloqueia o update do resto do jogo
        }
    }

    const auto& config = ConfigManager::getInstance().getConfig();
    sf::Vector2f playerPosition = Isaac->getPosition();

    if (roomManager->isTransitioning()) {
        Room* roomBefore = roomManager->getCurrentRoom();
        roomManager->updateTransition(deltaTime, playerPosition);
        Isaac->setPosition(playerPosition);
        Isaac->setSpeedMultiplier(0.f);
        if (roomManager->getCurrentRoom() != roomBefore) updateRoomVisuals();
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

    Room* currentRoom = roomManager->getCurrentRoom();
    if (currentRoom) {
        auto& isaacProjectiles = Isaac->getProjectiles();
        auto& demons = currentRoom->getDemons();
        auto& bishops = currentRoom->getBishops();
        auto& chubbies = currentRoom->getChubbies();
        auto& monstros = currentRoom->getMonstros();

        sf::FloatRect isaacBounds = Isaac->getGlobalBounds();

        // Colisões Isaac -> Inimigos
        for (auto itTear = isaacProjectiles.begin(); itTear != isaacProjectiles.end();) {
            bool hit = false;
            sf::FloatRect tearBounds = itTear->sprite.getGlobalBounds();

            for (auto& m : monstros) {
                if (m->getHealth() > 0 && checkCollision(tearBounds, m->getGlobalBounds())) {
                    m->takeDamage(config.player.stats.damage); hit = true; break;
                }
            }
            if (!hit) {
                for (auto& d : demons) {
                    if (d->getHealth() > 0 && checkCollision(tearBounds, d->getGlobalBounds())) {
                        d->takeDamage(config.player.stats.damage); hit = true; break;
                    }
                }
            }
            if (!hit) {
                for (auto& b : bishops) {
                    if (b->getHealth() > 0 && checkCollision(tearBounds, b->getGlobalBounds())) {
                        b->takeDamage(config.player.stats.damage); hit = true; break;
                    }
                }
            }
            if (!hit) {
                for (auto& c : chubbies) {
                    if (c->getHealth() > 0 && checkCollision(tearBounds, c->getGlobalBounds())) {
                        c->takeDamage(config.player.stats.damage); hit = true; break;
                    }
                }
            }

            if (hit) itTear = isaacProjectiles.erase(itTear);
            else ++itTear;
        }

        // Colisões Monstro -> Isaac
        for (auto& m : monstros) {
            if (m->getHealth() <= 0) continue;
            if (checkCollision(isaacBounds, m->getGlobalBounds())) {
                int dmg = (m->getState() == MonstroState::Falling) ? 2 : 1;
                Isaac->takeDamage(dmg);
            }
            auto& mProj = m->getProjectiles();
            for (auto itP = mProj.begin(); itP != mProj.end();) {
                if (checkCollision(isaacBounds, itP->sprite.getGlobalBounds())) {
                    Isaac->takeDamage(1); itP = mProj.erase(itP);
                }
                else ++itP;
            }
        }

        // Colisões Outros
        for (auto& d : demons) {
            if (d->getHealth() <= 0) continue;
            if (checkCollision(isaacBounds, d->getGlobalBounds())) { Isaac->takeDamage(1); }
            auto& dProj = d->getProjectiles();
            for (auto itP = dProj.begin(); itP != dProj.end();) {
                if (checkCollision(isaacBounds, itP->sprite.getGlobalBounds())) {
                    Isaac->takeDamage(1); itP = dProj.erase(itP);
                }
                else ++itP;
            }
        }

        for (auto& b : bishops) {
            if (b->getHealth() > 0 && checkCollision(isaacBounds, b->getGlobalBounds())) { Isaac->takeDamage(1); }
        }

        for (auto& c : chubbies) {
            if (c->getHealth() <= 0) continue;
            if (checkCollision(isaacBounds, c->getGlobalBounds())) { Isaac->takeDamage(1); }
            if (c->isBoomerangActive() && checkCollision(isaacBounds, c->getBoomerangBounds())) { Isaac->takeDamage(2); }
        }
    }

    if (Isaac->getHealth() <= 0) window.close();
}

void Game::render() {
    window.clear();
    const auto& config = ConfigManager::getInstance().getConfig();

    // Desenha o cenário
    if (cornerTL) window.draw(*cornerTL);
    if (cornerTR) window.draw(*cornerTR);
    if (cornerBL) window.draw(*cornerBL);
    if (cornerBR) window.draw(*cornerBR);

    if (roomManager) roomManager->draw(window);
    if (Isaac) Isaac->draw(window);

    // UI de Vida
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

    if (roomManager) {
        roomManager->drawMiniMap(window);
        roomManager->drawTransitionOverlay(window);
    }

    // --- RENDER DA INTRO DO BOSS ---
    if (showBossTitle) {
        // Sempre desenha o fundo preto durante a intro
        window.draw(bossIntroBackground);

        // Só desenha o texto após os 2 segundos iniciais de tela preta
        if (bossTitleTimer > 2.0f && bossNameSprite) {
            window.draw(*bossNameSprite);
        }
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