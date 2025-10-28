#include "RoomsManager.hpp"
#include "enemy.hpp"
#include "Utils.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random> 
#include <map>

// Construtor
RoomManager::RoomManager(AssetManager& assetManager, const sf::FloatRect& gameBounds)
    : assets(assetManager)
    , gameBounds(gameBounds)
    , currentRoom(nullptr)
    , currentRoomID(0)
    , nextRoomID(-1)
    , transitionState(TransitionState::None)
    , transitionDirection(DoorDirection::None)
    , transitionProgress(0.f)
    , transitionDuration(0.5f)
    // Inicialização do RNG
    , rng(rd())
{
    std::srand(std::time(nullptr));

    // Setup do overlay de transição (tela preta)
    transitionOverlay.setSize({ 1920.f, 1080.f });
    transitionOverlay.setFillColor(sf::Color(0, 0, 0, 0));
}

// Helper para logs (CORRIGIDO: Função local estática para evitar erro de 'não definido')
static std::string dirToString(DoorDirection direction) {
    switch (direction) {
    case DoorDirection::North: return "North";
    case DoorDirection::South: return "South";
    case DoorDirection::East: return "East";
    case DoorDirection::West: return "West";
    default: return "None";
    }
}

// Gera o labirinto
void RoomManager::generateDungeon(int numRooms) {
    std::cout << "\n Generating dungeon with " << numRooms << " rooms..." << std::endl;

    // Garante que o mapa está limpo antes de gerar
    rooms.clear();

    sf::Texture& doorTexture = assets.getTexture("Door");

    // 1. Cria Salas
    createRoom(0, RoomType::SafeZone);
    for (int i = 1; i < numRooms; ++i) {
        createRoom(i, RoomType::Combat);
    }

    // Estrutura para rastrear as direções usadas em cada sala
    std::map<int, std::vector<DoorDirection>> usedDirections;
    for (int i = 0; i < numRooms; ++i) {
        usedDirections[i] = {};
    }

    // 2. Conexões Principais (Cadeia linear)
    for (int i = 0; i < numRooms - 1; ++i) {
        Room& current = rooms.at(i);
        Room& next = rooms.at(i + 1);

        std::vector<DoorDirection> availableDirs = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };

        // Remove direções já usadas (garantido apenas para loops na secção 3, mas mantém a segurança)
        for (const auto& dir : usedDirections[i]) {
            availableDirs.erase(std::remove(availableDirs.begin(), availableDirs.end(), dir), availableDirs.end());
        }

        std::shuffle(availableDirs.begin(), availableDirs.end(), rng);
        DoorDirection connectionDir = availableDirs[0];
        DoorDirection oppositeDir = getOppositeDirection(connectionDir);

        // Adiciona e conecta
        current.addDoor(connectionDir, doorTexture);
        next.addDoor(oppositeDir, doorTexture);
        connectRooms(i, i + 1, connectionDir);

        usedDirections[i].push_back(connectionDir);
        usedDirections[i + 1].push_back(oppositeDir);

        std::cout << "   Room " << i << " [" << dirToString(connectionDir) << "] → Room " << (i + 1) << std::endl;
    }

    // 3. Conexões Extras (Loops para trás)
    for (int i = 0; i < numRooms - 1; ++i) {
        Room& current = rooms.at(i);

        // Lógica de probabilidade elevada para a Sala 0
        int doorsToAdd = 0;
        int baseChance2 = (i == 0) ? 90 : 50;
        int baseChance3 = (i == 0) ? 70 : 30;
        int baseChance4 = (i == 0) ? 50 : 10;

        // Decide quantas portas extra TENTAR adicionar
        if (rand() % 100 < baseChance2) doorsToAdd++; // 2+ portas
        if (rand() % 100 < baseChance3) doorsToAdd++; // 3+ portas
        if (rand() % 100 < baseChance4) doorsToAdd++; // 4 portas

        // Calcula quantas portas extra são necessárias
        int currentConnectedDoors = current.getDoors().size();
        int doorsNeeded = doorsToAdd; // Quantas portas extra queremos que a sala tenha (além da conexão principal)

        // Itera para tentar adicionar as portas extras (loops para salas anteriores)
        for (int j = 0; j < doorsNeeded; ++j) {

            // Re-checa se o número de portas já é suficiente após o último loop
            if (current.getDoors().size() >= 4) break;

            std::vector<DoorDirection> availableDirs = {
                DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West
            };

            // Remove as direções já usadas
            for (const auto& door : current.getDoors()) {
                availableDirs.erase(std::remove(availableDirs.begin(), availableDirs.end(), door.direction), availableDirs.end());
            }

            if (availableDirs.empty()) break;

            std::shuffle(availableDirs.begin(), availableDirs.end(), rng);
            DoorDirection extraDir = availableDirs[0];

            // Tenta conectar à sala anterior (i - 1)
            if (i > 0) {
                int targetRoomID = i - 1;
                DoorDirection neededDir = getOppositeDirection(extraDir);
                Room& targetRoom = rooms.at(targetRoomID);

                // Só cria a porta se a sala anterior tiver a direção oposta livre
                if (!targetRoom.hasDoor(neededDir)) {
                    // Adiciona porta em 'current'
                    current.addDoor(extraDir, doorTexture);
                    // Adiciona porta em 'targetRoom'
                    targetRoom.addDoor(neededDir, doorTexture);

                    // Conecta as duas
                    connectRooms(targetRoomID, i, neededDir); // Conecta Target -> Current
                    connectRooms(i, targetRoomID, extraDir); // Conecta Current -> Target

                    std::cout << "   Room " << i << " [" << dirToString(extraDir) << "] ↔ Loop to Room " << targetRoomID << std::endl;
                }
                // Se não deu para conectar (direção oposta ocupada), simplesmente ignora e não cria dead-end.
            }
        }
    }


    // Define sala inicial
    currentRoomID = 0;
    currentRoom = &rooms.at(currentRoomID);

    currentRoom->openDoors();

    std::cout << " Dungeon generated! Starting in Safe Zone (Room 0)\n" << std::endl;
}

