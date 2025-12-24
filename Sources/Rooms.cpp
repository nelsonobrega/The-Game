#include "Rooms.hpp"
#include "enemy.hpp"
#include "ConfigManager.hpp"
#include "AssetManager.hpp"
#include "Monstro.hpp"
#include "Vis.hpp"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>

Room::Room(int id, RoomType type, const sf::FloatRect& gameBounds)
    : roomID(id), type(type), gameBounds(gameBounds)
    , cleared(type == RoomType::SafeZone), doorsOpened(type == RoomType::SafeZone) {
}

void Room::addDoor(DoorDirection direction, DoorType doorType, sf::Texture& doorSpritesheet) {
    Door door; door.direction = direction; door.type = doorType;
    const auto& config = ConfigManager::getInstance().getConfig();
    const auto* doorConfig = &config.game.door_normal;
    if (doorType == DoorType::Boss) doorConfig = &config.game.door_boss;
    else if (doorType == DoorType::Treasure) doorConfig = &config.game.door_treasure;

    door.sprite.emplace(doorSpritesheet);
    door.sprite->setTextureRect({ {doorConfig->frame_start_x, doorConfig->frame_start_y}, {doorConfig->texture_width, doorConfig->texture_height} });
    door.sprite->setOrigin({ (float)doorConfig->origin_x, (float)doorConfig->origin_y });
    door.sprite->setScale({ doorConfig->scale_x, doorConfig->scale_y });

    door.leftHalf.emplace(doorSpritesheet);
    door.leftHalfOriginalRect = { {doorConfig->left_half_x, doorConfig->left_half_y}, {doorConfig->left_half_width, doorConfig->left_half_height} };
    door.leftHalf->setTextureRect(door.leftHalfOriginalRect);
    door.leftHalf->setOrigin({ (float)doorConfig->left_half_width, doorConfig->left_half_height / 2.0f });
    door.leftHalf->setScale({ doorConfig->scale_x, doorConfig->scale_y });

    door.rightHalf.emplace(doorSpritesheet);
    door.rightHalfOriginalRect = { {doorConfig->right_half_x, doorConfig->right_half_y}, {doorConfig->right_half_width, doorConfig->right_half_height} };
    door.rightHalf->setTextureRect(door.rightHalfOriginalRect);
    door.rightHalf->setOrigin({ 0.0f, doorConfig->right_half_height / 2.0f });
    door.rightHalf->setScale({ doorConfig->scale_x, doorConfig->scale_y });

    door.overlaySprite.emplace(doorSpritesheet);
    sf::IntRect overlayRect = (doorType == DoorType::Treasure) ? sf::IntRect({ 188, 0 }, { 49, 38 }) : (doorType == DoorType::Boss) ? sf::IntRect({ 318, 0 }, { 49, 43 }) : sf::IntRect({ 65, 0 }, { 49, 33 });
    door.overlaySprite->setTextureRect(overlayRect);
    door.overlaySprite->setOrigin(door.sprite->getOrigin());
    door.overlaySprite->setScale(door.sprite->getScale());

    if (type == RoomType::SafeZone || type == RoomType::Treasure || type == RoomType::Boss) { door.isOpen = true; door.state = DoorState::Open; door.animationProgress = 1.0f; }
    else { door.isOpen = false; door.state = DoorState::Closed; door.animationProgress = 0.0f; }

    sf::Vector2f pos = getDoorPosition(direction);
    if (type == RoomType::Boss) pos.y += 2.0f;
    door.sprite->setPosition(pos); door.overlaySprite->setPosition(pos);

    sf::Vector2f hOff(0.f, 0.f);
    if (direction == DoorDirection::North) hOff.y = -50.f; else if (direction == DoorDirection::South) hOff.y = 50.f;
    else if (direction == DoorDirection::East) hOff.x = 50.f; else if (direction == DoorDirection::West) hOff.x = -50.f;
    door.leftHalf->setPosition(pos + hOff); door.rightHalf->setPosition(pos + hOff);

    float rot = getDoorRotation(direction);
    door.sprite->setRotation(sf::degrees(rot)); door.overlaySprite->setRotation(sf::degrees(rot));
    door.leftHalf->setRotation(sf::degrees(rot)); door.rightHalf->setRotation(sf::degrees(rot));
    door.bounds = door.sprite->getGlobalBounds();
    doors.push_back(door);
}

