// Headers/ConfigManager.hpp
#ifndef CONFIGMANAGER_HPP
#define CONFIGMANAGER_HPP

#include "GameConfig.hpp"
#include <memory>

class ConfigManager {
private:
    GameConfig config;
    bool loaded = false;
    std::string currentFilepath;

    // Private constructor (Singleton)
    ConfigManager() = default;

    // Delete copy/move
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

public:
    // Singleton instance
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    // Load config from file
    bool loadConfig(const std::string& filepath = "config.json") {
        try {
            std::ifstream file(filepath);

            if (!file.is_open()) {
                std::cerr << "❌ ERRO: Não foi possível abrir '" << filepath << "'" << std::endl;
                std::cerr << "   Usando valores padrão..." << std::endl;
                config = GameConfig();  // Usa defaults das structs
                loaded = true;
                return false;
            }

            json j = json::parse(file);
            config = j.get<GameConfig>();
            currentFilepath = filepath;
            loaded = true;

            std::cout << "✅ Config carregado: " << filepath << std::endl;
            printLoadedConfig();

            return true;

        }
        catch (const json::parse_error& e) {
            std::cerr << "❌ ERRO ao parsear JSON: " << e.what() << std::endl;
            std::cerr << "   Linha: " << e.byte << std::endl;
            std::cerr << "   Usando valores padrão..." << std::endl;
            config = GameConfig();
            loaded = true;
            return false;

        }
        catch (const json::exception& e) {
            std::cerr << "❌ ERRO JSON: " << e.what() << std::endl;
            std::cerr << "   Usando valores padrão..." << std::endl;
            config = GameConfig();
            loaded = true;
            return false;

        }
        catch (const std::exception& e) {
            std::cerr << "❌ ERRO: " << e.what() << std::endl;
            config = GameConfig();
            loaded = true;
            return false;
        }
    }

    // Get config (const reference)
    const GameConfig& getConfig() const {
        if (!loaded) {
            throw std::runtime_error("Config não foi carregado! Chama loadConfig() primeiro.");
        }
        return config;
    }

    // Reload from same file
    bool reload() {
        if (currentFilepath.empty()) {
            return loadConfig();
        }
        return loadConfig(currentFilepath);
    }

    // Check if loaded
    bool isLoaded() const {
        return loaded;
    }

    // Get current filepath
    const std::string& getCurrentFilepath() const {
        return currentFilepath;
    }

    // Print loaded config (debug)
    void printLoadedConfig() const {
        if (!loaded) return;

        std::cout << "\n|====================================" << std::endl;
        std::cout << "|   CONFIGURACOES CARREGADAS         " << std::endl;
        std::cout << "|====================================" << std::endl;
        std::cout << "| Player:                            " << std::endl;
        std::cout << "|   Health: " << std::setw(4) << config.player.stats.initial_health
            << "                         " << std::endl;
        std::cout << "|   Damage: " << std::setw(4) << config.player.stats.damage
            << "                         " << std::endl;
        std::cout << "|   Speed:  " << std::setw(6) << std::setprecision(1) << std::fixed
            << config.player.stats.speed << "                     " << std::endl;
        std::cout << "|                                    " << std::endl;
        std::cout << "| Demon:                             " << std::endl;
        std::cout << "|   Health: " << std::setw(4) << config.demon.stats.initial_health
            << "                         " << std::endl;
        std::cout << "|   Damage: " << std::setw(4) << config.demon.stats.damage
            << "                         " << std::endl;
        std::cout << "|   Speed:  " << std::setw(6) << std::setprecision(1)
            << config.demon.stats.speed << "                     " << std::endl;
        std::cout << "|                                    " << std::endl;
        std::cout << "| Bishop:                            " << std::endl;
        std::cout << "|   Health: " << std::setw(4) << config.bishop.stats.initial_health
            << "                         " << std::endl;
        std::cout << "|   Damage: " << std::setw(4) << config.bishop.stats.damage
            << "                         " << std::endl;
        std::cout << "|   Heal:   " << std::setw(4) << config.bishop.heal.amount
            << "                         " << std::endl;
        std::cout << "|====================================\n" << std::endl;
        std::cout << "DEBUG: Player Attack Cooldown Carregado: "
            << config.player.attack.cooldown << std::endl;
    }
};

#endif // CONFIGMANAGER_HPP