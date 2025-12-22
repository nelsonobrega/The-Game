#include "Items.hpp"
#include <iostream>
#include <cmath>

Item::Item(ItemType type, sf::Vector2f pos, const sf::Texture& itemTex, const sf::Texture& altarTex)
    : type(type), position(pos), is_collected(false), bobTimer(0.0f) {

    // Altar
    altarSprite.emplace(altarTex);
    altarSprite->setOrigin({ altarTex.getSize().x / 2.f, altarTex.getSize().y / 2.f });
    altarSprite->setPosition(pos);
    altarSprite->setScale({ 3.5f, 3.5f });

    // Item
    itemSprite.emplace(itemTex);
    itemSprite->setOrigin({ itemTex.getSize().x / 2.f, itemTex.getSize().y / 2.f });
    itemSprite->setPosition({ pos.x, pos.y - 35.f });
    itemSprite->setScale({ 1.5f, 1.5f });
}

void Item::draw(sf::RenderWindow& window) {
    if (altarSprite.has_value()) {
        window.draw(*altarSprite);
    }

    if (!is_collected && itemSprite.has_value()) {
        // Efeito visual: o item flutua para cima e para baixo
        float offset = std::sin(bobTimer * 3.0f) * 5.0f;
        itemSprite->setPosition({ position.x, position.y - 35.f + offset });
        window.draw(*itemSprite);
    }
}

sf::FloatRect Item::getBounds() const {
    if (itemSprite.has_value() && !is_collected) {
        return itemSprite->getGlobalBounds();
    }
    return sf::FloatRect({ 0, 0 }, { 0, 0 });
}

// --- ItemManager ---

void ItemManager::spawnTreasureItem(ItemType type, sf::Vector2f position,
    const sf::Texture& itemTex, const sf::Texture& altarTex) {
    activeItems.emplace_back(type, position, itemTex, altarTex);
}

void ItemManager::update(sf::FloatRect playerBounds, PlayerStats& stats, float deltaTime) {
    for (auto it = activeItems.begin(); it != activeItems.end(); ++it) {

        // Atualiza a animação de flutuar de cada item
        it->updateAnimation(deltaTime);

        if (!it->isCollected()) {
            if (it->getBounds().findIntersection(playerBounds).has_value()) {

                switch (it->getType()) {
                case ItemType::SPEED_BALL:
                    stats.speedMultiplier *= 1.4f;
                    break;
                case ItemType::ROID_RAGE:
                    stats.hasRoidRage = true;
                    stats.damageMultiplier *= 1.5f;
                    stats.speedMultiplier *= 1.2f;
                    break;
                case ItemType::SQUEEZY_BLOOD_BAG:
                    stats.maxHPContainers += 2;
                    stats.currentHP = std::min(stats.maxHPContainers, stats.currentHP + 2);
                    break;
                case ItemType::EIGHT_INCH_NAIL:
                    stats.hasEightInchNail = true;
                    stats.damageMultiplier *= 1.3f;
                    break;
                }
                it->collect();
                std::cout << "Item coletado!" << std::endl;
            }
        }
    }
}

void ItemManager::draw(sf::RenderWindow& window) {
    for (auto& item : activeItems) {
        item.draw(window);
    }
}