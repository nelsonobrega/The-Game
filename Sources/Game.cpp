#include "Game.hpp"
#include "Utils.hpp"
#include "ConfigManager.hpp"
#include "Chubby.hpp"
#include "Monstro.hpp"
#include "Vis.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

void Game::loadGameAssets() {
    // Isaac & Tears
    assets.loadAnimation("I_Down", "Isaac/Front_Isaac", "F", 9, "V1.png");
    assets.loadAnimation("I_Up", "Isaac/Back_Isaac", "B", 9, "V1.png");
    assets.loadAnimation("I_Left", "Isaac/Left_Isaac", "L", 6, "V1.png");
    assets.loadAnimation("I_Right", "Isaac/Right_Isaac", "R", 6, "V1.png");
    assets.loadTexture("TearAtlas", "Images/Tears/bulletatlas.png");

    // Items
    assets.loadTexture("Speed Ball", "Images/Items/Speed Ball.png");
    assets.loadTexture("Roid Rage", "Images/Items/Roid Rage.png");
    assets.loadTexture("Blood Bag", "Images/Items/Blood Bag.png");
    assets.loadTexture("8 inch nail", "Images/Items/8inchnails_tears.png");
    assets.loadTexture("8inch_tears", "Images/Tears/8inchnails_tears.png");
    assets.loadTexture("Altar", "Images/Items/Item_altar.png");

    // Enemies
    assets.loadAnimation("D_Down", "Demon/Front_Demon", "F", 8, "D.png");
    assets.loadAnimation("D_Up", "Demon/Back_Demon", "B", 8, "D.png");
    assets.loadAnimation("D_Left", "Demon/Left_Demon", "L", 8, "D.png");
    assets.loadAnimation("D_Right", "Demon/Right_Demon", "R", 8, "D.png");
    assets.loadAnimation("Bishop", "Bishop", "B", 14, ".png");
    assets.loadTexture("ChubbySheet", "Images/Chubby/Chubby.png");
    assets.loadTexture("MonstroSheet", "Images/Monstro(BOSS)/Monstro.png");
    assets.loadTexture("VisSheet", "Images/Chubby/Chubby.png"); // Certifique-se que o caminho está correto

    // Environment & UI
    assets.loadTexture("Door", "Images/Background/Doors.png");
    assets.loadTexture("HeartF", "Images/UI/Life/Full.png");
    assets.loadTexture("HeartH", "Images/UI/Life/Half.png");
    assets.loadTexture("HeartE", "Images/UI/Life/Empty.png");
    assets.loadTexture("BasementCorner", "Images/Background/Basement_sheet.png");
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
    : window(sf::VideoMode({ 1920, 1080 }), "The Binding of Isaac: C++ Edition"),
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

    // Inicializar Isaac
    Isaac.emplace(assets.getAnimationSet("I_Down"), assets.getTexture("TearAtlas"),
        assets.getAnimationSet("I_Up"), assets.getAnimationSet("I_Left"),
        assets.getAnimationSet("I_Right"));

    if (Isaac) {
        const auto& projConfig = config.projectile_textures.isaac_tear;
        Isaac->setProjectileTextureRect(sf::IntRect({ projConfig.x, projConfig.y }, { projConfig.width, projConfig.height }));
        Isaac->setPosition({ (float)config.game.window_width / 2.f, (float)config.game.window_height / 2.f });
    }

    // UI de vida
    heartSpriteF.emplace(assets.getTexture("HeartF"));
    heartSpriteH.emplace(assets.getTexture("HeartH"));
    heartSpriteE.emplace(assets.getTexture("HeartE"));

    setupMenu();

    // Cenário (Cantos)
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

    if (showBossTitle) {
        bossTitleTimer += deltaTime;
        if (bossTitleTimer > 4.5f) showBossTitle = false;
        else return;
    }

    // Transição de Salas
    if (roomManager->isTransitioning()) {
        sf::Vector2f playerPos = Isaac->getPosition();
        Room* roomBefore = roomManager->getCurrentRoom();
        roomManager->updateTransition(deltaTime, playerPos);
        Isaac->setPosition(playerPos);
        Isaac->setSpeedMultiplier(0.f);
        if (roomManager->getCurrentRoom() != roomBefore) updateRoomVisuals();
        return;
    }

    Isaac->setSpeedMultiplier(1.f);
    Isaac->update(deltaTime, gameBounds);
    roomManager->update(deltaTime, Isaac->getPosition());

    Room* currentRoom = roomManager->getCurrentRoom();
    if (currentRoom) {
        // --- COLISÃO FÍSICA E COLETA DO ALTAR ---
        if (currentRoom->getType() == RoomType::Treasure) {
            auto& itemOpt = currentRoom->getRoomItem();
            if (itemOpt.has_value()) {
                sf::FloatRect altarBounds = itemOpt->getAltarBounds();
                sf::FloatRect isaacBounds = Isaac->getGlobalBounds();

                if (checkCollision(isaacBounds, altarBounds)) {
                    sf::Vector2f isaacPos = Isaac->getPosition();
                    float overlapLeft = (isaacBounds.position.x + isaacBounds.size.x) - altarBounds.position.x;
                    float overlapRight = (altarBounds.position.x + altarBounds.size.x) - isaacBounds.position.x;
                    float overlapTop = (isaacBounds.position.y + isaacBounds.size.y) - altarBounds.position.y;
                    float overlapBottom = (altarBounds.position.y + altarBounds.size.y) - isaacBounds.position.y;

                    float minOverlapX = std::min(overlapLeft, overlapRight);
                    float minOverlapY = std::min(overlapTop, overlapBottom);

                    if (minOverlapX < minOverlapY) {
                        if (overlapLeft < overlapRight) isaacPos.x -= overlapLeft;
                        else isaacPos.x += overlapRight;
                    }
                    else {
                        if (overlapTop < overlapBottom) isaacPos.y -= overlapTop;
                        else isaacPos.y += overlapBottom;
                    }
                    Isaac->setPosition(isaacPos);
                }

                if (!itemOpt->isCollected()) {
                    isaacBounds = Isaac->getGlobalBounds();
                    sf::Vector2f iCenter = { isaacBounds.position.x + isaacBounds.size.x / 2.f, isaacBounds.position.y + isaacBounds.size.y / 2.f };
                    sf::Vector2f aCenter = { altarBounds.position.x + altarBounds.size.x / 2.f, altarBounds.position.y + altarBounds.size.y / 2.f };
                    float dist = std::sqrt(std::pow(iCenter.x - aCenter.x, 2) + std::pow(iCenter.y - aCenter.y, 2));

                    if (dist < 95.f) {
                        ItemType type = itemOpt->getType();
                        if (type == ItemType::SPEED_BALL) Isaac->addSpeed(1.4f);
                        else if (type == ItemType::ROID_RAGE) { Isaac->addSpeed(1.2f); Isaac->addDamage(1.5f); }
                        else if (type == ItemType::BLOOD_BAG) { Isaac->increaseMaxHealth(2); Isaac->heal(4); }
                        else if (type == ItemType::EIGHT_INCH_NAIL) {
                            Isaac->addDamage(2.0f); Isaac->setEightInchNail(true);
                            Isaac->setTearTexture(assets.getTexture("8inch_tears"), sf::IntRect({ 12,12 }, { 9,7 }));
                        }
                        itemOpt->collect();
                    }
                }
            }
        }

        // --- COLISÕES: TIROS DO ISAAC -> INIMIGOS ---
        auto& tears = Isaac->getProjectiles();
        sf::FloatRect iBounds = Isaac->getGlobalBounds();

        for (auto itTear = tears.begin(); itTear != tears.end();) {
            bool hit = false;
            sf::FloatRect tBounds = itTear->sprite.getGlobalBounds();
            float dmg = itTear->damage;

            for (auto& m : currentRoom->getMonstros()) if (m->getHealth() > 0 && checkCollision(tBounds, m->getGlobalBounds())) { m->takeDamage(dmg); hit = true; break; }
            if (!hit) for (auto& d : currentRoom->getDemons()) if (d->getHealth() > 0 && checkCollision(tBounds, d->getGlobalBounds())) { d->takeDamage(dmg); hit = true; break; }
            if (!hit) for (auto& b : currentRoom->getBishops()) if (b->getHealth() > 0 && checkCollision(tBounds, b->getGlobalBounds())) { b->takeDamage(dmg); hit = true; break; }
            if (!hit) for (auto& c : currentRoom->getChubbies()) if (c->getHealth() > 0 && checkCollision(tBounds, c->getGlobalBounds())) { c->takeDamage(dmg); hit = true; break; }
            if (!hit) for (auto& v : currentRoom->getVisEnemies()) if (v->getHealth() > 0 && checkCollision(tBounds, v->getGlobalBounds())) { v->takeDamage(dmg); hit = true; break; }

            if (hit) {
                if (itTear->isNail) Isaac->incrementNailHits();
                itTear = tears.erase(itTear);
            }
            else ++itTear;
        }

        // --- COLISÕES: INIMIGOS -> ISAAC ---

        // Monstro
        for (auto& m : currentRoom->getMonstros()) {
            if (m->getHealth() <= 0) continue;
            if (checkCollision(iBounds, m->getGlobalBounds())) Isaac->takeDamage((m->getState() == MonstroState::Falling) ? 2 : 1);
            for (auto itP = m->getProjectiles().begin(); itP != m->getProjectiles().end();) {
                if (checkCollision(itP->sprite.getGlobalBounds(), iBounds)) { Isaac->takeDamage(1); itP = m->getProjectiles().erase(itP); }
                else ++itP;
            }
        }

        // Sistema Genérico de Hazard (Lasers do Vis e DoubleVis)
        for (auto& v : currentRoom->getVisEnemies()) {
            if (v->getHealth() <= 0) continue;
            // Colisão com o corpo
            if (checkCollision(iBounds, v->getGlobalBounds())) Isaac->takeDamage(2);

            // Colisão com áreas perigosas (Lasers)
            for (const auto& hazard : v->getHazardBounds()) {
                if (checkCollision(iBounds, hazard)) {
                    Isaac->takeDamage(1);
                    break; // Evita tomar dano múltiplo do mesmo laser no mesmo frame
                }
            }
        }

        // Outros Inimigos
        for (auto& d : currentRoom->getDemons()) {
            if (d->getHealth() <= 0) continue;
            if (checkCollision(iBounds, d->getGlobalBounds())) Isaac->takeDamage(1);
            for (auto itP = d->getProjectiles().begin(); itP != d->getProjectiles().end();) {
                if (checkCollision(itP->sprite.getGlobalBounds(), iBounds)) { Isaac->takeDamage(1); itP = d->getProjectiles().erase(itP); }
                else ++itP;
            }
        }
        for (auto& c : currentRoom->getChubbies()) {
            if (c->getHealth() <= 0) continue;
            if (checkCollision(iBounds, c->getGlobalBounds())) Isaac->takeDamage(1);
            if (checkCollision(iBounds, c->getBoomerangBounds())) Isaac->takeDamage(1);
        }
        for (auto& b : currentRoom->getBishops()) if (b->getHealth() > 0 && checkCollision(iBounds, b->getGlobalBounds())) Isaac->takeDamage(1);
    }

    // Portas
    DoorDirection doorHit = roomManager->checkPlayerAtDoor(Isaac->getGlobalBounds());
    if (doorHit != DoorDirection::None) {
        roomManager->requestTransition(doorHit);
    }

    if (Isaac->getHealth() <= 0) window.close();
}

