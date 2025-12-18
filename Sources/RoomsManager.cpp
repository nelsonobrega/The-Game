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
    , transitionDuration(ConfigManager::getInstance().getConfig().game.dungeon.transition_duration)
    // Inicialização do RNG
    , rng(rd())
{
    std::srand(std::time(nullptr));

    // Setup do overlay de transição (tela preta)
    const auto& config = ConfigManager::getInstance().getConfig();
    transitionOverlay.setSize({ (float)config.game.window_width, (float)config.game.window_height });
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

    // Reset das flags de salas especiais
    bossRoomGenerated = false;
    treasureRoomGenerated = false;

    // Fila de salas existentes que podem ter novas conexões (coordenadas)
    std::vector<sf::Vector2i> availableCoords;
    availableCoords.push_back({ 0, 0 });

    // 2. Criação da Cadeia Principal (DFS / Spanning Tree)
    // O loop deve ir até numRooms - 2 para deixar espaço para a Treasure Room e Boss Room
    while (nextAvailableRoomID < numRooms - 2 && !availableCoords.empty()) {

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
                createRoom(newRoomID, RoomType::Normal);
                coordToRoomID[nextCoord] = newRoomID;

	                // Conecta as duas salas
	                DoorDirection oppositeDir = getOppositeDirection(dir);
	                rooms.at(parentID).addDoor(dir, DoorType::Normal, doorTexture);
	                rooms.at(newRoomID).addDoor(oppositeDir, DoorType::Normal, doorTexture);
	                connectRooms(parentID, newRoomID, dir);

                // Adiciona nova sala ao pool de ramificação
                availableCoords.push_back(nextCoord);

                std::cout << "   Room " << parentID << " at (" << parentCoord.x << "," << parentCoord.y
                    << ") [" << dirToString(dir) << "] - Room " << newRoomID
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
    // Exclui as salas especiais (Boss e Treasure) da adição de portas extras
    for (int i = 0; i < numRooms - 2; ++i) {
        Room& currentRoom = rooms.at(i);

        // Define a probabilidade de tentar adicionar portas extras
        int doorsToAdd = 0;

        // Lógica de probabilidade elevada para a Sala 0
        bool isSafeZone = (i == 0);

// Decide quantas portas extra TENTAR adicionar (máximo 5 tentativas)
        const auto& config = ConfigManager::getInstance().getConfig();
        const auto& chancesConfig = config.game.dungeon.extra_door_chances;
        const std::vector<int>* chances = nullptr;

        for (const auto& chanceEntry : chancesConfig) {
            if (chanceEntry.is_safe_zone == isSafeZone) {
                chances = &chanceEntry.chances;
                break;
            }
        }

        if (chances) {
            for (int chance : *chances) {
                if (rand() % 100 < chance) doorsToAdd++;
            }
        } else {
            // Fallback para valores hardcoded se a config não for encontrada (embora devesse ser)
            // Mantido o fallback para evitar problemas, mas os valores devem ser lidos do JSON
            int baseChance2 = isSafeZone ? 100 : 70;
            int baseChance3 = isSafeZone ? 90 : 50;
            int baseChance4 = isSafeZone ? 70 : 30;
            int baseChance5 = isSafeZone ? 50 : 15;
            int baseChance6 = isSafeZone ? 20 : 5;
            if (rand() % 100 < baseChance2) doorsToAdd++;
            if (rand() % 100 < baseChance3) doorsToAdd++;
            if (rand() % 100 < baseChance4) doorsToAdd++;
            if (rand() % 100 < baseChance5) doorsToAdd++;
            if (rand() % 100 < baseChance6) doorsToAdd++;
        }

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
	                        currentRoom.addDoor(extraDir, DoorType::Normal, doorTexture);
	                        neighborRoom.addDoor(oppositeDir, DoorType::Normal, doorTexture);
	
	                        // Conecta-as
	                        connectRooms(i, neighborID, extraDir);

                        std::cout << "   Room " << i << " at (" << currentCoord.x << "," << currentCoord.y << ") [" << dirToString(extraDir)
                            << "] - Loop to Room " << neighborID << " at (" << neighborCoord.x << "," << neighborCoord.y << ")" << std::endl;
                    }
                }
            }
        }
    }


    // 4. Geração das Salas Especiais (Boss e Treasure)
    // A Boss Room deve ser a última sala a ser gerada (ID = numRooms - 1)
    // A Treasure Room deve ser a penúltima sala a ser gerada (ID = numRooms - 2)

    // Encontra uma sala para a Treasure Room (deve ser uma sala Normal)
    sf::Vector2i treasureParentCoord = { 0, 0 };
    int treasureParentID = -1;
    DoorDirection treasureDoorDir = DoorDirection::None;

    // Tenta encontrar uma sala Normal com pelo menos uma porta livre
    for (const auto& pair : coordToRoomID) {
        int roomID = pair.second;
        Room& room = rooms.at(roomID);

        if (room.getType() == RoomType::Normal) {
            std::vector<DoorDirection> availableDirs = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
            for (const auto& door : room.getDoors()) {
                availableDirs.erase(std::remove(availableDirs.begin(), availableDirs.end(), door.direction), availableDirs.end());
            }

            if (!availableDirs.empty()) {
                std::shuffle(availableDirs.begin(), availableDirs.end(), rng);
                treasureDoorDir = availableDirs[0];
                treasureParentCoord = pair.first;
                treasureParentID = roomID;
                break;
            }
        }
    }

	    if (treasureParentID != -1) {
	        int treasureRoomID = numRooms - 2;
	        sf::Vector2i treasureCoord = getNextCoord(treasureParentCoord, treasureDoorDir);
	
	        // Cria a Treasure Room
	        createRoom(treasureRoomID, RoomType::Treasure);
	        coordToRoomID[treasureCoord] = treasureRoomID;
	        treasureRoomGenerated = true;
	
	        // Conecta a Treasure Room
	        sf::Texture& doorTexture = assets.getTexture("Door");
	        DoorDirection oppositeDir = getOppositeDirection(treasureDoorDir);
	
	        rooms.at(treasureParentID).addDoor(treasureDoorDir, DoorType::Treasure, doorTexture);
	        rooms.at(treasureRoomID).addDoor(oppositeDir, DoorType::Treasure, doorTexture);
	        connectRooms(treasureParentID, treasureRoomID, treasureDoorDir);
	
	        std::cout << "   Treasure Room " << treasureRoomID << " at (" << treasureCoord.x << "," << treasureCoord.y
	            << ") connected to Room " << treasureParentID << " at (" << treasureParentCoord.x << "," << treasureParentCoord.y << ")" << std::endl;
	    }


    // Encontra uma sala para a Boss Room (deve ser uma sala Normal)
    sf::Vector2i bossParentCoord = { 0, 0 };
    int bossParentID = -1;
    DoorDirection bossDoorDir = DoorDirection::None;

    // Tenta encontrar uma sala Normal com pelo menos uma porta livre
    for (const auto& pair : coordToRoomID) {
        int roomID = pair.second;
        Room& room = rooms.at(roomID);

        if (room.getType() == RoomType::Normal) {
            std::vector<DoorDirection> availableDirs = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
            for (const auto& door : room.getDoors()) {
                availableDirs.erase(std::remove(availableDirs.begin(), availableDirs.end(), door.direction), availableDirs.end());
            }

            if (!availableDirs.empty()) {
                std::shuffle(availableDirs.begin(), availableDirs.end(), rng);
                bossDoorDir = availableDirs[0];
                bossParentCoord = pair.first;
                bossParentID = roomID;
                break;
            }
        }
    }

	    if (bossParentID != -1) {
	        int bossRoomID = numRooms - 1;
	        sf::Vector2i bossCoord = getNextCoord(bossParentCoord, bossDoorDir);
	
	        // Cria a Boss Room
	        createRoom(bossRoomID, RoomType::Boss);
	        coordToRoomID[bossCoord] = bossRoomID;
	        bossRoomGenerated = true;
	
	        // Conecta a Boss Room
	        sf::Texture& doorTexture = assets.getTexture("Door");
	        DoorDirection oppositeDir = getOppositeDirection(bossDoorDir);
	
	        // A Boss Room só pode ter uma entrada, então a sala Boss só recebe a porta
	        rooms.at(bossParentID).addDoor(bossDoorDir, DoorType::Boss, doorTexture);
	        rooms.at(bossRoomID).addDoor(oppositeDir, DoorType::Boss, doorTexture);
	        connectRooms(bossParentID, bossRoomID, bossDoorDir);
	
	        std::cout << "   Boss Room " << bossRoomID << " at (" << bossCoord.x << "," << bossCoord.y
	            << ") connected to Room " << bossParentID << " at (" << bossParentCoord.x << "," << bossParentCoord.y << ")" << std::endl;
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
    const auto& config = ConfigManager::getInstance().getConfig();
    const auto& minimapConfig = config.game.minimap;

    const float miniMapSize = minimapConfig.size;
    const float roomSize = minimapConfig.room_size;
    const float roomSpacing = minimapConfig.room_spacing;
    const float connectionThickness = minimapConfig.connection_thickness;

    // Posição do minimapa (canto superior direito)
    const sf::Vector2f miniMapPosition(window.getSize().x - miniMapSize - minimapConfig.offset_x, minimapConfig.offset_y);

    // Fundo semitransparente do minimapa
    sf::RectangleShape background;
    background.setSize({ miniMapSize, miniMapSize });
    background.setPosition(miniMapPosition);
    background.setFillColor(sf::Color(0, 0, 0, minimapConfig.background_color_alpha));
    background.setOutlineColor(sf::Color(100, 100, 100, minimapConfig.outline_color_alpha));
    background.setOutlineThickness(minimapConfig.outline_thickness);
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

                connection.setSize({ length, minimapConfig.connection_thickness_mini });
                connection.setPosition(roomPosition);
                connection.setOrigin({ 0.f, minimapConfig.connection_thickness_mini / 2.f });
                connection.setRotation(sf::degrees(angle));
                connection.setFillColor(sf::Color(80, 80, 80, minimapConfig.connection_color_alpha));

                window.draw(connection);
            }
        }

        // Desenha a sala
        sf::RectangleShape roomRect;
        roomRect.setSize({ roomSize, roomSize });
        roomRect.setOrigin({ roomSize / 2.f, roomSize / 2.f });
        roomRect.setPosition(roomPosition);

        // MODIFICADO: Define a cor baseada no estado E tipo da sala
        if (visitedRoomID == currentRoomID) {
            // Sala atual - SEMPRE vermelha (independente do tipo)
            roomRect.setFillColor(sf::Color(255, 0, 0, 255));
            roomRect.setOutlineColor(sf::Color(255, 255, 255, 255));
            roomRect.setOutlineThickness(minimapConfig.current_room_outline_thickness);
        }
        else {
            // Salas visitadas - cor baseada no TIPO
            RoomType roomType = room.getType();

            if (roomType == RoomType::Treasure) {
                // Treasure Room - AMARELA (sempre, mesmo que limpa)
                roomRect.setFillColor(sf::Color(255, 215, 0, 255)); // Dourado/Amarelo
                roomRect.setOutlineColor(sf::Color(200, 200, 0, 200));
                roomRect.setOutlineThickness(minimapConfig.cleared_room_outline_thickness);
            }
            else if (roomType == RoomType::Boss) {
                // Boss Room - PRETA (sempre, mesmo que limpa)
                roomRect.setFillColor(sf::Color(0, 0, 0, 255)); // Preto
                roomRect.setOutlineColor(sf::Color(255, 0, 0, 255)); // Contorno vermelho para destacar
                roomRect.setOutlineThickness(minimapConfig.cleared_room_outline_thickness);
            }
            else if (room.isCleared()) {
                // Sala normal limpa - branca
                roomRect.setFillColor(sf::Color(255, 255, 255, 255));
                roomRect.setOutlineColor(sf::Color(200, 200, 200, 200));
                roomRect.setOutlineThickness(minimapConfig.cleared_room_outline_thickness);
            }
            else {
                // Sala normal visitada mas não limpa - cinzenta
                roomRect.setFillColor(sf::Color(150, 150, 150, 255));
                roomRect.setOutlineColor(sf::Color(200, 200, 200, 200));
                roomRect.setOutlineThickness(minimapConfig.cleared_room_outline_thickness);
            }
        }

        window.draw(roomRect);
    }
}
