#include "RoomsManager.hpp"
#include "enemy.hpp"
#include "Utils.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random> 
#include <map>
#include <SFML/System/Vector2.hpp> 


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
    , transitionDuration(0.49f)
    // Inicialização do RNG
    , rng(rd())
{
    std::srand(std::time(nullptr));

    // Setup do overlay de transição (tela preta)
    transitionOverlay.setSize({ 1920.f, 1080.f });
    transitionOverlay.setFillColor(sf::Color(0, 0, 0, 0));
}

// Helper para logs
static std::string dirToString(DoorDirection direction) {
    switch (direction) {
    case DoorDirection::North: return "North";
    case DoorDirection::South: return "South";
    case DoorDirection::East: return "East";
    case DoorDirection::West: return "West";
    default: return "None";
    }
}

// Retorna a próxima coordenada baseada na direção
sf::Vector2i getNextCoord(const sf::Vector2i& current, DoorDirection direction) {
    switch (direction) {
    case DoorDirection::North: return { current.x, current.y - 1 };
    case DoorDirection::South: return { current.x, current.y + 1 };
    case DoorDirection::East: return { current.x + 1, current.y };
    case DoorDirection::West: return { current.x - 1, current.y };
    default: return current;
    }
}

// Gera o labirinto
void RoomManager::generateDungeon(int numRooms) {
    std::cout << "\n Generating dungeon with " << numRooms << " rooms using Grid Coordinates..." << std::endl;

    // Limpa estruturas
    rooms.clear();
    coordToRoomID.clear();

    sf::Texture& doorTexture = assets.getTexture("Door");
    int nextAvailableRoomID = 0;

    // 1. Cria Sala 0 (Safe Zone)
    createRoom(nextAvailableRoomID, RoomType::SafeZone);
    coordToRoomID[{0, 0}] = nextAvailableRoomID;
    nextAvailableRoomID++;

    // Fila de salas existentes que podem ter novas conexões (coordenadas)
    std::vector<sf::Vector2i> availableCoords;
    availableCoords.push_back({ 0, 0 });

    // 2. Criação da Cadeia Principal (DFS / Spanning Tree)
    while (nextAvailableRoomID < numRooms && !availableCoords.empty()) {

        // Escolhe uma sala existente de onde ramificar
        std::uniform_int_distribution<> coordDist(0, availableCoords.size() - 1);
        sf::Vector2i parentCoord = availableCoords[coordDist(rng)];
        int parentID = coordToRoomID[parentCoord];

        // Tenta ramificar em direções aleatórias
        std::vector<DoorDirection> potentialDirections = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
        std::shuffle(potentialDirections.begin(), potentialDirections.end(), rng);

        bool roomAdded = false;

        for (DoorDirection dir : potentialDirections) {
            sf::Vector2i nextCoord = getNextCoord(parentCoord, dir);

            // Verifica se a coordenada está livre e se ainda temos IDs para usar
            if (coordToRoomID.find(nextCoord) == coordToRoomID.end() && nextAvailableRoomID < numRooms) {

                int newRoomID = nextAvailableRoomID;

                // Cria a nova sala
                createRoom(newRoomID, RoomType::Combat);
                coordToRoomID[nextCoord] = newRoomID;

                // Conecta as duas salas
                DoorDirection oppositeDir = getOppositeDirection(dir);
                rooms.at(parentID).addDoor(dir, doorTexture);
                rooms.at(newRoomID).addDoor(oppositeDir, doorTexture);
                connectRooms(parentID, newRoomID, dir);

                // Adiciona nova sala ao pool de ramificação
                availableCoords.push_back(nextCoord);

                std::cout << "   Room " << parentID << " at (" << parentCoord.x << "," << parentCoord.y
                    << ") [" << dirToString(dir) << "] → Room " << newRoomID
                    << " at (" << nextCoord.x << "," << nextCoord.y << ")" << std::endl;

                roomAdded = true;
                nextAvailableRoomID++;
                break; // Adicionou 1, avança para a próxima iteração
            }
        }

        // Se a sala não puder mais ramificar, remove-a do pool (exceto a Safe Zone)
        if (!roomAdded && parentID != 0) {
            availableCoords.erase(std::remove(availableCoords.begin(), availableCoords.end(), parentCoord), availableCoords.end());
        }
    }


    // 3. Adiciona Conexões Extras (Loops para evitar dead-ends no grid e dar variedade)
    for (int i = 0; i < nextAvailableRoomID; ++i) {
        Room& currentRoom = rooms.at(i);

        // Define a probabilidade de tentar adicionar portas extras
        int doorsToAdd = 0;

        // Lógica de probabilidade elevada para a Sala 0
        bool isSafeZone = (i == 0);

        // --- PROBABILIDADES AUMENTADAS ---
        // Chance para tentar adicionar a porta extra N (além da garantida)
        int baseChance2 = isSafeZone ? 100 : 70; // Aumentado de 90/50
        int baseChance3 = isSafeZone ? 90 : 50;  // Aumentado de 70/30
        int baseChance4 = isSafeZone ? 70 : 30;  // Aumentado de 50/10
        int baseChance5 = isSafeZone ? 50 : 15;  // NOVO
        int baseChance6 = isSafeZone ? 20 : 5;   // NOVO


        // Decide quantas portas extra TENTAR adicionar (máximo 5 tentativas)
        if (rand() % 100 < baseChance2) doorsToAdd++;
        if (rand() % 100 < baseChance3) doorsToAdd++;
        if (rand() % 100 < baseChance4) doorsToAdd++;
        if (rand() % 100 < baseChance5) doorsToAdd++; // NOVO
        if (rand() % 100 < baseChance6) doorsToAdd++; // NOVO

        // Itera para tentar adicionar as portas extras
        for (int j = 0; j < doorsToAdd; ++j) {

            if (currentRoom.getDoors().size() >= 4) break;

            // Pega as direções disponíveis na sala atual
            std::vector<DoorDirection> availableDirs = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
            for (const auto& door : currentRoom.getDoors()) {
                availableDirs.erase(std::remove(availableDirs.begin(), availableDirs.end(), door.direction), availableDirs.end());
            }

            if (availableDirs.empty()) break;

            std::shuffle(availableDirs.begin(), availableDirs.end(), rng);
            DoorDirection extraDir = availableDirs[0];

            // Encontra a coordenada da sala atual
            sf::Vector2i currentCoord = { 0, 0 };
            bool found = false;

            for (const auto& pair : coordToRoomID) {
                if (pair.second == i) {
                    currentCoord = pair.first;
                    found = true;
                    break;
                }
            }
            if (!found) continue;

            sf::Vector2i neighborCoord = getNextCoord(currentCoord, extraDir);

            // Verifica se a sala vizinha existe
            if (coordToRoomID.count(neighborCoord)) {
                int neighborID = coordToRoomID.at(neighborCoord);

                // Evita loops que se ligam à própria sala
                if (neighborID == i) continue;

                // Verifica se a porta já existe na sala atual 
                if (!currentRoom.hasDoor(extraDir)) {
                    Room& neighborRoom = rooms.at(neighborID);
                    DoorDirection oppositeDir = getOppositeDirection(extraDir);

                    // Só conecta se a sala vizinha também tiver a direção oposta livre
                    if (!neighborRoom.hasDoor(oppositeDir)) {

                        // Adiciona portas a ambas as salas
                        currentRoom.addDoor(extraDir, doorTexture);
                        neighborRoom.addDoor(oppositeDir, doorTexture);

                        // Conecta-as
                        connectRooms(i, neighborID, extraDir);

                        std::cout << "   Room " << i << " at (" << currentCoord.x << "," << currentCoord.y << ") [" << dirToString(extraDir)
                            << "] ↔ Loop to Room " << neighborID << " at (" << neighborCoord.x << "," << neighborCoord.y << ")" << std::endl;
                    }
                }
            }
        }
    }


    // Define sala inicial
    currentRoomID = 0;
    currentRoom = &rooms.at(currentRoomID);

    currentRoom->openDoors();

    std::cout << "\n Dungeon generated! Starting in Safe Zone (Room 0) at (0,0)\n" << std::endl;
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
// ============================================================
// IMPLEMENTAÇÃO DO MINIMAPA
// ============================================================

// Retorna as coordenadas da sala atual
sf::Vector2i RoomManager::getCurrentRoomCoord() const {
    for (const auto& pair : coordToRoomID) {
        if (pair.second == currentRoomID) {
            return pair.first;
        }
    }
    return { 0, 0 }; // Fallback para a sala inicial
}

// Desenha o minimapa no canto superior direito
void RoomManager::drawMiniMap(sf::RenderWindow& window) {
    // Configurações do minimapa
    const float miniMapSize = 150.f;
    const float roomSize = 15.f;
    const float roomSpacing = 18.f; // Espaço entre salas (inclui tamanho + margem)
    const float connectionThickness = 2.f;
    
    // Posição do minimapa (canto superior direito)
    const sf::Vector2f miniMapPosition(window.getSize().x - miniMapSize - 20.f, 20.f);
    
    // Fundo semitransparente do minimapa
    sf::RectangleShape background;
    background.setSize({ miniMapSize, miniMapSize });
    background.setPosition(miniMapPosition);
    background.setFillColor(sf::Color(0, 0, 0, 100));
    background.setOutlineColor(sf::Color(100, 100, 100, 200));
    background.setOutlineThickness(2.f);
    window.draw(background);
    
    // Adiciona a sala atual às salas visitadas
    visitedRooms.insert(currentRoomID);
    
    // Obtém coordenadas da sala atual
    sf::Vector2i currentCoord = getCurrentRoomCoord();
    
    // Centro do minimapa
    sf::Vector2f miniMapCenter = miniMapPosition + sf::Vector2f(miniMapSize / 2.f, miniMapSize / 2.f);
    
    // Desenha apenas as salas visitadas e suas conexões
    for (int visitedRoomID : visitedRooms) {
        // Encontra as coordenadas desta sala visitada
        sf::Vector2i roomCoord = { 0, 0 };
        bool found = false;
        
        for (const auto& pair : coordToRoomID) {
            if (pair.second == visitedRoomID) {
                roomCoord = pair.first;
                found = true;
                break;
            }
        }
        
        if (!found) continue;
        
        // Calcula posição relativa à sala atual
        sf::Vector2i relativeCoord = roomCoord - currentCoord;
        
        // Posição no minimapa (centralizada na sala atual)
        sf::Vector2f roomPosition = miniMapCenter + sf::Vector2f(
            relativeCoord.x * roomSpacing,
            relativeCoord.y * roomSpacing
        );
        
        // Verifica se a sala está dentro dos limites do minimapa
        sf::FloatRect miniMapBounds({ miniMapPosition.x, miniMapPosition.y }, { miniMapSize, miniMapSize });
        sf::FloatRect roomBounds({ roomPosition.x - roomSize / 2.f, roomPosition.y - roomSize / 2.f }, { roomSize, roomSize });
        
        if (!checkCollision(miniMapBounds, roomBounds)) {
            continue; // Sala fora do minimapa visível
        }
        
        // Desenha conexões (linhas) para salas adjacentes visitadas
        Room& room = rooms.at(visitedRoomID);
        for (const auto& door : room.getDoors()) {
            int neighborID = door.leadsToRoomID;
            
            // Verifica se a sala vizinha foi visitada
            if (neighborID != -1 && visitedRooms.count(neighborID) > 0) {
                sf::Vector2i neighborCoord = { 0, 0 };
                
                for (const auto& pair : coordToRoomID) {
                    if (pair.second == neighborID) {
                        neighborCoord = pair.first;
                        break;
                    }
                }
                
                sf::Vector2i relativeNeighbor = neighborCoord - currentCoord;
                sf::Vector2f neighborPosition = miniMapCenter + sf::Vector2f(
                    relativeNeighbor.x * roomSpacing,
                    relativeNeighbor.y * roomSpacing
                );
                
                // Desenha linha de conexão
                sf::RectangleShape connection;
                sf::Vector2f direction = neighborPosition - roomPosition;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f;
                
                connection.setSize({ length, connectionThickness });
                connection.setPosition(roomPosition);
                connection.setOrigin({ 0.f, connectionThickness / 2.f });
                connection.setRotation(sf::degrees(angle));
                connection.setFillColor(sf::Color(80, 80, 80, 200));
                
                window.draw(connection);
            }
        }
        
        // Desenha a sala
        sf::RectangleShape roomRect;
        roomRect.setSize({ roomSize, roomSize });
        roomRect.setOrigin({ roomSize / 2.f, roomSize / 2.f });
        roomRect.setPosition(roomPosition);
        
        // Define a cor baseada no estado da sala
        if (visitedRoomID == currentRoomID) {
            // Sala atual - vermelho
            roomRect.setFillColor(sf::Color(255, 0, 0, 255));
            roomRect.setOutlineColor(sf::Color(255, 255, 255, 255));
            roomRect.setOutlineThickness(1.5f);
        }
        else if (room.isCleared()) {
            // Sala limpa - branco
            roomRect.setFillColor(sf::Color(255, 255, 255, 255));
            roomRect.setOutlineColor(sf::Color(200, 200, 200, 200));
            roomRect.setOutlineThickness(1.f);
        }
        else {
            // Sala visitada mas não limpa - cinzenta
            roomRect.setFillColor(sf::Color(150, 150, 150, 255));
            roomRect.setOutlineColor(sf::Color(200, 200, 200, 200));
            roomRect.setOutlineThickness(1.f);
        }
        
        window.draw(roomRect);
    }
}
