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
    // A Safe Zone (Room 0) está sempre limpa
    , cleared(type == RoomType::SafeZone)
    , doorsOpened(type == RoomType::SafeZone)
{
}

// Adiciona uma porta à sala
void Room::addDoor(DoorDirection direction, sf::Texture& doorTexture) {
    Door door;
    door.direction = direction;

    // 1. Inicializa o std::optional<sf::Sprite>
    door.sprite.emplace(doorTexture);

    // 2. Acede ao sprite interno usando o operador ->
    const auto& doorConfig = ConfigManager::getInstance().getConfig().game.door_normal;
    door.sprite->setTextureRect(sf::IntRect({ 0, 0 }, { doorConfig.texture_width, doorConfig.texture_height }));

    // Origem: 24.f (Centro X) e 31.f (Base - 1px para ficar no limite)
    door.sprite->setOrigin({ doorConfig.origin_x, doorConfig.origin_y });

    door.sprite->setScale({ doorConfig.scale_x, doorConfig.scale_y });

    door.isOpen = (type == RoomType::SafeZone);
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

    // AJUSTE FINAL: Offset para portas nos limites
    const float doorOffset = ConfigManager::getInstance().getConfig().game.dungeon.door_offset;

    switch (direction) {
    case DoorDirection::North:
        return { centerX, gameBounds.position.y + doorOffset };  // Top 
    case DoorDirection::South:
        return { centerX, gameBounds.position.y + gameBounds.size.y - doorOffset };  // Bottom
    case DoorDirection::East:
        return { gameBounds.position.x + gameBounds.size.x - doorOffset, centerY };  // Right
    case DoorDirection::West:
        return { gameBounds.position.x + doorOffset, centerY };  // Left
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
    // Se for Safe Zone, retorna imediatamente (Nunca tem inimigos)
    if (type == RoomType::SafeZone) {
        return;
    }

    // Não spawna inimigos se a sala já foi limpa.
    if (cleared) {
        return;
    }

    // Verifica se já existem inimigos (proteção contra chamadas duplas antes de limpar)
    if (demon.has_value() || bishop.has_value()) {
        return;
    }

    // Demon sempre spawna em salas de combate
    if (type == RoomType::Normal) {
        demon.emplace(
            demonWalkDown,
            demonWalkUp,
            demonWalkLeft,
            demonWalkRight,
            demonProjectileTexture
        );

        // Define recorte de projétil do demon
        const auto& demonTearConfig = ConfigManager::getInstance().getConfig().projectile_textures.demon_tear;
        const sf::IntRect DEMON_TEAR_RECT = { {demonTearConfig.x, demonTearConfig.y}, {demonTearConfig.width, demonTearConfig.height} };
        demon->setProjectileTextureRect(DEMON_TEAR_RECT);

        std::cout << "Demon spawned in room " << roomID << std::endl;
    }

    // Bishop tem X% de chance de spawnar (lido da config)
    const auto& bishopConfig = ConfigManager::getInstance().getConfig().bishop.stats;
    int random = rand() % 100;
    if (random < bishopConfig.spawn_chance_percent) {  // Chance lida da config
        bishop.emplace(bishopTextures);
        std::cout << "Bishop spawned in room " << roomID << " (30% chance)" << std::endl;
    }
    else {
        std::cout << "Bishop NOT spawned in room " << roomID << " (rolled " << random << "%)" << std::endl;
    }
}

// Update da sala
void Room::update(float deltaTime, sf::Vector2f playerPosition) {
    // Update demon
    if (demon && demon->getHealth() > 0) {
        demon->update(deltaTime, playerPosition, gameBounds);
    }

    // Update bishop
    if (bishop && bishop->getHealth() > 0) {
        bishop->update(deltaTime, playerPosition, gameBounds);
    }

    // Bishop cura demon
    if (bishop && bishop->shouldHealDemon()) {
        if (demon && demon->getHealth() > 0) {
            // Acede à config.bishop.heal.amount (agora aninhado)
            const int healAmount = ConfigManager::getInstance().getConfig().bishop.heal.amount;
            demon->heal(healAmount);
            bishop->resetHealFlag();
            std::cout << "Bishop healed Demon for " << healAmount << " HP" << std::endl;
        }
    }

    // Verifica se a sala foi limpa
    checkIfCleared();
}

// Draw da sala
void Room::draw(sf::RenderWindow& window) {
    // Desenha portas
    for (const auto& door : doors) {
        // Usa .has_value() para garantir que o sprite foi inicializado
        if (door.sprite.has_value() && (door.isOpen || cleared)) {
            window.draw(*door.sprite); // Usa * para obter o objeto sf::Sprite
        }
    }

    // Desenha inimigos
    if (demon && demon->getHealth() > 0) {
        demon->draw(window);
    }

    if (bishop && bishop->getHealth() > 0) {
        bishop->draw(window);
    }
}

// Verifica se a sala está limpa (todos inimigos mortos)
void Room::checkIfCleared() {
    if (cleared) return;  // Já limpa

    // Um inimigo não existe (!demon) ou está morto (getHealth() <= 0)
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
    // Apenas fecha se a sala não estiver limpa
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

// Retorna a posição ideal de SPAN DO JOGADOR perto de uma porta.
sf::Vector2f Room::getPlayerSpawnPosition(DoorDirection doorDirection) const {
    sf::Vector2f pos = getDoorPosition(doorDirection);
    float offset = ConfigManager::getInstance().getConfig().game.dungeon.player_spawn_offset; // Afasta X pixels da porta

    switch (doorDirection) {
        case DoorDirection::North:
            return { pos.x, pos.y + offset }; // Aparece abaixo da porta
        case DoorDirection::South:
            return { pos.x, pos.y - offset }; // Aparece acima da porta
        case DoorDirection::East:
            return { pos.x - offset, pos.y }; // Aparece à esquerda da porta
        case DoorDirection::West:
            return { pos.x + offset, pos.y }; // Aparece à direita da porta
        default:
            return { gameBounds.position.x + gameBounds.size.x / 2.f, gameBounds.position.y + gameBounds.size.y / 2.f }; // Centro
        }
}