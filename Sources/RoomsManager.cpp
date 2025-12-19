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
    , rng(rd())
{
    std::srand(std::time(nullptr));

    const auto& config = ConfigManager::getInstance().getConfig();
    transitionOverlay.setSize({ (float)config.game.window_width, (float)config.game.window_height });
    transitionOverlay.setFillColor(sf::Color(0, 0, 0, 0));
}

static std::string dirToString(DoorDirection direction) {
    switch (direction) {
    case DoorDirection::North: return "North";
    case DoorDirection::South: return "South";
    case DoorDirection::East: return "East";
    case DoorDirection::West: return "West";
    default: return "None";
    }
}

sf::Vector2i getNextCoord(const sf::Vector2i& current, DoorDirection direction) {
    switch (direction) {
    case DoorDirection::North: return { current.x, current.y - 1 };
    case DoorDirection::South: return { current.x, current.y + 1 };
    case DoorDirection::East: return { current.x + 1, current.y };
    case DoorDirection::West: return { current.x - 1, current.y };
    default: return current;
    }
}

void RoomManager::generateDungeon(int numRooms) {
    std::cout << "\n Generating dungeon with " << numRooms << " rooms..." << std::endl;

    rooms.clear();
    coordToRoomID.clear();
    visitedRooms.clear();

    sf::Texture& doorTexture = assets.getTexture("Door");
    int nextAvailableRoomID = 0;

    // 1. Cria Sala Inicial
    createRoom(nextAvailableRoomID, RoomType::SafeZone);
    coordToRoomID[{0, 0}] = nextAvailableRoomID;
    nextAvailableRoomID++;

    bossRoomGenerated = false;
    treasureRoomGenerated = false;

    std::vector<sf::Vector2i> availableCoords;
    availableCoords.push_back({ 0, 0 });

    // 2. Cadeia Principal
    while (nextAvailableRoomID < numRooms - 2 && !availableCoords.empty()) {
        std::uniform_int_distribution<> coordDist(0, availableCoords.size() - 1);
        sf::Vector2i parentCoord = availableCoords[coordDist(rng)];
        int parentID = coordToRoomID[parentCoord];

        std::vector<DoorDirection> potentialDirections = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
        std::shuffle(potentialDirections.begin(), potentialDirections.end(), rng);

        bool roomAdded = false;
        for (DoorDirection dir : potentialDirections) {
            sf::Vector2i nextCoord = getNextCoord(parentCoord, dir);

            if (coordToRoomID.find(nextCoord) == coordToRoomID.end() && nextAvailableRoomID < numRooms) {
                int newRoomID = nextAvailableRoomID;
                createRoom(newRoomID, RoomType::Normal);
                coordToRoomID[nextCoord] = newRoomID;

                DoorDirection oppositeDir = getOppositeDirection(dir);
                rooms.at(parentID).addDoor(dir, DoorType::Normal, doorTexture);
                rooms.at(newRoomID).addDoor(oppositeDir, DoorType::Normal, doorTexture);
                connectRooms(parentID, newRoomID, dir);

                availableCoords.push_back(nextCoord);
                nextAvailableRoomID++;
                roomAdded = true;
                break;
            }
        }
        if (!roomAdded && parentID != 0) {
            availableCoords.erase(std::remove(availableCoords.begin(), availableCoords.end(), parentCoord), availableCoords.end());
        }
    }

    // 3. Adiciona Conexões Extras (Loops)
    for (int i = 0; i < numRooms - 2; ++i) {
        Room& currentRoomRef = rooms.at(i);
        bool isSafeZone = (i == 0);
        int doorsToAdd = 0;

        const auto& config = ConfigManager::getInstance().getConfig();
        for (const auto& chanceEntry : config.game.dungeon.extra_door_chances) {
            if (chanceEntry.is_safe_zone == isSafeZone) {
                for (int chance : chanceEntry.chances) {
                    if (rand() % 100 < chance) doorsToAdd++;
                }
                break;
            }
        }

        for (int j = 0; j < doorsToAdd; ++j) {
            if (currentRoomRef.getDoors().size() >= 4) break;
            std::vector<DoorDirection> availableDirs = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
            for (const auto& door : currentRoomRef.getDoors()) {
                availableDirs.erase(std::remove(availableDirs.begin(), availableDirs.end(), door.direction), availableDirs.end());
            }
            if (availableDirs.empty()) break;

            std::shuffle(availableDirs.begin(), availableDirs.end(), rng);
            DoorDirection extraDir = availableDirs[0];

            sf::Vector2i currentCoord;
            bool found = false;
            for (const auto& pair : coordToRoomID) { if (pair.second == i) { currentCoord = pair.first; found = true; break; } }
            if (!found) continue;

            sf::Vector2i neighborCoord = getNextCoord(currentCoord, extraDir);
            if (coordToRoomID.count(neighborCoord)) {
                int neighborID = coordToRoomID.at(neighborCoord);
                if (neighborID == i) continue;
                if (!currentRoomRef.hasDoor(extraDir)) {
                    Room& neighborRoom = rooms.at(neighborID);
                    DoorDirection oppositeDir = getOppositeDirection(extraDir);
                    if (!neighborRoom.hasDoor(oppositeDir)) {
                        currentRoomRef.addDoor(extraDir, DoorType::Normal, doorTexture);
                        neighborRoom.addDoor(oppositeDir, DoorType::Normal, doorTexture);
                        connectRooms(i, neighborID, extraDir);
                    }
                }
            }
        }
    }

    // 4. Salas Especiais (Treasure & Boss)
    auto setupSpecialRoom = [&](RoomType rType, int rID, DoorType dType) {
        sf::Vector2i parentCoord;
        int parentID = -1;
        DoorDirection doorDir;

        for (const auto& pair : coordToRoomID) {
            Room& r = rooms.at(pair.second);
            if (r.getType() == RoomType::Normal) {
                std::vector<DoorDirection> av = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
                for (const auto& d : r.getDoors()) av.erase(std::remove(av.begin(), av.end(), d.direction), av.end());
                if (!av.empty()) {
                    std::shuffle(av.begin(), av.end(), rng);
                    doorDir = av[0]; parentCoord = pair.first; parentID = pair.second; break;
                }
            }
        }

        if (parentID != -1) {
            sf::Vector2i specialCoord = getNextCoord(parentCoord, doorDir);
            createRoom(rID, rType);
            coordToRoomID[specialCoord] = rID;
            rooms.at(parentID).addDoor(doorDir, dType, doorTexture);
            rooms.at(rID).addDoor(getOppositeDirection(doorDir), dType, doorTexture);
            connectRooms(parentID, rID, doorDir);
            return true;
        }
        return false;
        };

    treasureRoomGenerated = setupSpecialRoom(RoomType::Treasure, numRooms - 2, DoorType::Treasure);
    bossRoomGenerated = setupSpecialRoom(RoomType::Boss, numRooms - 1, DoorType::Boss);

    currentRoomID = 0;
    currentRoom = &rooms.at(currentRoomID);
    currentRoom->openDoors();
}

