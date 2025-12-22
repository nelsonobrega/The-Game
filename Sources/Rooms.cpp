#include "Rooms.hpp"
#include "enemy.hpp"
#include "ConfigManager.hpp"
#include "AssetManager.hpp"
#include "Monstro.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

Room::Room(int id, RoomType type, const sf::FloatRect& gameBounds)
    : roomID(id)
    , type(type)
    , gameBounds(gameBounds)
    , cleared(type == RoomType::SafeZone)
    , doorsOpened(type == RoomType::SafeZone)
{
}

void Room::addDoor(DoorDirection direction, DoorType doorType, sf::Texture& doorSpritesheet) {
    Door door;
    door.direction = direction;
    door.type = doorType;

    const auto& config = ConfigManager::getInstance().getConfig();
    const auto* doorConfig = &config.game.door_normal;
    const float bossOffsetY = (type == RoomType::Boss) ? 2.0f : 0.0f;

    if (doorType == DoorType::Boss) doorConfig = &config.game.door_boss;
    else if (doorType == DoorType::Treasure) doorConfig = &config.game.door_treasure;

    door.sprite.emplace(doorSpritesheet);
    sf::IntRect textureRect({ doorConfig->frame_start_x, doorConfig->frame_start_y },
        { doorConfig->texture_width, doorConfig->texture_height });
    door.sprite->setTextureRect(textureRect);
    door.sprite->setOrigin({ (float)doorConfig->origin_x, (float)doorConfig->origin_y });
    door.sprite->setScale({ doorConfig->scale_x, doorConfig->scale_y });

    door.leftHalf.emplace(doorSpritesheet);
    sf::IntRect leftRect({ doorConfig->left_half_x, doorConfig->left_half_y },
        { doorConfig->left_half_width, doorConfig->left_half_height });
    door.leftHalf->setTextureRect(leftRect);
    door.leftHalf->setOrigin({ (float)doorConfig->left_half_width, doorConfig->left_half_height / 2.0f });
    door.leftHalf->setScale({ doorConfig->scale_x, doorConfig->scale_y });
    door.leftHalfOriginalRect = leftRect;

    door.rightHalf.emplace(doorSpritesheet);
    sf::IntRect rightRect({ doorConfig->right_half_x, doorConfig->right_half_y },
        { doorConfig->right_half_width, doorConfig->right_half_height });
    door.rightHalf->setTextureRect(rightRect);
    door.rightHalf->setOrigin({ 0.0f, doorConfig->right_half_height / 2.0f });
    door.rightHalf->setScale({ doorConfig->scale_x, doorConfig->scale_y });
    door.rightHalfOriginalRect = rightRect;

    door.overlaySprite.emplace(doorSpritesheet);
    sf::IntRect overlayRect = (doorType == DoorType::Treasure) ? sf::IntRect({ 188, 0 }, { 49, 38 }) :
        (doorType == DoorType::Boss) ? sf::IntRect({ 318, 0 }, { 49, 43 }) :
        sf::IntRect({ 65, 0 }, { 49, 33 });
    door.overlaySprite->setTextureRect(overlayRect);
    door.overlaySprite->setOrigin(door.sprite->getOrigin());
    door.overlaySprite->setScale(door.sprite->getScale());

    if (this->type == RoomType::SafeZone || this->type == RoomType::Treasure || this->type == RoomType::Boss) {
        door.isOpen = true; door.state = DoorState::Open; door.animationProgress = 1.0f;
    }
    else {
        door.isOpen = false; door.state = DoorState::Closed; door.animationProgress = 0.0f;
    }

    sf::Vector2f position = getDoorPosition(direction);
    position.y += bossOffsetY;
    door.sprite->setPosition(position);
    door.overlaySprite->setPosition(position);

    sf::Vector2f halvesOffset(0.f, 0.f);
    if (direction == DoorDirection::North) halvesOffset.y = -50.f;
    else if (direction == DoorDirection::South) halvesOffset.y = 50.f;
    else if (direction == DoorDirection::East)  halvesOffset.x = 50.f;
    else if (direction == DoorDirection::West)  halvesOffset.x = -50.f;
    halvesOffset.y += bossOffsetY;

    door.leftHalf->setPosition(position + halvesOffset);
    door.rightHalf->setPosition(position + halvesOffset);

    float rotation = getDoorRotation(direction);
    door.sprite->setRotation(sf::degrees(rotation));
    door.overlaySprite->setRotation(sf::degrees(rotation));
    door.leftHalf->setRotation(sf::degrees(rotation));
    door.rightHalf->setRotation(sf::degrees(rotation));

    door.bounds = door.sprite->getGlobalBounds();
    doors.push_back(door);
}

