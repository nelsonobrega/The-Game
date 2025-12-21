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
    std::srand((unsigned int)std::time(nullptr));
    const auto& config = ConfigManager::getInstance().getConfig();
    transitionOverlay.setSize({ (float)config.game.window_width, (float)config.game.window_height });
    transitionOverlay.setFillColor(sf::Color(0, 0, 0, 0));
}

sf::Vector2i getNextCoord(const sf::Vector2i& current, DoorDirection direction) {
    if (direction == DoorDirection::North) return { current.x, current.y - 1 };
    if (direction == DoorDirection::South) return { current.x, current.y + 1 };
    if (direction == DoorDirection::East)  return { current.x + 1, current.y };
    if (direction == DoorDirection::West)  return { current.x - 1, current.y };
    return current;
}

void RoomManager::generateDungeon(int numRooms) {
    rooms.clear();
    coordToRoomID.clear();
    visitedRooms.clear();
    sf::Texture& doorTexture = assets.getTexture("Door");
    int nextAvailableRoomID = 0;

    createRoom(nextAvailableRoomID, RoomType::SafeZone);
    coordToRoomID[{0, 0}] = nextAvailableRoomID++;

    std::vector<sf::Vector2i> availableCoords = { {0, 0} };

    while (nextAvailableRoomID < numRooms - 2 && !availableCoords.empty()) {
        std::uniform_int_distribution<> coordDist(0, (int)availableCoords.size() - 1);
        sf::Vector2i parentCoord = availableCoords[coordDist(rng)];
        int parentID = coordToRoomID[parentCoord];

        std::vector<DoorDirection> dirs = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
        std::shuffle(dirs.begin(), dirs.end(), rng);

        bool roomAdded = false;
        for (DoorDirection dir : dirs) {
            sf::Vector2i nextCoord = getNextCoord(parentCoord, dir);
            if (coordToRoomID.find(nextCoord) == coordToRoomID.end()) {
                createRoom(nextAvailableRoomID, RoomType::Normal);
                coordToRoomID[nextCoord] = nextAvailableRoomID;
                rooms.at(parentID).addDoor(dir, DoorType::Normal, doorTexture);
                rooms.at(nextAvailableRoomID).addDoor(getOppositeDirection(dir), DoorType::Normal, doorTexture);
                connectRooms(parentID, nextAvailableRoomID, dir);
                availableCoords.push_back(nextCoord);
                nextAvailableRoomID++;
                roomAdded = true;
                break;
            }
        }
        if (!roomAdded && parentID != 0)
            availableCoords.erase(std::remove(availableCoords.begin(), availableCoords.end(), parentCoord), availableCoords.end());
    }

    auto setupSpecial = [&](RoomType rType, int rID, DoorType dType) {
        for (auto const& [coord, id] : coordToRoomID) {
            if (rooms.at(id).getType() != RoomType::Normal) continue;
            std::vector<DoorDirection> av = { DoorDirection::North, DoorDirection::South, DoorDirection::East, DoorDirection::West };
            for (auto const& d : rooms.at(id).getDoors())
                av.erase(std::remove(av.begin(), av.end(), d.direction), av.end());

            if (!av.empty()) {
                std::shuffle(av.begin(), av.end(), rng);
                createRoom(rID, rType);
                coordToRoomID[getNextCoord(coord, av[0])] = rID;
                rooms.at(id).addDoor(av[0], dType, doorTexture);
                rooms.at(rID).addDoor(getOppositeDirection(av[0]), dType, doorTexture);
                connectRooms(id, rID, av[0]);
                return true;
            }
        }
        return false;
        };

    setupSpecial(RoomType::Treasure, numRooms - 2, DoorType::Treasure);
    setupSpecial(RoomType::Boss, numRooms - 1, DoorType::Boss);

    currentRoomID = 0;
    currentRoom = &rooms.at(0);
    currentRoom->openDoors();
}

void RoomManager::createRoom(int id, RoomType type) {
    rooms.emplace(id, Room(id, type, gameBounds));
    std::vector<sf::IntRect> vars = { {{0, 0}, {234, 156}}, {{0, 156}, {234, 156}}, {{234, 0}, {234, 156}} };
    if (type == RoomType::Boss) rooms.at(id).setCornerTextureRect({ {234, 156}, {234, 156} });
    else rooms.at(id).setCornerTextureRect(vars[std::uniform_int_distribution<>(0, 2)(rng)]);
}

void RoomManager::connectRooms(int roomA, int roomB, DoorDirection dirA) {
    rooms.at(roomA).connectDoor(dirA, roomB);
    rooms.at(roomB).connectDoor(getOppositeDirection(dirA), roomA);
}

DoorDirection RoomManager::getOppositeDirection(DoorDirection dir) {
    if (dir == DoorDirection::North) return DoorDirection::South;
    if (dir == DoorDirection::South) return DoorDirection::North;
    if (dir == DoorDirection::East) return DoorDirection::West;
    if (dir == DoorDirection::West) return DoorDirection::East;
    return DoorDirection::None;
}