void RoomManager::createRoom(int id, RoomType type) {
    rooms.emplace(id, Room(id, type, gameBounds));

    // MEMÓRIA VISUAL: Sorteia os cantos uma vez na criação
    std::vector<sf::IntRect> variants = {
        sf::IntRect({0, 0}, {234, 156}),
        sf::IntRect({0, 156}, {234, 156}),
        sf::IntRect({234, 0}, {234, 156})
    };
    std::uniform_int_distribution<> dist(0, variants.size() - 1);

    if (type == RoomType::Boss) rooms.at(id).setCornerTextureRect(sf::IntRect({ 234, 156 }, { 234, 156 }));
    else rooms.at(id).setCornerTextureRect(variants[dist(rng)]);
}

void RoomManager::connectRooms(int roomA, int roomB, DoorDirection directionFromA) {
    rooms.at(roomA).connectDoor(directionFromA, roomB);
    rooms.at(roomB).connectDoor(getOppositeDirection(directionFromA), roomA);
}

DoorDirection RoomManager::getOppositeDirection(DoorDirection direction) {
    switch (direction) {
    case DoorDirection::North: return DoorDirection::South;
    case DoorDirection::South: return DoorDirection::North;
    case DoorDirection::East: return DoorDirection::West;
    case DoorDirection::West: return DoorDirection::East;
    default: return DoorDirection::None;
    }
}

DoorDirection RoomManager::checkPlayerAtDoor(const sf::FloatRect& playerBounds) {
    if (!currentRoom || !currentRoom->isCleared()) return DoorDirection::None;
    for (const auto& door : currentRoom->getDoors()) {
        if (door.isOpen && door.leadsToRoomID != -1 && checkCollision(playerBounds, door.bounds)) return door.direction;
    }
    return DoorDirection::None;
}

