#include "RoomsManager.hpp"
#include "enemy.hpp"
#include "Utils.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random> // Necessário para std::mt19937 e std::shuffle

// Construtor
// NOTA: 'rng(rd())' inicializa o motor de aleatoriedade usando a 'seed' do sistema.
RoomManager::RoomManager(AssetManager& assetManager, const sf::FloatRect& gameBounds)
    : assets(assetManager)
    , gameBounds(gameBounds)
    , currentRoom(nullptr)
    , currentRoomID(0)
    , nextRoomID(-1)
    , transitionState(TransitionState::None)
    , transitionDirection(DoorDirection::None)
    , transitionProgress(0.f)
    // Inicialização do RNG
    , rng(rd())
{
    // Boa prática: Inicializa a seed do rand() antigo, caso seja usado noutros locais.
    std::srand(std::time(nullptr));

    // Setup do overlay de transição (tela preta)
    transitionOverlay.setSize({ 1920.f, 1080.f });
    transitionOverlay.setFillColor(sf::Color(0, 0, 0, 0));  // Transparente no início
}

// Gera o labirinto
void RoomManager::generateDungeon(int numRooms) {
    std::cout << "\n Generating dungeon with " << numRooms << " rooms..." << std::endl;

    // Carrega textura das portas
    sf::Texture& doorTexture = assets.getTexture("Door");

    // Sala 0: Safe Zone (sempre a primeira)
    createRoom(0, RoomType::SafeZone);

    // Cria salas de combate
    for (int i = 1; i < numRooms; ++i) {
        createRoom(i, RoomType::Combat);
    }

    // Conecta as salas em cadeia (labirito simples)
    for (int i = 0; i < numRooms - 1; ++i) {
        Room& currentRoom = rooms.at(i);
        Room& nextRoom = rooms.at(i + 1);

        // Direções aleatórias para variedade
        std::vector<DoorDirection> availableDirections = {
            DoorDirection::North,
            DoorDirection::South,
            DoorDirection::East,
            DoorDirection::West
        };

        // Embaralha (CORRIGIDO: usa std::shuffle e rng)
        std::shuffle(availableDirections.begin(), availableDirections.end(), rng);

        // Conecta usando a primeira direção disponível
        DoorDirection connectionDir = availableDirections[0];
        DoorDirection oppositeDir = getOppositeDirection(connectionDir);

        // Adiciona portas
        currentRoom.addDoor(connectionDir, doorTexture);
        nextRoom.addDoor(oppositeDir, doorTexture);

        // Conecta
        connectRooms(i, i + 1, connectionDir);

        std::cout << "   Room " << i << " ["
            << (connectionDir == DoorDirection::North ? "North" :
                connectionDir == DoorDirection::South ? "South" :
                connectionDir == DoorDirection::East ? "East" : "West")
            << "] → Room " << (i + 1) << std::endl;
    }

    // Adiciona portas extras aleatórias (becos sem saída para complexidade)
    for (auto& [id, room] : rooms) {
        // 30% chance de ter uma porta extra que não leva a lugar nenhum
        if (rand() % 100 < 30) {
            std::vector<DoorDirection> possibleDirs = {
                DoorDirection::North,
                DoorDirection::South,
                DoorDirection::East,
                DoorDirection::West
            };

            // Remove direções já usadas
            for (const auto& door : room.getDoors()) {
                possibleDirs.erase(
                    std::remove(possibleDirs.begin(), possibleDirs.end(), door.direction),
                    possibleDirs.end()
                );
            }

            if (!possibleDirs.empty()) {
                // Embaralha (CORRIGIDO: usa std::shuffle e rng)
                std::shuffle(possibleDirs.begin(), possibleDirs.end(), rng);
                room.addDoor(possibleDirs[0], doorTexture);
                std::cout << "   Room " << id << " got a dead-end door" << std::endl;
            }
        }
    }

    // Define sala inicial
    currentRoomID = 0;
    currentRoom = &rooms.at(currentRoomID);

    std::cout << " Dungeon generated! Starting in Safe Zone (Room 0)\n" << std::endl;
}

// Cria uma sala
void RoomManager::createRoom(int id, RoomType type) {
    rooms.emplace(id, Room(id, type, gameBounds));

    // Spawna inimigos se não for safe zone
    if (type != RoomType::SafeZone) {
        rooms.at(id).spawnEnemies(
            assets.getAnimationSet("D_Down"),
            assets.getAnimationSet("D_Up"),
            assets.getAnimationSet("D_Left"),
            assets.getAnimationSet("D_Right"),
            assets.getTexture("TearAtlas"),
            assets.getAnimationSet("Bishop")
        );
    }
}

// Conecta duas salas
void RoomManager::connectRooms(int roomA, int roomB, DoorDirection directionFromA) {
    rooms.at(roomA).connectDoor(directionFromA, roomB);

    DoorDirection oppositeDir = getOppositeDirection(directionFromA);
    rooms.at(roomB).connectDoor(oppositeDir, roomA);
}

// Retorna direção oposta
DoorDirection RoomManager::getOppositeDirection(DoorDirection direction) {
    switch (direction) {
    case DoorDirection::North: return DoorDirection::South;
    case DoorDirection::South: return DoorDirection::North;
    case DoorDirection::East: return DoorDirection::West;
    case DoorDirection::West: return DoorDirection::East;
    default: return DoorDirection::None;
    }
}