DoorDirection RoomManager::checkPlayerAtDoor(const sf::FloatRect& pBounds) {
    if (!currentRoom || !currentRoom->isCleared()) return DoorDirection::None;
    for (auto const& d : currentRoom->getDoors())
        if (d.isOpen && d.leadsToRoomID != -1 && checkCollision(pBounds, d.bounds)) return d.direction;
    return DoorDirection::None;
}

void RoomManager::requestTransition(DoorDirection dir) {
    if (transitionState != TransitionState::None) return;
    nextRoomID = currentRoom->getDoorLeadsTo(dir);
    if (nextRoomID == -1) return;
    currentRoom->closeDoors();
    transitionDirection = dir;
    transitionState = TransitionState::FadingOut;
    transitionProgress = 0.f;
}

void RoomManager::updateTransition(float deltaTime, sf::Vector2f& pPos) {
    if (transitionState == TransitionState::None) return;
    transitionProgress = std::min(1.f, transitionProgress + (deltaTime / transitionDuration));

    if (transitionState == TransitionState::FadingOut) {
        transitionOverlay.setFillColor(sf::Color(0, 0, 0, (int)(255 * transitionProgress)));
        if (transitionProgress >= 1.f) {
            currentRoomID = nextRoomID;
            currentRoom = &rooms.at(currentRoomID);
            if (!currentRoom->isCleared()) currentRoom->closeDoors();

            // SPAWN COM CHUBBY: Usamos a mesma textura "ChubbySheet" para corpo e projétil
            currentRoom->spawnEnemies(
                assets.getAnimationSet("D_Down"), assets.getAnimationSet("D_Up"),
                assets.getAnimationSet("D_Left"), assets.getAnimationSet("D_Right"),
                assets.getTexture("TearAtlas"), assets.getAnimationSet("Bishop"),
                assets.getTexture("ChubbySheet"),
                assets.getTexture("ChubbySheet")
            );

            pPos = currentRoom->getPlayerSpawnPosition(getOppositeDirection(transitionDirection));
            transitionState = TransitionState::FadingIn;
            transitionProgress = 0.f;
        }
    }
    else {
        transitionOverlay.setFillColor(sf::Color(0, 0, 0, (int)(255 * (1.f - transitionProgress))));
        if (transitionProgress >= 1.f) { transitionState = TransitionState::None; transitionOverlay.setFillColor(sf::Color::Transparent); }
    }
}

void RoomManager::update(float dt, sf::Vector2f pPos) { if (currentRoom) currentRoom->update(dt, pPos); }
void RoomManager::draw(sf::RenderWindow& win) { if (currentRoom) currentRoom->draw(win); }
void RoomManager::drawTransitionOverlay(sf::RenderWindow& win) { if (transitionState != TransitionState::None) win.draw(transitionOverlay); }

sf::Vector2i RoomManager::getCurrentRoomCoord() const {
    for (auto const& [coord, id] : coordToRoomID) if (id == currentRoomID) return coord;
    return { 0,0 };
}

void RoomManager::drawMiniMap(sf::RenderWindow& win) {
    const auto& m = ConfigManager::getInstance().getConfig().game.minimap;
    sf::Vector2f pos(win.getSize().x - m.size - m.offset_x, m.offset_y);
    sf::RectangleShape bg({ m.size, m.size }); bg.setPosition(pos); bg.setFillColor({ 0,0,0,(uint8_t)m.background_color_alpha }); win.draw(bg);

    visitedRooms.insert(currentRoomID);
    sf::Vector2i curC = getCurrentRoomCoord();
    sf::Vector2f center = pos + sf::Vector2f(m.size / 2.f, m.size / 2.f);

    for (int id : visitedRooms) {
        sf::Vector2i rC; bool found = false;
        for (auto const& [coord, rid] : coordToRoomID) if (rid == id) { rC = coord; found = true; break; }
        if (!found) continue;

        sf::Vector2f rP = center + sf::Vector2f((rC.x - curC.x) * m.room_spacing, (rC.y - curC.y) * m.room_spacing);
        sf::RectangleShape rr({ (float)m.room_size, (float)m.room_size });
        rr.setOrigin({ m.room_size / 2.f, m.room_size / 2.f });
        rr.setPosition(rP);

        if (id == currentRoomID) rr.setFillColor(sf::Color::Red);
        else if (rooms.at(id).getType() == RoomType::Treasure) rr.setFillColor(sf::Color::Yellow);
        else if (rooms.at(id).getType() == RoomType::Boss) rr.setFillColor(sf::Color::Black);
        else rr.setFillColor(rooms.at(id).isCleared() ? sf::Color::White : sf::Color(150, 150, 150));
        win.draw(rr);
    }
}