void Game::render() {
    window.clear();
    const auto& config = ConfigManager::getInstance().getConfig();

    if (cornerTL) window.draw(*cornerTL);
    if (cornerTR) window.draw(*cornerTR);
    if (cornerBL) window.draw(*cornerBL);
    if (cornerBR) window.draw(*cornerBR);

    if (roomManager) roomManager->draw(window);
    if (Isaac) Isaac->draw(window);

    // UI Vida
    if (Isaac && heartSpriteF) {
        int hp = Isaac->getHealth();
        int totalMaxHP = (config.game.ui.max_hearts * 2) + Isaac->getMaxHealthBonus();
        float x = config.game.ui.heart_ui_x;
        float y = config.game.ui.heart_ui_y;

        for (int i = 0; i < totalMaxHP / 2; ++i) {
            sf::Sprite* s;
            if (hp >= 2) { s = &*heartSpriteF; hp -= 2; }
            else if (hp == 1) { s = &*heartSpriteH; hp -= 1; }
            else { s = &*heartSpriteE; }
            s->setPosition({ x, y });
            s->setScale({ config.game.ui.heart_scale, config.game.ui.heart_scale });
            window.draw(*s);
            x += config.game.ui.heart_spacing;
            if ((i + 1) % 6 == 0) { x = config.game.ui.heart_ui_x; y += 40.f; }
        }
    }

    if (roomManager) {
        roomManager->drawMiniMap(window);
        roomManager->drawTransitionOverlay(window);
    }

    if (showBossTitle) {
        window.draw(bossIntroBackground);
        if (bossTitleTimer > 2.0f && bossNameSprite) window.draw(*bossNameSprite);
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