#ifndef ITEMS_HPP
#define ITEMS_HPP

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <string>

// Estrutura para armazenar os modificadores que os itens aplicam ao Isaac
struct PlayerStats {
    float speedMultiplier = 1.0f;
    float damageMultiplier = 1.0f;
    bool hasRoidRage = false;
    bool hasEightInchNail = false;
    int maxHPContainers = 6;
    int currentHP = 6;
};

// Tipos de itens disponíveis no jogo
enum class ItemType {
    SPEED_BALL,
    ROID_RAGE,
    BLOOD_BAG,
    EIGHT_INCH_NAIL
};

class Item {
public:
    Item(ItemType type, sf::Vector2f pos, const sf::Texture& itemTex, const sf::Texture& altarTex);

    // Getters
    bool isCollected() const { return is_collected; }
    ItemType getType() const { return type; }

    // Declarações (A lógica está no .cpp para evitar erro de corpo duplicado)
    void collect();
    void updateAnimation(float dt);

    sf::FloatRect getBounds() const;
    sf::FloatRect getAltarBounds() const;
    void draw(sf::RenderWindow& window);

private:
    ItemType type;
    sf::Vector2f position;
    bool is_collected = false;

    std::optional<sf::Sprite> itemSprite;
    std::optional<sf::Sprite> altarSprite;
    float bobTimer = 0.0f;
};

class ItemManager {
public:
    // Cria um novo item num pedestal
    void spawnTreasureItem(ItemType type, sf::Vector2f position,
        const sf::Texture& itemTex, const sf::Texture& altarTex);

    // Gere a animação e a colisão com o jogador
    void update(sf::FloatRect playerBounds, PlayerStats& stats, float deltaTime);

    // Desenha todos os pedestais e itens ativos
    void draw(sf::RenderWindow& window);

private:
    std::vector<Item> activeItems;
};

#endif