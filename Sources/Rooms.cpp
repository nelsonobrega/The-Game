#include "Rooms.hpp"
#include "enemy.hpp" 
#include "ConfigManager.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>

// Construtor
Room::Room(int id, RoomType type, const sf::FloatRect& gameBounds)
    : roomID(id)
    , type(type)
    , gameBounds(gameBounds)
    , cleared(type == RoomType::SafeZone)
    , doorsOpened(type == RoomType::SafeZone)
{
}

// MODIFICADO: Adiciona uma porta à sala usando o spritesheet correto
void Room::addDoor(DoorDirection direction, DoorType doorType, sf::Texture& doorSpritesheet) {
    Door door;
    door.direction = direction;
    door.type = doorType;

    // Inicializa o sprite com o spritesheet
    door.sprite.emplace(doorSpritesheet);

    const auto& config = ConfigManager::getInstance().getConfig();
    const auto* doorConfig = &config.game.door_normal;

    // Seleciona a configuração correta baseada no tipo
    if (doorType == DoorType::Boss) {
        doorConfig = &config.game.door_boss;
    }
    else if (doorType == DoorType::Treasure) {
        doorConfig = &config.game.door_treasure;
    }

    // Define o rect correto no spritesheet
    sf::IntRect textureRect;

    if (doorType == DoorType::Boss || doorType == DoorType::Treasure) {
        // Boss e Treasure usam frame_start_x e frame_start_y
        textureRect = sf::IntRect({ doorConfig->frame_start_x,doorConfig->frame_start_y }, { doorConfig->texture_width, doorConfig->texture_height });
    }
    else {
        // Normal usa posição (0, 0)
        textureRect = sf::IntRect({ 0,0 }, { doorConfig->texture_width, doorConfig->texture_height });
    }

    door.sprite->setTextureRect(textureRect);
    door.sprite->setOrigin({ doorConfig->origin_x, doorConfig->origin_y });
    door.sprite->setScale({ doorConfig->scale_x, doorConfig->scale_y });

    door.isOpen = (this->type == RoomType::SafeZone);
    door.leadsToRoomID = -1;

    // Define posição baseada na direção
    sf::Vector2f position = getDoorPosition(direction);
    door.sprite->setPosition(position);

    // Rotação baseada na direção
    float rotation = getDoorRotation(direction);
    door.sprite->setRotation(sf::degrees(rotation));

    // Define bounds da porta (para colisão com player)
    door.bounds = door.sprite->getGlobalBounds();

    doors.push_back(door);
}

// Conecta uma porta a outra sala
void Room::connectDoor(DoorDirection direction, int targetRoomID) {
    for (auto& door : doors) {
        if (door.direction == direction) {
            door.leadsToRoomID = targetRoomID;
            return;
        }
    }
}

// Retorna posição da porta baseada na direção
sf::Vector2f Room::getDoorPosition(DoorDirection direction) const {
    float centerX = gameBounds.position.x + gameBounds.size.x / 2.f;
    float centerY = gameBounds.position.y + gameBounds.size.y / 2.f;

    const float doorOffset = ConfigManager::getInstance().getConfig().game.dungeon.door_offset;

    switch (direction) {
    case DoorDirection::North:
        return { centerX, gameBounds.position.y + doorOffset };
    case DoorDirection::South:
        return { centerX, gameBounds.position.y + gameBounds.size.y - doorOffset };
    case DoorDirection::East:
        return { gameBounds.position.x + gameBounds.size.x - doorOffset, centerY };
    case DoorDirection::West:
        return { gameBounds.position.x + doorOffset, centerY };
    default:
        return { centerX, centerY };
    }
}

// Retorna rotação da porta baseada na direção
float Room::getDoorRotation(DoorDirection direction) const {
    switch (direction) {
    case DoorDirection::North:
        return 0.f;
    case DoorDirection::South:
        return 180.f;
    case DoorDirection::East:
        return 90.f;
    case DoorDirection::West:
        return -90.f;
    default:
        return 0.f;
    }
}

