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

    const auto& config = ConfigManager::getInstance().getConfig();
    const auto* doorConfig = &config.game.door_normal;

    // CORREÇÃO/ADICIONAL: Offset de 2px para a sala Boss
    const float bossOffsetY = (type == RoomType::Boss) ? 2.0f : 0.0f;

    // Seleciona a configuração correta baseada no tipo
    if (doorType == DoorType::Boss) {
        doorConfig = &config.game.door_boss;
    }
    else if (doorType == DoorType::Treasure) {
        doorConfig = &config.game.door_treasure;
    }

    // === FRAME DA PORTA (CONTÉM TUDO) - ORIGINAL ===
    door.sprite.emplace(doorSpritesheet);
    sf::IntRect textureRect(
        sf::Vector2i(doorConfig->frame_start_x, doorConfig->frame_start_y),
        sf::Vector2i(doorConfig->texture_width, doorConfig->texture_height)
    );
    door.sprite->setTextureRect(textureRect);
    door.sprite->setOrigin(sf::Vector2f(doorConfig->origin_x, doorConfig->origin_y));
    door.sprite->setScale(sf::Vector2f(doorConfig->scale_x, doorConfig->scale_y));

    // === METADE ESQUERDA ===
    door.leftHalf.emplace(doorSpritesheet);
    sf::IntRect leftRect(
        sf::Vector2i(doorConfig->left_half_x, doorConfig->left_half_y),
        sf::Vector2i(doorConfig->left_half_width, doorConfig->left_half_height)
    );
    door.leftHalf->setTextureRect(leftRect);
    door.leftHalf->setOrigin(sf::Vector2f(doorConfig->left_half_width, doorConfig->left_half_height / 2.0f));
    door.leftHalf->setScale(sf::Vector2f(doorConfig->scale_x, doorConfig->scale_y));
    door.leftHalfOriginalRect = leftRect; // Guardar original

    // === METADE DIREITA ===
    door.rightHalf.emplace(doorSpritesheet);
    sf::IntRect rightRect(
        sf::Vector2i(doorConfig->right_half_x, doorConfig->right_half_y),
        sf::Vector2i(doorConfig->right_half_width, doorConfig->right_half_height)
    );
    door.rightHalf->setTextureRect(rightRect);
    door.rightHalf->setOrigin(sf::Vector2f(0.0f, doorConfig->right_half_height / 2.0f));
    door.rightHalf->setScale(sf::Vector2f(doorConfig->scale_x, doorConfig->scale_y));
    door.rightHalfOriginalRect = rightRect; // Guardar original

    // === NOVO: MOLDURA DA PORTA (OVERLAY) - Baseada no tipo ===
    door.overlaySprite.emplace(doorSpritesheet);
    sf::IntRect overlayRect;

    // DEFINIÇÃO DAS NOVAS RECTS
    if (doorType == DoorType::Treasure) {
        // frame Treasure - ({188, 0},{49, 38})
        overlayRect = sf::IntRect(sf::Vector2i(188, 0), sf::Vector2i(49, 38));
    }
    else if (doorType == DoorType::Boss) {
        // A area da frame da boss- ({318, 0}{49, 43})
        overlayRect = sf::IntRect(sf::Vector2i(318, 0), sf::Vector2i(49, 43));
    }
    else { // DoorType::Normal
        // Coordenadas originais da porta Normal: IntRect({65, 0},{49, 33})
        overlayRect = sf::IntRect(sf::Vector2i(65, 0), sf::Vector2i(49, 33));
    }

    door.overlaySprite->setTextureRect(overlayRect);
    // Usar a mesma origem e escala da frame principal para alinhamento
    door.overlaySprite->setOrigin(door.sprite->getOrigin());
    door.overlaySprite->setScale(door.sprite->getScale());

    // Estado inicial: SafeZone/Treasure/Boss DEVEM estar ABERTAS
    if (this->type == RoomType::SafeZone || this->type == RoomType::Treasure || this->type == RoomType::Boss) {
        door.isOpen = true;
        door.state = DoorState::Open;
        door.animationProgress = 1.0f;
    }
    else {
        door.isOpen = false;
        door.state = DoorState::Closed;
        door.animationProgress = 0.0f;
    }

    door.leadsToRoomID = -1;

    // Define posição baseada na direção, ajustando para o offset Boss se necessário
    sf::Vector2f position = getDoorPosition(direction);
    position.y += bossOffsetY; // Aplicar offset Y

    // Posiciona frame
    door.sprite->setPosition(position);
    door.overlaySprite->setPosition(position); // Posiciona a moldura

    // Offset interno (36px) para alinhar metades ao centro
    sf::Vector2f doorHalvesOffset(0.f, 0.f);
    switch (direction) {
    case DoorDirection::North:
        doorHalvesOffset.y = -40.f;
        break;
    case DoorDirection::South:
        doorHalvesOffset.y = 40.f;
        break;
    case DoorDirection::East:
        doorHalvesOffset.x = 40.f;
        break;
    case DoorDirection::West:
        doorHalvesOffset.x = -40.f;
        break;
    }
    doorHalvesOffset.y += bossOffsetY; // Aplicar offset Boss às metades

    door.leftHalf->setPosition(position + doorHalvesOffset);
    door.rightHalf->setPosition(position + doorHalvesOffset);

    // Rotação baseada na direção
    float rotation = getDoorRotation(direction);
    door.sprite->setRotation(sf::degrees(rotation));
    door.overlaySprite->setRotation(sf::degrees(rotation));
    door.leftHalf->setRotation(sf::degrees(rotation));
    door.rightHalf->setRotation(sf::degrees(rotation));

    // === COLLISION SHAPE ===
    // CORREÇÃO: Definir frameBounds aqui
    sf::FloatRect frameBounds = door.sprite->getGlobalBounds();

    // As correções abaixo mantêm os bounds do mesmo tamanho que o código original
    door.collisionShape.emplace();
    door.collisionShape->setFillColor(sf::Color::Transparent);
    door.collisionShape->setOutlineColor(sf::Color::Transparent);

    if (direction == DoorDirection::North || direction == DoorDirection::South) {
        // Reduz nas laterais (7px de cada lado) e expande no topo/base (+1px)
        door.collisionShape->setSize(sf::Vector2f(
            frameBounds.size.x - 14.f,
            frameBounds.size.y + 6.f
        ));
        door.collisionShape->setPosition(sf::Vector2f(
            frameBounds.position.x + 7.f - 2.f,
            frameBounds.position.y - 1.f + 2.f
        ));
    }
    else {
        // East/West: Reduz em cima/baixo (7px de cada lado) e expande nas laterais (+1px)
        door.collisionShape->setSize(sf::Vector2f(
            frameBounds.size.x + 6.f,
            frameBounds.size.y - 14.f
        ));
        door.collisionShape->setPosition(sf::Vector2f(
            frameBounds.position.x - 1.f - 2.f,
            frameBounds.position.y + 7.f + 2.f
        ));
    }

    // Define bounds a partir do collision shape
    door.bounds = door.collisionShape->getGlobalBounds();

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
    const float extraOffset = 5.0f;

    switch (direction) {
    case DoorDirection::North:
        return sf::Vector2f(centerX, gameBounds.position.y + doorOffset - extraOffset);
    case DoorDirection::South:
        return sf::Vector2f(centerX, gameBounds.position.y + gameBounds.size.y - doorOffset + extraOffset);
    case DoorDirection::East:
        return sf::Vector2f(gameBounds.position.x + gameBounds.size.x - doorOffset + extraOffset, centerY);
    case DoorDirection::West:
        return sf::Vector2f(gameBounds.position.x + doorOffset - extraOffset, centerY);
    default:
        return sf::Vector2f(centerX, centerY);
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
        const sf::IntRect DEMON_TEAR_RECT(
            sf::Vector2i(demonTearConfig.x, demonTearConfig.y),
            sf::Vector2i(demonTearConfig.width, demonTearConfig.height)
        );
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
        const sf::IntRect DEMON_TEAR_RECT(
            sf::Vector2i(demonTearConfig.x, demonTearConfig.y),
            sf::Vector2i(demonTearConfig.width, demonTearConfig.height)
        );
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

    // Atualizar animações de portas
    updateDoorAnimations(deltaTime);

    checkIfCleared();
}

