#include "Rooms.hpp"
#include <iostream>
#include <utility> // Para std::move

Room::Room(RoomType type, int x, int y)
    : type(type), mapX(x), mapY(y), assets(AssetManager::getInstance())
{
    buildRoomLayout();
}

void Room::addTile(std::vector<sf::Sprite>& tileVector, const sf::IntRect& rect, float x, float y) {
    try {
        // CORREÇÃO: Usa const_cast para aceder à textura.
        sf::Texture& sheet = const_cast<sf::Texture&>(assets.getTexture("BasementCorner"));

        sf::Sprite tile(sheet);
        tile.setTextureRect(rect);

        tile.setPosition({ x, y });

        // Calcula a escala: TILE_EFFECTIVE_SIZE / Tamanho_Original_no_Rect
        const float scaleX = TILE_EFFECTIVE_SIZE_X / (float)rect.size.x;
        const float scaleY = TILE_EFFECTIVE_SIZE_Y / (float)rect.size.y;
        tile.setScale({ scaleX, scaleY });

        tileVector.push_back(std::move(tile));
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERRO: Falha ao adicionar tile. " << e.what() << std::endl;
    }
}

void Room::buildRoomLayout() {
    // Tamanho do tile em pixels no ecrã
    const float T_X = TILE_EFFECTIVE_SIZE_X;
    const float T_Y = TILE_EFFECTIVE_SIZE_Y;

    // --- 1. CHÃO CENTRAL ---
    try {
        const sf::IntRect& floorRect = assets.getTextureRect("Basement_floor_simple");

        // Loop para preencher o chão central (de 3 a TILES_X/Y - 3)
        for (int y = 3; y < TILES_Y - 3; ++y) {
            for (int x = 3; x < TILES_X - 3; ++x) {
                addTile(floorTiles, floorRect, x * T_X, y * T_Y);
            }
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERRO: Falha ao carregar rect do chão: " << e.what() << std::endl;
    }

    // --- 2. PAREDES SUPERIOR e INFERIOR ---
    try {
        const sf::IntRect& wallTopRect = assets.getTextureRect("Basement_wall_top");
        const sf::IntRect& wallBottomRect = assets.getTextureRect("Basement_wall_bottom");

        for (int x = 0; x < TILES_X; ++x) {
            // Parede Superior (Linha 2)
            addTile(wallTiles, wallTopRect, x * T_X, 2 * T_Y);
            // Parede Inferior (Linha TILES_Y - 3)
            addTile(wallTiles, wallBottomRect, x * T_X, (TILES_Y - 3) * T_Y);
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERRO: Falha ao carregar rects das paredes horizontais: " << e.what() << std::endl;
    }

    // --- 3. PAREDES LATERAIS ---
    try {
        const sf::IntRect& wallSideRect = assets.getTextureRect("Basement_wall_side");
        // Obter a sheet para criar a sprite da parede direita invertida
        sf::Texture& sheet = const_cast<sf::Texture&>(assets.getTexture("BasementCorner"));

        for (int y = 3; y < TILES_Y - 3; ++y) {
            // Parede Esquerda (Coluna 2)
            addTile(wallTiles, wallSideRect, 2 * T_X, y * T_Y);

            // Parede Direita (Coluna TILES_X - 3) - Criada separadamente para inverter
            sf::Sprite wallRight(sheet);
            wallRight.setTextureRect(wallSideRect);

            const float scaleX = TILE_EFFECTIVE_SIZE_X / (float)wallSideRect.size.x;
            const float scaleY = TILE_EFFECTIVE_SIZE_Y / (float)wallSideRect.size.y;

            // Inverte a sprite ao definir uma escala X negativa
            wallRight.setScale({ -scaleX, scaleY });
            wallRight.setPosition({ (TILES_X - 2) * T_X, y * T_Y });
            wallTiles.push_back(std::move(wallRight));
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERRO: Falha ao carregar rects das paredes laterais: " << e.what() << std::endl;
    }
}


void Room::update(sf::Time deltaTime) {
    // Lógica de atualização da sala
}

void Room::draw(sf::RenderWindow& window) {
    // Desenhar o Chão
    for (auto& tile : floorTiles) {
        window.draw(tile);
    }

    // Desenhar as Paredes (por cima do chão)
    for (auto& tile : wallTiles) {
        window.draw(tile);
    }
}