void RoomManager::requestTransition(DoorDirection direction) {
    if (transitionState != TransitionState::None) return;
    int target = currentRoom->getDoorLeadsTo(direction);
    if (target == -1) return;
    currentRoom->closeDoors();
    nextRoomID = target;
    transitionDirection = direction;
    transitionState = TransitionState::FadingOut;
    transitionProgress = 0.f;
}

void RoomManager::updateTransition(float deltaTime, sf::Vector2f& playerPosition) {
    if (transitionState == TransitionState::None) return;
    transitionProgress = std::min(1.f, transitionProgress + (deltaTime / transitionDuration));

    if (transitionState == TransitionState::FadingOut) {
        transitionOverlay.setFillColor(sf::Color(0, 0, 0, (int)(255 * transitionProgress)));
        if (transitionProgress >= 1.f) {
            currentRoomID = nextRoomID;
            currentRoom = &rooms.at(currentRoomID);
            if (!currentRoom->isCleared()) currentRoom->closeDoors();
            currentRoom->spawnEnemies(assets.getAnimationSet("D_Down"), assets.getAnimationSet("D_Up"), assets.getAnimationSet("D_Left"), assets.getAnimationSet("D_Right"), assets.getTexture("TearAtlas"), assets.getAnimationSet("Bishop"));
            playerPosition = currentRoom->getPlayerSpawnPosition(getOppositeDirection(transitionDirection));
            transitionState = TransitionState::FadingIn;
            transitionProgress = 0.f;
        }
    }
    else if (transitionState == TransitionState::FadingIn) {
        transitionOverlay.setFillColor(sf::Color(0, 0, 0, (int)(255 * (1.f - transitionProgress))));
        if (transitionProgress >= 1.f) { transitionState = TransitionState::None; transitionOverlay.setFillColor(sf::Color::Transparent); }
    }
}

void RoomManager::update(float deltaTime, sf::Vector2f playerPosition) { if (currentRoom) currentRoom->update(deltaTime, playerPosition); }
void RoomManager::draw(sf::RenderWindow& window) { if (currentRoom) currentRoom->draw(window); }
void RoomManager::drawTransitionOverlay(sf::RenderWindow& window) { if (transitionState != TransitionState::None) window.draw(transitionOverlay); }
sf::Vector2i RoomManager::getCurrentRoomCoord() const {
    for (const auto& pair : coordToRoomID) if (pair.second == currentRoomID) return pair.first;
    return { 0,0 };
}

void RoomManager::drawMiniMap(sf::RenderWindow& window) {
    const auto& m = ConfigManager::getInstance().getConfig().game.minimap;
    sf::Vector2f pos(window.getSize().x - m.size - m.offset_x, m.offset_y);
    sf::RectangleShape bg({ m.size, m.size }); bg.setPosition(pos); bg.setFillColor(sf::Color(0, 0, 0, m.background_color_alpha)); window.draw(bg);

    visitedRooms.insert(currentRoomID);
    sf::Vector2i curC = getCurrentRoomCoord();
    sf::Vector2f center = pos + sf::Vector2f(m.size / 2.f, m.size / 2.f);

    for (int id : visitedRooms) {
        sf::Vector2i rC; bool f = false;
        for (const auto& p : coordToRoomID) if (p.second == id) { rC = p.first; f = true; break; }
        if (!f) continue;

        sf::Vector2f rP = center + sf::Vector2f((rC.x - curC.x) * m.room_spacing, (rC.y - curC.y) * m.room_spacing);
        sf::RectangleShape rr(sf::Vector2f(m.room_size, m.room_size));
        rr.setOrigin(sf::Vector2f(m.room_size / 2.f, m.room_size / 2.f));
        rr.setPosition(rP);

        if (id == currentRoomID) rr.setFillColor(sf::Color::Red);
        else if (rooms.at(id).getType() == RoomType::Treasure) rr.setFillColor(sf::Color::Yellow);
        else if (rooms.at(id).getType() == RoomType::Boss) rr.setFillColor(sf::Color::Black);
        else rr.setFillColor(rooms.at(id).isCleared() ? sf::Color::White : sf::Color(150, 150, 150));

        window.draw(rr);
    }
}