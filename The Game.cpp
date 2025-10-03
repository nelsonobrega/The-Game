#include "SFML/Graphics.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <stdexcept>

#include "Headers/AssetManager.hpp" 
#include "Headers/player.hpp" 
#include "Headers/enemy.hpp" 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


enum class GameState {
    menu,
    playing,
    exiting,
};

void loadGameAssets() {
    AssetManager& assets = AssetManager::getInstance();

    assets.loadAnimation("I_Down", "Front_Isaac", "F", 9, "V1.png");
    assets.loadAnimation("I_Up", "Back_Isaac", "B", 9, "V1.png");
    assets.loadAnimation("I_Left", "Left_Isaac", "L", 6, "V1.png");
    assets.loadAnimation("I_Right", "Right_Isaac", "R", 6, "V1.png");
    assets.loadTexture("Hit", "Images/Hit.png");

    assets.loadAnimation("D_Down", "Front_Demon", "F", 8, "D.png");
    assets.loadAnimation("D_Up", "Back_Demon", "B", 8, "D.png");
    assets.loadAnimation("D_Left", "Left_Demon", "L", 8, "D.png");
    assets.loadAnimation("D_Right", "Right_Demon", "R", 8, "D.png");
    assets.loadTexture("EnemyHit", "Images/Enemy_Knife.png");

    assets.loadAnimation("Bishop", "Bishop", "B", 14, ".png");

    assets.loadTexture("HeartF", "Images/UI/Life/Full.png");
    assets.loadTexture("HeartH", "Images/UI/Life/Half.png");
    assets.loadTexture("HeartE", "Images/UI/Life/Empty.png");

    assets.loadTexture("BasementCorner", "Images/Background/Basement_sheet.png");
}