void Room::spawnEnemies(std::vector<sf::Texture>& dDown, std::vector<sf::Texture>& dUp,
    std::vector<sf::Texture>& dLeft, std::vector<sf::Texture>& dRight,
    sf::Texture& dProj, std::vector<sf::Texture>& bTex,
    sf::Texture& cSheet, sf::Texture& cProj) {

    if (type == RoomType::SafeZone || type == RoomType::Treasure || cleared) return;
    if (!demons.empty() || !bishops.empty() || !chubbies.empty() || !monstros.empty()) return;

    const auto& config = ConfigManager::getInstance().getConfig();
    const auto& dConfigTear = config.projectile_textures.demon_tear;
    sf::IntRect demonTearRect({ dConfigTear.x, dConfigTear.y }, { dConfigTear.width, dConfigTear.height });

    // Função auxiliar para gerar posição aleatória segura dentro da sala
    auto getRandomPos = [&]() -> sf::Vector2f {
        float margin = 200.f; // Margem para não nascer colado na parede/porta
        float rx = margin + static_cast<float>(rand() % static_cast<int>(gameBounds.size.x - margin * 2));
        float ry = margin + static_cast<float>(rand() % static_cast<int>(gameBounds.size.y - margin * 2));
        return { gameBounds.position.x + rx, gameBounds.position.y + ry };
        };

    if (type == RoomType::Boss) {
        sf::Texture& monstroTex = AssetManager::getInstance().getTexture("MonstroSheet");
        sf::Vector2f centerPos = {
            gameBounds.position.x + gameBounds.size.x / 2.f,
            gameBounds.position.y + gameBounds.size.y / 2.f
        };
        auto boss = std::make_unique<Monstro>(monstroTex, dProj, centerPos);
        monstros.push_back(std::move(boss));
    }
    else if (type == RoomType::Normal) {
        int enemyLogic = rand() % 100;

        if (enemyLogic < 40) { // 40% Sala de Chubbies
            int count = 2 + (rand() % 2);
            for (int i = 0; i < count; i++) {
                auto chubby = std::make_unique<Chubby>(cSheet, cProj);
                chubby->setPosition(getRandomPos());
                chubbies.push_back(std::move(chubby));
            }
        }
        else { // 60% Sala de Demons
            // Agora passando a posição aleatória no construtor
            auto demon = std::make_unique<Demon_ALL>(dDown, dUp, dLeft, dRight, dProj, getRandomPos());
            demon->setProjectileTextureRect(demonTearRect);
            demons.push_back(std::move(demon));
        }

        // --- SPAWN DO BISHOP (Usando a chance da Config) ---
        // Agora o Bishop não é mais obrigatório, ele segue a % do JSON
        if ((rand() % 100) < config.bishop.stats.spawn_chance_percent) {
            bishops.push_back(std::make_unique<Bishop_ALL>(bTex, getRandomPos()));
        }
    }
}

void Room::update(float deltaTime, sf::Vector2f playerPosition) {
    for (auto& d : demons) if (d->getHealth() > 0) d->update(deltaTime, playerPosition, gameBounds);
    for (auto& b : bishops) if (b->getHealth() > 0) b->update(deltaTime, playerPosition, gameBounds);
    for (auto& c : chubbies) if (c->getHealth() > 0) c->update(deltaTime, playerPosition, gameBounds);
    for (auto& m : monstros) if (m->getHealth() > 0) m->update(deltaTime, playerPosition, gameBounds);

    updateDoorAnimations(deltaTime);
    checkIfCleared();
}

