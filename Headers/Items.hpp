#ifndef ITEMS_HPP
#define ITEMS_HPP

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <string>

struct PlayerStats {
    float speedMultiplier = 1.0f;
    float damageMultiplier = 1.0f;
    bool hasRoidRage = false;
    bool hasEightInchNail = false;
    int maxHPContainers = 6;
    int currentHP = 6;
};

enum class ItemType {
    SPEED_BALL,
    ROID_RAGE,
    SQUEEZY_BLOOD_BAG,
    EIGHT_INCH_NAIL
};

class Item {
public:
    Item(ItemType type, sf::Vector2f pos, const sf::Texture& itemTex, const sf::Texture& altarTex);

    bool isCollected() const { return is_collected; }
    void collect() { is_collected = true; }
    ItemType getType() const { return type; }

    // NOVO: Método para atualizar o timer de flutuação
    void updateAnimation(float dt) { bobTimer += dt; }

    sf::FloatRect getBounds() const;
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
    void spawnTreasureItem(ItemType type, sf::Vector2f position,
        const sf::Texture& itemTex, const sf::Texture& altarTex);

    void update(sf::FloatRect playerBounds, PlayerStats& stats, float deltaTime);
    void draw(sf::RenderWindow& window);

private:
    std::vector<Item> activeItems;
};

#endif