// Spawna inimigos na sala
void Room::spawnEnemies(
    std::vector<sf::Texture>& demonWalkDown,
    std::vector<sf::Texture>& demonWalkUp,
    std::vector<sf::Texture>& demonWalkLeft,
    std::vector<sf::Texture>& demonWalkRight,
    sf::Texture& demonProjectileTexture,
    std::vector<sf::Texture>& bishopTextures)
{
    if (type == RoomType::SafeZone || type == RoomType::Treasure) {
        return;
    }

    if (cleared) {
        return;
    }

    if (demon.has_value() || bishop.has_value()) {
        return;
    }

    if (type == RoomType::Normal) {
        demon.emplace(
            demonWalkDown,
            demonWalkUp,
            demonWalkLeft,
            demonWalkRight,
            demonProjectileTexture
        );

        const auto& demonTearConfig = ConfigManager::getInstance().getConfig().projectile_textures.demon_tear;
        const sf::IntRect DEMON_TEAR_RECT = { {demonTearConfig.x, demonTearConfig.y}, {demonTearConfig.width, demonTearConfig.height} };
        demon->setProjectileTextureRect(DEMON_TEAR_RECT);

        std::cout << "Demon spawned in room " << roomID << std::endl;

        const auto& bishopConfig = ConfigManager::getInstance().getConfig().bishop.stats;
        int random = rand() % 100;
        if (random < bishopConfig.spawn_chance_percent) {
            bishop.emplace(bishopTextures);
            std::cout << "Bishop spawned in room " << roomID << " (" << bishopConfig.spawn_chance_percent << "% chance)" << std::endl;
        }
        else {
            std::cout << "Bishop NOT spawned in room " << roomID << " (rolled " << random << "%)" << std::endl;
        }
    }
    else if (type == RoomType::Boss) {
        demon.emplace(
            demonWalkDown,
            demonWalkUp,
            demonWalkLeft,
            demonWalkRight,
            demonProjectileTexture
        );

        const auto& demonConfig = ConfigManager::getInstance().getConfig().demon.stats;
        demon->setHealth(demonConfig.max_health);

        const auto& demonTearConfig = ConfigManager::getInstance().getConfig().projectile_textures.demon_tear;
        const sf::IntRect DEMON_TEAR_RECT = { {demonTearConfig.x, demonTearConfig.y}, {demonTearConfig.width, demonTearConfig.height} };
        demon->setProjectileTextureRect(DEMON_TEAR_RECT);

        std::cout << "BOSS Demon spawned in room " << roomID << " with " << demonConfig.max_health << " HP" << std::endl;
    }
}

// Update da sala
void Room::update(float deltaTime, sf::Vector2f playerPosition) {
    if (demon && demon->getHealth() > 0) {
        demon->update(deltaTime, playerPosition, gameBounds);
    }

    if (bishop && bishop->getHealth() > 0) {
        bishop->update(deltaTime, playerPosition, gameBounds);
    }

    if (bishop && bishop->shouldHealDemon()) {
        if (demon && demon->getHealth() > 0) {
            const int healAmount = ConfigManager::getInstance().getConfig().bishop.heal.amount;
            demon->heal(healAmount);
            bishop->resetHealFlag();
            std::cout << "Bishop healed Demon for " << healAmount << " HP" << std::endl;
        }
    }

    checkIfCleared();
}

// Draw da sala
void Room::draw(sf::RenderWindow& window) {
    for (const auto& door : doors) {
        if (door.sprite.has_value() && (door.isOpen || cleared)) {
            window.draw(*door.sprite);
        }
    }

    if (demon && demon->getHealth() > 0) {
        demon->draw(window);
    }

    if (bishop && bishop->getHealth() > 0) {
        bishop->draw(window);
    }
}

// Verifica se a sala está limpa
void Room::checkIfCleared() {
    if (cleared) return;

    if (type == RoomType::Treasure) {
        cleared = true;
        openDoors();
        std::cout << "Room " << roomID << " (Treasure) CLEARED! Doors opened." << std::endl;
        return;
    }

    bool demonDead = !demon || demon->getHealth() <= 0;
    bool bishopDead = !bishop || bishop->getHealth() <= 0;

    if (demonDead && bishopDead) {
        cleared = true;
        openDoors();
        std::cout << "Room " << roomID << " CLEARED! Doors opened." << std::endl;
    }
}

// Abre as portas
void Room::openDoors() {
    for (auto& door : doors) {
        door.isOpen = true;
    }
    doorsOpened = true;
}

// Fecha as portas
void Room::closeDoors() {
    if (!cleared) {
        for (auto& door : doors) {
            door.isOpen = false;
        }
        doorsOpened = false;
    }
}

// Verifica se tem porta numa direção
bool Room::hasDoor(DoorDirection direction) const {
    for (const auto& door : doors) {
        if (door.direction == direction) {
            return true;
        }
    }
    return false;
}

// Retorna ID da sala que a porta leva
int Room::getDoorLeadsTo(DoorDirection direction) const {
    for (const auto& door : doors) {
        if (door.direction == direction) {
            return door.leadsToRoomID;
        }
    }
    return -1;
}

// Retorna o tipo de porta
DoorType Room::getDoorType(DoorDirection direction) const {
    for (const auto& door : doors) {
        if (door.direction == direction) {
            return door.type;
        }
    }
    return DoorType::Normal;
}

// Retorna a posição de spawn do jogador
sf::Vector2f Room::getPlayerSpawnPosition(DoorDirection doorDirection) const {
    sf::Vector2f pos = getDoorPosition(doorDirection);
    float offset = ConfigManager::getInstance().getConfig().game.dungeon.player_spawn_offset;

    switch (doorDirection) {
    case DoorDirection::North:
        return { pos.x, pos.y + offset };
    case DoorDirection::South:
        return { pos.x, pos.y - offset };
    case DoorDirection::East:
        return { pos.x - offset, pos.y };
    case DoorDirection::West:
        return { pos.x + offset, pos.y };
    default:
        return { gameBounds.position.x + gameBounds.size.x / 2.f, gameBounds.position.y + gameBounds.size.y / 2.f };
    }
}