// ATUALIZADO: Adicionado parâmetro visSheet
void Room::spawnEnemies(
    std::vector<sf::Texture>& dDown,
    std::vector<sf::Texture>& dUp,
    std::vector<sf::Texture>& dLeft,
    std::vector<sf::Texture>& dRight,
    sf::Texture& dProj,
    std::vector<sf::Texture>& bTex,
    sf::Texture& cSheet,
    sf::Texture& cProj,
    sf::Texture& visSheet
) {
    if (type == RoomType::SafeZone || type == RoomType::Treasure || cleared) return;
    if (!demons.empty() || !bishops.empty() || !chubbies.empty() || !monstros.empty() || !visEnemies.empty()) return;

    const auto& config = ConfigManager::getInstance().getConfig();
    auto getRandomPos = [&]() -> sf::Vector2f {
        float margin = 200.f;
        float rx = margin + static_cast<float>(rand() % static_cast<int>(gameBounds.size.x - margin * 2));
        float ry = margin + static_cast<float>(rand() % static_cast<int>(gameBounds.size.y - margin * 2));
        return { gameBounds.position.x + rx, gameBounds.position.y + ry };
        };

    if (type == RoomType::Boss) {
        monstros.push_back(std::make_unique<Monstro>(
            AssetManager::getInstance().getTexture("MonstroSheet"),
            dProj,
            sf::Vector2f(gameBounds.position.x + gameBounds.size.x / 2.f, gameBounds.position.y + gameBounds.size.y / 2.f)
        ));
    }
    else if (type == RoomType::Normal) {
        int r = rand() % 100;

        // 35% Chance de Chubbies
        if (r < 35) {
            int count = 3 + rand() % 3;
            for (int i = 0; i < count; i++) {
                auto c = std::make_unique<Chubby>(cSheet, cProj);
                c->setPosition(getRandomPos());
                chubbies.push_back(std::move(c));
            }
        }
        // 35% Chance de Demons (35 a 69)
        else if (r < 70) {
            auto d = std::make_unique<Demon_ALL>(dDown, dUp, dLeft, dRight, dProj, getRandomPos());
            d->setProjectileTextureRect({ {config.projectile_textures.demon_tear.x, config.projectile_textures.demon_tear.y}, {config.projectile_textures.demon_tear.width, config.projectile_textures.demon_tear.height} });
            demons.push_back(std::move(d));
        }
        // 30% Chance de Vis (70 a 99)
        else {
            int count = 2 + rand() % 2; // Spawna 2 ou 3 Vis
            for (int i = 0; i < count; i++) {
                auto v = std::make_unique<Vis>(visSheet);
                v->setPosition(getRandomPos());
                visEnemies.push_back(std::move(v));
            }
        }

        // Bishop tem chance independente de aparecer (exceto em sala de Vis para não ficar muito caótico)
        if ((rand() % 100) < config.bishop.stats.spawn_chance_percent && r < 70) {
            bishops.push_back(std::make_unique<Bishop_ALL>(bTex, getRandomPos()));
        }
    }
}

void Room::update(float dt, sf::Vector2f pPos) {
    for (auto& d : demons) if (d->getHealth() > 0) d->update(dt, pPos, gameBounds);
    for (auto& b : bishops) if (b->getHealth() > 0) b->update(dt, pPos, gameBounds);
    for (auto& c : chubbies) if (c->getHealth() > 0) c->update(dt, pPos, gameBounds);
    for (auto& m : monstros) if (m->getHealth() > 0) m->update(dt, pPos, gameBounds);
    for (auto& v : visEnemies) if (v->getHealth() > 0) v->update(dt, pPos, gameBounds); // NOVO

    updateDoorAnimations(dt);
    checkIfCleared();
}

void Room::draw(sf::RenderWindow& window) {
    for (const auto& door : doors) drawDoor(window, door);

    if (roomItem.has_value()) {
        roomItem->draw(window);
    }

    // Desenhar Inimigos
    for (auto& d : demons) if (d->getHealth() > 0) d->draw(window);
    for (auto& b : bishops) if (b->getHealth() > 0) b->draw(window);
    for (auto& c : chubbies) if (c->getHealth() > 0) c->draw(window);
    for (auto& m : monstros) if (m->getHealth() > 0) m->draw(window);
    for (auto& v : visEnemies) if (v->getHealth() > 0) v->draw(window); // NOVO
}

