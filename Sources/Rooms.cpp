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

// Adiciona uma porta à sala
void Room::addDoor(DoorDirection direction, sf::Texture& doorTexture) {
    Door door;
    door.direction = direction;

    // 1. ✅ CORREÇÃO: Inicializa o std::optional<sf::Sprite> chamando o construtor
    //    sf::Sprite(doorTexture). Isto resolve os erros de construtor padrão e std::construct_at.
    door.sprite.emplace(doorTexture);

    // 2. Acede ao sprite interno usando o operador ->
    door.sprite->setTextureRect(sf::IntRect({ 0, 0 }, { 48, 32 }));
    door.sprite->setOrigin({ 24.f, 16.f });  // Centro da porta
    door.sprite->setScale({ 3.f, 3.f });

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

    switch (direction) {
    case DoorDirection::North:
        return { centerX, gameBounds.position.y + 50.f };  // Top
    case DoorDirection::South:
        return { centerX, gameBounds.position.y + gameBounds.size.y - 50.f };  // Bottom
    case DoorDirection::East:
        return { gameBounds.position.x + gameBounds.size.x - 50.f, centerY };  // Right
    case DoorDirection::West:
        return { gameBounds.position.x + 50.f, centerY };  // Left
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
    // Safe zone não tem inimigos
    if (type == RoomType::SafeZone) {
        return;
    }

    // Demon sempre spawna em salas de combate
    if (type == RoomType::Combat) {
        demon.emplace(
            demonWalkDown,
            demonWalkUp,
            demonWalkLeft,
            demonWalkRight,
            demonProjectileTexture
        );

        // Define recorte de projétil do demon
        const sf::IntRect DEMON_TEAR_RECT = { {8, 103}, {16, 16} };
        demon->setProjectileTextureRect(DEMON_TEAR_RECT);

        std::cout << "Demon spawned in room " << roomID << std::endl;
    }

    // Bishop tem 30% de chance de spawnar
    int random = rand() % 100;
    if (random < 30) {  // 30% chance
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
            // Acede à config.bishop.heal_amount (assumindo a estrutura correta)
            const int healAmount = ConfigManager::getInstance().getConfig().bishop.heal_amount;
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