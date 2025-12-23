#include "Items.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

// --- Implementação do Item ---

Item::Item(ItemType type, sf::Vector2f pos, const sf::Texture& itemTex, const sf::Texture& altarTex)
    : type(type), position(pos), is_collected(false), bobTimer(0.0f)
{
    // 1. Configuração do Altar (Sólido)
    altarSprite.emplace(altarTex);
    altarSprite->setOrigin({ altarTex.getSize().x / 2.f, altarTex.getSize().y / 2.f });
    altarSprite->setPosition(pos);
    altarSprite->setScale({ 3.5f, 3.5f });

    // 2. Configuração do Item (Coletável)
    itemSprite.emplace(itemTex);

    // Aplicar os recortes (TextureRect) e transformações específicas
    if (type == ItemType::BLOOD_BAG) {
        itemSprite->setTextureRect({ {6, 68}, {20, 23} });
        itemSprite->setScale({ 3.2f, 3.2f });
    }
    else if (type == ItemType::ROID_RAGE) {
        itemSprite->setTextureRect({ {13, 252}, {12, 24} });
        itemSprite->setScale({ 3.2f, 3.2f });
    }
    else if (type == ItemType::SPEED_BALL) {
        itemSprite->setTextureRect({ {117, 290}, {12, 24} });
        itemSprite->setScale({ 3.2f, 3.2f });
    }
    else if (type == ItemType::EIGHT_INCH_NAIL) {
        itemSprite->setTextureRect({ {128, 39}, {32, 17} });
        itemSprite->setRotation(sf::degrees(90.f)); // Rotação apenas para o prego
        itemSprite->setScale({ 1.5f, 1.5f });        // Escala menor apenas para o prego
    }

    // 3. Origem no centro do recorte (Deve ser feito após o TextureRect)
    sf::FloatRect localBounds = itemSprite->getLocalBounds();
    itemSprite->setOrigin({ localBounds.size.x / 2.f, localBounds.size.y / 2.f });

    // 4. Posicionamento acima do altar
    itemSprite->setPosition({ pos.x, pos.y - 45.f });
}

void Item::collect() {
    is_collected = true;
}

void Item::updateAnimation(float dt) {
    if (!is_collected) {
        bobTimer += dt;
    }
}

void Item::draw(sf::RenderWindow& window) {
    // O altar é desenhado sempre
    if (altarSprite.has_value()) window.draw(*altarSprite);

    if (!is_collected && itemSprite.has_value()) {
        // Efeito de flutuação
        float offset = std::sin(bobTimer * 3.5f) * 8.0f;
        itemSprite->setPosition({ position.x, position.y - 45.f + offset });
        window.draw(*itemSprite);
    }
}

// Hitbox para COLETAR o item (Trigger)
sf::FloatRect Item::getBounds() const {
    if (itemSprite.has_value() && !is_collected) {
        sf::FloatRect bounds = itemSprite->getGlobalBounds();
        // Margem extra para facilitar a coleta já que o pedestal bloqueia o Isaac
        return sf::FloatRect({ bounds.position.x - 10.f, bounds.position.y - 10.f },
            { bounds.size.x + 20.f, bounds.size.y + 20.f });
    }
    return sf::FloatRect({ 0.f, 0.f }, { 0.f, 0.f });
}

// Hitbox sólida do Pedestal
sf::FloatRect Item::getAltarBounds() const {
    if (altarSprite.has_value()) {
        return altarSprite->getGlobalBounds();
    }
    return sf::FloatRect({ 0.f, 0.f }, { 0.f, 0.f });
}

// --- ItemManager ---

void ItemManager::spawnTreasureItem(ItemType type, sf::Vector2f position,
    const sf::Texture& itemTex, const sf::Texture& altarTex)
{
    activeItems.emplace_back(type, position, itemTex, altarTex);
}

void ItemManager::update(sf::FloatRect playerBounds, PlayerStats& stats, float deltaTime) {
    for (auto& item : activeItems) {
        item.updateAnimation(deltaTime);

        if (!item.isCollected()) {
            if (item.getBounds().findIntersection(playerBounds).has_value()) {
                switch (item.getType()) {
                case ItemType::SPEED_BALL:
                    stats.speedMultiplier *= 1.4f;
                    break;
                case ItemType::ROID_RAGE:
                    stats.hasRoidRage = true;
                    stats.damageMultiplier *= 1.5f;
                    stats.speedMultiplier *= 1.2f;
                    break;
                case ItemType::BLOOD_BAG:
                    stats.maxHPContainers += 2;
                    stats.currentHP = std::min(stats.maxHPContainers, stats.currentHP + 2);
                    break;
                case ItemType::EIGHT_INCH_NAIL:
                    stats.hasEightInchNail = true;
                    stats.damageMultiplier *= 1.3f;
                    break;
                }
                item.collect();
                std::cout << "Item coletado!" << std::endl;
            }
        }
    }
}

void ItemManager::draw(sf::RenderWindow& window) {
    for (auto& item : activeItems) item.draw(window);
}