// Atualiza animações de todas as portas
void Room::updateDoorAnimations(float deltaTime) {
    for (auto& door : doors) {
        updateSingleDoorAnimation(door, deltaTime);
    }
}

// Atualiza animação de uma porta individual
void Room::updateSingleDoorAnimation(Door& door, float deltaTime) {
    const float animDuration = ConfigManager::getInstance().getConfig().game.dungeon.door_animation_duration;
    if (animDuration <= 0.0f) return;
    const float animSpeed = 1.0f / animDuration;

    switch (door.state) {
    case DoorState::Closing:
        door.animationProgress -= animSpeed * deltaTime;
        if (door.animationProgress <= 0.0f) {
            door.animationProgress = 0.0f;
            door.state = DoorState::Closed;
            door.isOpen = false;
        }
        break;

    case DoorState::Opening:
        door.animationProgress += animSpeed * deltaTime;
        if (door.animationProgress >= 1.0f) {
            door.animationProgress = 1.0f;
            door.state = DoorState::Open;
            door.isOpen = true;
        }
        break;

    case DoorState::Open:
    case DoorState::Closed:
        break;
    }
}

// Draw da sala
void Room::draw(sf::RenderWindow& window) {
    // Desenhar portas
    for (const auto& door : doors) {
        drawDoor(window, door);
    }

    // Desenhar inimigos
    if (demon && demon->getHealth() > 0) {
        demon->draw(window);
    }
    if (bishop && bishop->getHealth() > 0) {
        bishop->draw(window);
    }
}

