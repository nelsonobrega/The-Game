// Source/AssetManager.cpp

#include "AssetManager.hpp"
#include <utility>

// Inicialização da variável static fora da classe
std::map<std::string, std::vector<sf::Texture>> AssetManager::animationSets;


bool AssetManager::loadTexture(const std::string& name, const std::string& filename) {
    sf::Texture texture;
    if (!texture.loadFromFile(filename)) {
        std::cerr << "ERRO: Falha ao carregar textura: " << filename << std::endl;
        // Lançar exceção para debug imediato.
        throw std::runtime_error("Asset Not Found: " + filename);
    }
    textures[name] = std::move(texture);
    return true;
}

void AssetManager::loadAnimation(
    const std::string& setPrefix,
    const std::string& folderName,
    const std::string& prefix,
    int totalFrames,
    const std::string& suffix)
{
    std::string base_path = "Images/";
    std::vector<sf::Texture> newSet;
    newSet.resize(totalFrames);

    for (int i = 0; i < totalFrames; ++i) {
        std::string filename = base_path + folderName + "/" + prefix + std::to_string(i + 1) + suffix;

        if (!newSet[i].loadFromFile(filename)) {
            std::cerr << "ERRO FATAL DE ASSET: Falha ao carregar frame: " << filename << std::endl;
            // LANÇAR EXCEÇÃO PARA PARAR O PROGRAMA IMEDIATAMENTE NO DEBUG
            throw std::runtime_error("Asset Not Found: " + filename);
        }
    }

    // Move o vetor de texturas carregado para o mapa estático
    animationSets[setPrefix] = std::move(newSet);
}

sf::Texture& AssetManager::getTexture(const std::string& name) {
    if (textures.find(name) == textures.end()) {
        std::cerr << "ERRO: Textura nao encontrada: " << name << std::endl;
        throw std::runtime_error("Textura nao encontrada: " + name);
    }
    return textures.at(name);
}

std::vector<sf::Texture>& AssetManager::getAnimationSet(const std::string& setPrefix) {
    if (animationSets.find(setPrefix) == animationSets.end()) {
        std::cerr << "ERRO: Set de Animacao nao encontrado: " << setPrefix << std::endl;
        throw std::runtime_error("Set de Animacao nao encontrado: " + setPrefix);
    }
    return animationSets.at(setPrefix);
}