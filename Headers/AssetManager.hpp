// Headers/AssetManager.hpp

#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include "SFML/Graphics.hpp"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

class AssetManager {
private:
    // Mapas para guardar todas as texturas e vetores de anima��o
    std::map<std::string, sf::Texture> textures;
    static std::map<std::string, std::vector<sf::Texture>> animationSets;

    // Construtor privado para garantir o Singleton
    AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

public:
    // M�todo para obter a �nica inst�ncia (Singleton)
    static AssetManager& getInstance() {
        static AssetManager instance;
        return instance;
    }

    // Carrega uma �nica textura
    bool loadTexture(const std::string& name, const std::string& filename);

    // Carrega uma anima��o completa
    void loadAnimation(
        const std::string& setPrefix,
        const std::string& folderName,
        const std::string& prefix,
        int totalFrames,
        const std::string& suffix);

    // Obter uma �nica textura por nome
    sf::Texture& getTexture(const std::string& name);

    // Obter um vetor de texturas de uma anima��o carregada
    std::vector<sf::Texture>& getAnimationSet(const std::string& setPrefix);
};

#endif // ASSETMANAGER_HPP