int main()
{
    try {
        sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "Isaac test");
        GameState currentState = GameState::menu;

        sf::Texture pButton;
        pButton.loadFromFile("Images/playButton.png");
        sf::Sprite playButton(pButton);
        playButton.setPosition({ 150.0f, 170.0f });
        playButton.setScale(sf::Vector2f(0.5, 0.5));

        sf::Texture eButton;
        eButton.loadFromFile("Images/exitButton.png");
        sf::Sprite exitButton(eButton);
        exitButton.setPosition({ 150.0f, 750.0f });
        exitButton.setScale(sf::Vector2f(0.4900965, 0.537958));

        sf::Texture menuTexture;
        if (!menuTexture.loadFromFile("Images/backgroundMenu.png")) return -1;
        sf::Sprite menuGround(menuTexture);
        menuGround.setScale({ 1.25f, 1.05571847f });

        while (window.isOpen() and currentState == GameState::menu) {
            auto mousePosition = sf::Vector2f(sf::Mouse::getPosition(window));
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>()) window.close();
            }

            if (playButton.getGlobalBounds().contains(mousePosition) and (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)))
            {
                currentState = GameState::playing;
            }
            if (exitButton.getGlobalBounds().contains(mousePosition) and (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)))
            {
                currentState = GameState::exiting;
            }

            window.clear();
            window.draw(menuGround);
            window.draw(playButton);
            window.draw(exitButton);
            window.display();
        }

        if (currentState == GameState::exiting) return 0;

        loadGameAssets();
        AssetManager& assets = AssetManager::getInstance();

        Player_ALL Isaac(
            assets.getAnimationSet("I_Down"),
            assets.getTexture("Hit"));

        Isaac.textures_walk_up = &assets.getAnimationSet("I_Up");
        Isaac.textures_walk_left = &assets.getAnimationSet("I_Left");
        Isaac.textures_walk_right = &assets.getAnimationSet("I_Right");

        Demon_ALL Enemy(
            assets.getAnimationSet("D_Down"),
            assets.getAnimationSet("D_Up"),
            assets.getAnimationSet("D_Left"),
            assets.getAnimationSet("D_Right"),
            assets.getTexture("EnemyHit"));

        Bishop_ALL eBishop(assets.getAnimationSet("Bishop"));

        sf::Sprite heartSpriteF(assets.getTexture("HeartF"));
        sf::Sprite heartSpriteH(assets.getTexture("HeartH"));
        sf::Sprite heartSpriteE(assets.getTexture("HeartE"));

        sf::Texture& Basement_Corner = assets.getTexture("BasementCorner");

        float scaleX = 960.f / 234.f;
        float scaleY = 540.f / 156.f;

        sf::Sprite cornerTL(Basement_Corner);
        cornerTL.setTextureRect(sf::IntRect({ 0, 156 }, { 234, 311 }));
        cornerTL.setPosition({ 0.f, 0.f });
        cornerTL.setScale({ scaleX, scaleY });

        sf::Sprite cornerTR(Basement_Corner);
        cornerTR.setTextureRect(sf::IntRect({ 0, 156 }, { 234, 311 }));
        cornerTR.setPosition({ 1920.f, 0.f });
        cornerTR.setScale({ -scaleX, scaleY });

        sf::Sprite cornerBL(Basement_Corner);
        cornerBL.setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
        cornerBL.setPosition({ 0.f, 1080.f });
        cornerBL.setScale({ scaleX, -scaleY - 0.1f });

        sf::Sprite cornerBR(Basement_Corner);
        cornerBR.setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
        cornerBR.setPosition({ 1920.f, 1080.f });
        cornerBR.setScale({ -scaleX, -scaleY - 0.1f });

        float left = 213.33f;
        float top = 179.80f;
        float width = 1493.34f;
        float height = 720.40f;
        sf::FloatRect gameBounds({ left, top }, { width, height });

        sf::Clock clock;

        while (window.isOpen())
        {
            sf::Time deltaTime = clock.restart();

            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>()) window.close();
            }

            Isaac.update(deltaTime.asSeconds(), gameBounds);
            Enemy.update(deltaTime.asSeconds(), Isaac.getPosition(), gameBounds);
            eBishop.update(deltaTime.asSeconds(), Isaac.getPosition(), gameBounds);

            if (eBishop.shouldHealDemon()) {
                if (Enemy.getHealth() > 0) {
                    Enemy.heal(2);
                    eBishop.resetHealFlag();
                }
            }

            std::vector<Projectile>& isaacProjectiles = Isaac.getProjectiles();
            for (auto it = isaacProjectiles.begin(); it != isaacProjectiles.end(); )
            {
                sf::FloatRect projBounds = it->sprite.getGlobalBounds();
                if (Enemy.getHealth() > 0 && checkCollision(projBounds, Enemy.getGlobalBounds()))
                {
                    Enemy.takeDamage(1);
                    it = isaacProjectiles.erase(it);
                }
                else { ++it; }
            }

            std::vector<EnemyProjectile>& enemyProjectiles = Enemy.getProjectiles();
            for (auto it = enemyProjectiles.begin(); it != enemyProjectiles.end(); )
            {
                sf::FloatRect projBounds = it->sprite.getGlobalBounds();
                if (Isaac.getHealth() > 0 && checkCollision(projBounds, Isaac.getGlobalBounds()))
                {
                    Isaac.takeDamage(2);
                    it = enemyProjectiles.erase(it);
                }
                else { ++it; }
            }

            if (Enemy.getHealth() <= 0 || Isaac.getHealth() <= 0)
                window.close();

            window.clear();
            window.draw(cornerTL);
            window.draw(cornerTR);
            window.draw(cornerBL);
            window.draw(cornerBR);

            int currentHealth = Isaac.getHealth();
            float xPosition = 0.f + 200.f;
            float yPosition = 90.f;
            const int maxHearts = 3;
            const float heartSpacing = 67.f;

            heartSpriteF.setScale({ 3.f, 3.f });
            heartSpriteH.setScale({ 3.f, 3.f });
            heartSpriteE.setScale({ 3.f, 3.f });

            for (int i = 0; i < maxHearts; ++i)
            {
                const sf::Sprite* spriteToDraw;
                if (currentHealth >= 2) spriteToDraw = &heartSpriteF;
                else if (currentHealth == 1) spriteToDraw = &heartSpriteH;
                else spriteToDraw = &heartSpriteE;

                const_cast<sf::Sprite*>(spriteToDraw)->setPosition({ xPosition, yPosition });
                window.draw(*spriteToDraw);
                xPosition += heartSpacing;
                currentHealth -= 2;
            }

            Enemy.draw(window);
            Isaac.draw(window);
            eBishop.draw(window);

            window.display();
        }

    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL NA EXECUÇÃO: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}