void Room::drawDoor(sf::RenderWindow& window, const Door& door) const {
    if (!door.sprite || !door.overlaySprite) return;
    window.draw(*door.sprite);
    if (door.state != DoorState::Open && door.leftHalf && door.rightHalf) {
        float maxW = (float)door.leftHalfOriginalRect.size.x;
        float off = maxW * door.leftHalf->getScale().x * door.animationProgress;
        int visW = std::max(0, (int)(maxW * (1.0f - door.animationProgress)));
        sf::Sprite lC = *door.leftHalf; sf::Sprite rC = *door.rightHalf;
        float rot = lC.getRotation().asDegrees();
        if (std::abs(rot) < 1.f) { lC.move({ -off, 0 }); rC.move({ off, 0 }); }
        else if (std::abs(rot - 180.f) < 1.f) { lC.move({ off, 0 }); rC.move({ -off, 0 }); }
        else if (std::abs(rot - 90.f) < 1.f) { lC.move({ 0, -off }); rC.move({ 0, off }); }
        else if (std::abs(rot + 90.f) < 1.f) { lC.move({ 0, off }); rC.move({ 0, -off }); }
        if (visW > 0) {
            sf::IntRect lr = door.leftHalfOriginalRect; lr.size.x = visW; lC.setTextureRect(lr); window.draw(lC);
            sf::IntRect rr = door.rightHalfOriginalRect; rr.size.x = visW; rC.setTextureRect(rr); window.draw(rC);
        }
    }
    window.draw(*door.overlaySprite);
}

void Room::checkIfCleared() {
    if (cleared) return;
    auto dead = [](const auto& v) { return std::all_of(v.begin(), v.end(), [](const auto& e) { return e->getHealth() <= 0; }); };

    // Atualizado para checar Vis
    if (dead(demons) && dead(bishops) && dead(chubbies) && dead(monstros) && dead(visEnemies)) {
        cleared = true;
        openDoors();
    }
}

void Room::openDoors() { for (auto& d : doors) d.state = DoorState::Opening; doorsOpened = true; }
void Room::closeDoors() { if (!cleared) { for (auto& d : doors) if (d.type == DoorType::Normal) d.state = DoorState::Closing; doorsOpened = false; } }

void Room::updateDoorAnimations(float dt) {
    float speed = 1.0f / ConfigManager::getInstance().getConfig().game.dungeon.door_animation_duration;
    for (auto& d : doors) {
        if (d.state == DoorState::Closing) { d.animationProgress -= speed * dt; if (d.animationProgress <= 0.f) { d.animationProgress = 0.f; d.state = DoorState::Closed; d.isOpen = false; } }
        else if (d.state == DoorState::Opening) { d.animationProgress += speed * dt; if (d.animationProgress >= 1.f) { d.animationProgress = 1.f; d.state = DoorState::Open; d.isOpen = true; } }
    }
}

sf::Vector2f Room::getDoorPosition(DoorDirection dir) const {
    float cx = gameBounds.position.x + gameBounds.size.x / 2.f, cy = gameBounds.position.y + gameBounds.size.y / 2.f;
    float off = ConfigManager::getInstance().getConfig().game.dungeon.door_offset - 5.0f;
    if (dir == DoorDirection::North) return { cx, gameBounds.position.y + off + 8.0f };
    if (dir == DoorDirection::South) return { cx, gameBounds.position.y + gameBounds.size.y - off };
    if (dir == DoorDirection::East)  return { gameBounds.position.x + gameBounds.size.x - off, cy };
    if (dir == DoorDirection::West)  return { gameBounds.position.x + off + 8.0f, cy };
    return { cx, cy };
}

float Room::getDoorRotation(DoorDirection dir) const {
    if (dir == DoorDirection::South) return 180.f; if (dir == DoorDirection::East) return 90.f; if (dir == DoorDirection::West) return -90.f; return 0.f;
}

void Room::connectDoor(DoorDirection dir, int id) { for (auto& d : doors) if (d.direction == dir) d.leadsToRoomID = id; }
bool Room::hasDoor(DoorDirection dir) const { for (const auto& d : doors) if (d.direction == dir) return true; return false; }
int Room::getDoorLeadsTo(DoorDirection dir) const { for (const auto& d : doors) if (d.direction == dir) return d.leadsToRoomID; return -1; }

sf::Vector2f Room::getPlayerSpawnPosition(DoorDirection dir) const {
    sf::Vector2f p = getDoorPosition(dir); float off = 100.0f;
    if (dir == DoorDirection::North) return { p.x, p.y + off }; if (dir == DoorDirection::South) return { p.x, p.y - off };
    if (dir == DoorDirection::East)  return { p.x - off, p.y }; if (dir == DoorDirection::West)  return { p.x + off, p.y };
    return { gameBounds.position.x + gameBounds.size.x / 2.f, gameBounds.position.y + gameBounds.size.y / 2.f };
}