// Desenha uma porta individual COM CLIPPING DE TEXTURA
void Room::drawDoor(sf::RenderWindow& window, const Door& door) const {
    if (!door.sprite.has_value() || !door.overlaySprite.has_value() ||
        !door.leftHalf.has_value() || !door.rightHalf.has_value()) {
        return;
    }

    // 1. Desenhar a frame da porta (parte fixa)
    window.draw(*door.sprite);

    // 2. Desenhar as metades da porta COM CLIPPING (Se não estiver completamente aberta)
    if (door.state != DoorState::Open) {

        // --- Cálculo do Movimento/Clipping ---

        const float maxHalfWidth = static_cast<float>(door.leftHalfOriginalRect.size.x);
        const float scaleX = door.leftHalf->getScale().x;

        // Distância máxima que a porta se move no ecrã (Ex: 14 * scaleX)
        const float maxOpenOffset = maxHalfWidth * scaleX;

        // Distância real que a porta moveu-se do centro
        float currentOffset = maxOpenOffset * door.animationProgress;

        // Padding: A margem que queremos que fique sempre fechada na textura (em pixels de textura)
        const float clippingPadding = 2.0f;

        // Largura da textura que realmente vai animar (e.g., 14 - 2 = 12px)
        const float effectiveMaxTexWidth = maxHalfWidth - clippingPadding;

        // Percentagem visível da textura (1.0 = fechada, 0.0 = aberta)
        float visiblePercent = 1.0f - door.animationProgress;

        // Largura visível da textura (em pixels de textura)
        int leftVisibleWidth = static_cast<int>(effectiveMaxTexWidth * visiblePercent);
        int rightVisibleWidth = static_cast<int>(effectiveMaxTexWidth * visiblePercent);

        leftVisibleWidth = std::max(0, leftVisibleWidth);
        rightVisibleWidth = std::max(0, rightVisibleWidth);

        sf::Sprite leftCopy = *door.leftHalf;
        sf::Sprite rightCopy = *door.rightHalf;
        float rotation = leftCopy.getRotation().asDegrees();


        // === 2a. APLICAR MOVIMENTO FÍSICO (Movimento para Longe do Centro) ===
        if (std::abs(rotation) < 1.0f) { // North (0°)
            leftCopy.move(sf::Vector2f(-currentOffset, 0.0f));
            rightCopy.move(sf::Vector2f(currentOffset, 0.0f));
        }
        else if (std::abs(rotation - 180.0f) < 1.0f) { // South (180°)
            leftCopy.move(sf::Vector2f(currentOffset, 0.0f));
            rightCopy.move(sf::Vector2f(-currentOffset, 0.0f));
        }
        else if (std::abs(rotation - 90.0f) < 1.0f) { // East (90°)
            leftCopy.move(sf::Vector2f(0.0f, -currentOffset));
            rightCopy.move(sf::Vector2f(0.0f, currentOffset));
        }
        else if (std::abs(rotation + 90.0f) < 1.0f) { // West (-90° ou 270°)
            leftCopy.move(sf::Vector2f(0.0f, currentOffset));
            rightCopy.move(sf::Vector2f(0.0f, -currentOffset));
        }

        // === 2b. APLICAR CLIPPING DE TEXTURA ===

        // --- METADE ESQUERDA (Clip da Esquerda / Origem fixa à Direita) ---
        if (leftVisibleWidth > 0) {
            sf::IntRect leftClippedRect = door.leftHalfOriginalRect;

            // A posição X na textura deve avançar para esconder o padding e o movimento
            float totalTextureMove = clippingPadding + (effectiveMaxTexWidth - leftVisibleWidth);

            leftClippedRect.position.x += static_cast<int>(totalTextureMove);
            leftClippedRect.size.x = leftVisibleWidth;
            leftCopy.setTextureRect(leftClippedRect);

            // A origem está na direita, então deve ser a nova largura visível
            leftCopy.setOrigin(sf::Vector2f(leftVisibleWidth, door.leftHalfOriginalRect.size.y / 2.0f));

            window.draw(leftCopy);
        }

        // --- METADE DIREITA (Clip da Direita / Origem fixa à Esquerda) ---
        if (rightVisibleWidth > 0) {
            sf::IntRect rightClippedRect = door.rightHalfOriginalRect;

            // A posição X na textura deve avançar para esconder o padding
            rightClippedRect.position.x += static_cast<int>(clippingPadding);

            // A largura da textura é reduzida para esconder o movimento e o padding
            rightClippedRect.size.x = rightVisibleWidth;

            window.draw(rightCopy);
        }
    }

    // 3. Desenhar a moldura da porta (overlay) por cima de tudo
    // (Isto garante que a moldura fica sempre por cima das metades/animação)
    window.draw(*door.overlaySprite);
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

// Abre as portas (inicia animação)
void Room::openDoors() {
    for (auto& door : doors) {
        if (door.state == DoorState::Closed || door.state == DoorState::Closing) {
            door.state = DoorState::Opening;
        }
    }
    doorsOpened = true;
}

// Fecha as portas (inicia animação)
void Room::closeDoors() {
    if (!cleared) {
        for (auto& door : doors) {
            // CORREÇÃO: Apenas portas normais devem fechar
            if (door.type == DoorType::Normal) {
                if (door.state == DoorState::Open || door.state == DoorState::Opening) {
                    door.state = DoorState::Closing;
                }
            }
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
    const float extraSpawnOffset = 5.0f; // +5px para evitar teleporte loop
    const float bossOffsetY = (type == RoomType::Boss) ? 2.0f : 0.0f; // Adicionar offset Y

    switch (doorDirection) {
    case DoorDirection::North:
        return sf::Vector2f(pos.x, pos.y + offset + extraSpawnOffset + bossOffsetY);
    case DoorDirection::South:
        return sf::Vector2f(pos.x, pos.y - offset - extraSpawnOffset + bossOffsetY);
    case DoorDirection::East:
        return sf::Vector2f(pos.x - offset - extraSpawnOffset, pos.y + bossOffsetY);
    case DoorDirection::West:
        return sf::Vector2f(pos.x + offset + extraSpawnOffset, pos.y + bossOffsetY);
    default:
        return sf::Vector2f(gameBounds.position.x + gameBounds.size.x / 2.f, gameBounds.position.y + gameBounds.size.y / 2.f + bossOffsetY);
    }
}