void Room::draw(sf::RenderWindow& window) {
    for (const auto& door : doors) drawDoor(window, door);

    for (auto& d : demons) if (d->getHealth() > 0) d->draw(window);
    for (auto& b : bishops) if (b->getHealth() > 0) b->draw(window);
    for (auto& c : chubbies) if (c->getHealth() > 0) c->draw(window);
    for (auto& m : monstros) if (m->getHealth() > 0) m->draw(window);
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
            sf::IntRect lr = door.leftHalfOriginalRect; lr.size.x = visW;
            lC.setTextureRect(lr); window.draw(lC);
            sf::IntRect rr = door.rightHalfOriginalRect; rr.size.x = visW;
            rC.setTextureRect(rr); window.draw(rC);
        }
    }
    window.draw(*door.overlaySprite);
}

void Room::checkIfCleared() {
    if (cleared) return;
    auto dead = [](const auto& v) { return std::all_of(v.begin(), v.end(), [](const auto& e) { return e->getHealth() <= 0; }); };

    if (dead(demons) && dead(bishops) && dead(chubbies) && dead(monstros)) {
        cleared = true;
        openDoors();
    }
}

void Room::openDoors() { for (auto& d : doors) d.state = DoorState::Opening; doorsOpened = true; }
void Room::closeDoors() { if (!cleared) { for (auto& d : doors) if (d.type == DoorType::Normal) d.state = DoorState::Closing; doorsOpened = false; } }

void Room::updateDoorAnimations(float deltaTime) {
    float speed = 1.0f / ConfigManager::getInstance().getConfig().game.dungeon.door_animation_duration;
    for (auto& d : doors) {
        if (d.state == DoorState::Closing) {
            d.animationProgress -= speed * deltaTime;
            if (d.animationProgress <= 0.f) { d.animationProgress = 0.f; d.state = DoorState::Closed; d.isOpen = false; }
        }
        else if (d.state == DoorState::Opening) {
            d.animationProgress += speed * deltaTime;
            if (d.animationProgress >= 1.f) { d.animationProgress = 1.f; d.state = DoorState::Open; d.isOpen = true; }
        }
    }
}

sf::Vector2f Room::getDoorPosition(DoorDirection dir) const {
    float cx = gameBounds.position.x + gameBounds.size.x / 2.f;
    float cy = gameBounds.position.y + gameBounds.size.y / 2.f;
    float off = ConfigManager::getInstance().getConfig().game.dungeon.door_offset - 5.0f;

    if (dir == DoorDirection::North) return { cx, gameBounds.position.y + off + 8.0f };
    if (dir == DoorDirection::South) return { cx, gameBounds.position.y + gameBounds.size.y - off };
    if (dir == DoorDirection::East)  return { gameBounds.position.x + gameBounds.size.x - off, cy };
    if (dir == DoorDirection::West)  return { gameBounds.position.x + off + 8.0f, cy };
    return { cx, cy };
}

float Room::getDoorRotation(DoorDirection dir) const {
    if (dir == DoorDirection::South) return 180.f;
    if (dir == DoorDirection::East) return 90.f;
    if (dir == DoorDirection::West) return -90.f;
    return 0.f;
}

void Room::connectDoor(DoorDirection dir, int id) { for (auto& d : doors) if (d.direction == dir) d.leadsToRoomID = id; }
bool Room::hasDoor(DoorDirection dir) const { for (const auto& d : doors) if (d.direction == dir) return true; return false; }
int Room::getDoorLeadsTo(DoorDirection dir) const { for (const auto& d : doors) if (d.direction == dir) return d.leadsToRoomID; return -1; }

sf::Vector2f Room::getPlayerSpawnPosition(DoorDirection dir) const {
    sf::Vector2f p = getDoorPosition(dir);
    float off = 100.0f;
    if (dir == DoorDirection::North) return { p.x, p.y + off };
    if (dir == DoorDirection::South) return { p.x, p.y - off };
    if (dir == DoorDirection::East)  return { p.x - off, p.y };
    if (dir == DoorDirection::West)  return { p.x + off, p.y };
    return { gameBounds.position.x + gameBounds.size.x / 2.f, gameBounds.position.y + gameBounds.size.y / 2.f };
}