// Verifica se player está numa porta
DoorDirection RoomManager::checkPlayerAtDoor(const sf::FloatRect& playerBounds) {
    if (!currentRoom || !currentRoom->isCleared()) {
        return DoorDirection::None;  // Sala não está limpa, portas fechadas
    }

    // Usa checkCollision (assumindo que está em Utils.hpp)
    for (const auto& door : currentRoom->getDoors()) {
        // NOTA: Acesso a door.bounds é seguro, pois é um sf::FloatRect
        if (door.isOpen && door.leadsToRoomID != -1 && checkCollision(playerBounds, door.bounds)) {
            return door.direction;
        }
    }

    return DoorDirection::None;
}

// Requisita transição para outra sala
void RoomManager::requestTransition(DoorDirection direction) {
    if (transitionState != TransitionState::None) return;  // Já em transição

    int targetRoomID = currentRoom->getDoorLeadsTo(direction);
    if (targetRoomID == -1) {
        std::cout << " Door leads nowhere (dead end)" << std::endl;
        return;  // Porta não leva a lugar nenhum
    }

    nextRoomID = targetRoomID;
    transitionDirection = direction;
    transitionState = TransitionState::FadingOut;
    transitionProgress = 0.f;

    // Fecha portas ao sair (se não for safe zone, pode ser ignorado se a sala já estiver limpa)
    // currentRoom->closeDoors(); 

    std::cout << " Transitioning from Room " << currentRoomID
        << " to Room " << nextRoomID << std::endl;
}

// Update da transição
void RoomManager::updateTransition(float deltaTime, sf::Vector2f& playerPosition) {
    if (transitionState == TransitionState::None) return;

    // Adiciona o delta time com clamp para 1.0f para evitar ultrapassar
    transitionProgress = std::min(1.f, transitionProgress + (deltaTime / transitionDuration));

    switch (transitionState) {
    case TransitionState::FadingOut: {
        // Fade out (escurece a tela)
        int alpha = static_cast<int>(255 * transitionProgress);
        transitionOverlay.setFillColor(sf::Color(0, 0, 0, alpha));

        if (transitionProgress >= 1.f) {
            transitionState = TransitionState::Moving;
            transitionProgress = 0.f;

            // --- MUDANÇA DE SALA ---
            currentRoomID = nextRoomID;
            currentRoom = &rooms.at(currentRoomID);

            // Se a nova sala não estiver limpa, feche as portas imediatamente
            if (!currentRoom->isCleared()) {
                currentRoom->closeDoors();
            }

            // Reposiciona player do lado oposto, ligeiramente fora da porta
            sf::Vector2f offset = getTransitionOffset(
                getOppositeDirection(transitionDirection),
                1.f // Movimento total de reposicionamento
            );
            playerPosition += offset;
        }
        break;
    }

    case TransitionState::Moving: {
        // Este estado serve como um pequeno atraso/suavização no movimento do player.
        // Para um efeito de "puxar para dentro da sala", pode comentar este bloco
        // ou ajustar a lógica de reposicionamento para o FadingOut.

        // O movimento suave foi substituído pela lógica de reposicionamento no FadingOut.
        // Aqui, apenas avançamos o progresso para passar para o FadingIn.

        if (transitionProgress >= 1.f) {
            transitionState = TransitionState::FadingIn;
            transitionProgress = 0.f;
        }
        break;
    }

    case TransitionState::FadingIn: {
        // Fade in (clareia a tela)
        int alpha = static_cast<int>(255 * (1.f - transitionProgress));
        transitionOverlay.setFillColor(sf::Color(0, 0, 0, alpha));

        if (transitionProgress >= 1.f) {
            transitionState = TransitionState::None;
            transitionDirection = DoorDirection::None;
            transitionProgress = 0.f;
            transitionOverlay.setFillColor(sf::Color(0, 0, 0, 0));
        }
        break;
    }

    default:
        break;
    }
}

// Calcula offset de movimento baseado na direção
sf::Vector2f RoomManager::getTransitionOffset(DoorDirection direction, float progress) {
    const float moveDistance = 100.f;  // Distância total de movimento/reposicionamento

    switch (direction) {
    case DoorDirection::North:
        return { 0.f, -moveDistance * progress };
    case DoorDirection::South:
        return { 0.f, moveDistance * progress };
    case DoorDirection::East:
        return { moveDistance * progress, 0.f };
    case DoorDirection::West:
        return { -moveDistance * progress, 0.f };
    default:
        return { 0.f, 0.f };
    }
}

// Update
void RoomManager::update(float deltaTime, sf::Vector2f playerPosition) {
    if (currentRoom) {
        currentRoom->update(deltaTime, playerPosition);
    }
}

// Draw
void RoomManager::draw(sf::RenderWindow& window) {
    if (currentRoom) {
        currentRoom->draw(window);
    }
}

// Draw overlay de transição
void RoomManager::drawTransitionOverlay(sf::RenderWindow& window) {
    if (transitionState != TransitionState::None) {
        window.draw(transitionOverlay);
    }
}