// Cria uma sala
void RoomManager::createRoom(int id, RoomType type) {
    rooms.emplace(id, Room(id, type, gameBounds));
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
        return DoorDirection::None;
    }

    for (const auto& door : currentRoom->getDoors()) {
        if (door.isOpen && door.leadsToRoomID != -1 && checkCollision(playerBounds, door.bounds)) {
            return door.direction;
        }
    }

    return DoorDirection::None;
}

// Requisita transição para outra sala
void RoomManager::requestTransition(DoorDirection direction) {
    if (transitionState != TransitionState::None) return;

    int targetRoomID = currentRoom->getDoorLeadsTo(direction);
    if (targetRoomID == -1) {
        std::cout << " Door leads nowhere (dead end)" << std::endl;
        return;
    }

    currentRoom->closeDoors();

    nextRoomID = targetRoomID;
    transitionDirection = direction;
    transitionState = TransitionState::FadingOut;
    transitionProgress = 0.f;

    std::cout << " Transitioning from Room " << currentRoomID
        << " to Room " << nextRoomID << std::endl;
}

// Update da transição
void RoomManager::updateTransition(float deltaTime, sf::Vector2f& playerPosition) {
    if (transitionState == TransitionState::None) return;

    transitionProgress = std::min(1.f, transitionProgress + (deltaTime / transitionDuration));

    switch (transitionState) {
    case TransitionState::FadingOut: {
        int alpha = static_cast<int>(255 * transitionProgress);
        transitionOverlay.setFillColor(sf::Color(0, 0, 0, alpha));

        if (transitionProgress >= 1.f) {

            // --- 1. MUDANÇA DE SALA ---
            currentRoomID = nextRoomID;
            currentRoom = &rooms.at(currentRoomID);

            // --- 2. CONFIGURAÇÃO DA NOVA SALA E SPAWN DE INIMIGOS ---
            if (!currentRoom->isCleared()) {
                currentRoom->closeDoors();
            }

            currentRoom->spawnEnemies(
                assets.getAnimationSet("D_Down"),
                assets.getAnimationSet("D_Up"),
                assets.getAnimationSet("D_Left"),
                assets.getAnimationSet("D_Right"),
                assets.getTexture("TearAtlas"),
                assets.getAnimationSet("Bishop")
            );

            // --- 3. REPOSICIONAMENTO DO PLAYER (teletransporte) ---
            DoorDirection spawnDoor = getOppositeDirection(transitionDirection);
            playerPosition = currentRoom->getPlayerSpawnPosition(spawnDoor);

            // --- 4. PRÓXIMO ESTADO ---
            transitionState = TransitionState::FadingIn;
            transitionProgress = 0.f;

            transitionOverlay.setFillColor(sf::Color::Black);
        }
        break;
    }

    case TransitionState::FadingIn: {
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

// Este método agora é redundante
sf::Vector2f RoomManager::getTransitionOffset(DoorDirection direction, float progress) {
    return { 0.f, 0.f };
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