// Headers/GameConfig.hpp
#ifndef GAMECONFIG_HPP
#define GAMECONFIG_HPP

#include <string>
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// ============================================================================
// PLAYER
// ============================================================================
struct PlayerAnimationConfig {
    float frame_duration = 0.1f;
    int frames_vertical = 9;
    int frames_horizontal = 6;
};

struct PlayerConfig {
    // Stats
    int initial_health = 6;
    float speed = 350.0f;

    // Visual
    float scale = 3.0f;
    float origin_x = 13.5f;
    float origin_y = 17.0f;

    // Position
    float start_position_x = 960.0f;
    float start_position_y = 540.0f;

    // Combat
    float attack_cooldown = 0.3f;
    float projectile_speed = 750.0f;
    float projectile_max_distance = 900.0f;
    float projectile_scale = 1.5f;
    float projectile_origin_x = 7.5f;
    float projectile_origin_y = 12.0f;

    // Effects
    float hit_flash_duration = 0.1f;

    // Animation
    PlayerAnimationConfig animation;
};

// ============================================================================
// DEMON
// ============================================================================
struct DemonConfig {
    // Stats
    int initial_health = 13;
    int max_health = 30;
    float speed = 150.0f;

    // Visual
    float scale = 3.0f;
    float origin_x = 14.0f;
    float origin_y = 16.0f;

    // Position
    float start_position_x = 960.0f;
    float start_position_y = 540.0f;

    // Combat
    float fire_cooldown = 2.0f;
    float attack_delay = 0.5f;
    int projectile_count = 3;
    float projectile_spread = 20.0f;
    float projectile_speed = 700.0f;
    float projectile_scale = 2.0f;
    float projectile_max_distance = 800.0f;

    // Animation
    float frame_duration = 0.12f;
    int animation_frames = 8;
};

// ============================================================================
// BISHOP
// ============================================================================
struct BishopConfig {
    // Stats
    int initial_health = 20;
    float speed = 310.0f;

    // Visual
    float scale = 3.0f;
    float origin_x = 14.0f;
    float origin_y = 16.0f;

    // Position
    float start_position_x = 1500.0f;
    float start_position_y = 540.0f;

    // Healing
    float heal_cooldown = 7.0f;
    int heal_amount = 3;

    // Movement
    float center_pull_weight = 0.1f;
    float lateral_bias_frequency = 1.5f;
    float lateral_bias_strength = 0.5f;

    // Animation
    float frame_duration = 0.08f;
    int animation_frames = 14;
    int heal_trigger_frame = 6;  // FRAME_B7_INDEX
};

// ============================================================================
// GAME BOUNDS
// ============================================================================
struct GameBoundsConfig {
    float left = 213.33f;
    float top = 179.80f;
    float width = 1493.34f;
    float height = 720.40f;
};

// ============================================================================
// GAME
// ============================================================================
struct GameConfig_General {
    int window_width = 1920;
    int window_height = 1080;
    std::string window_title = "Isaac Test";
    GameBoundsConfig bounds;
};

// ============================================================================
// UI
// ============================================================================
struct UIConfig {
    // Hearts
    int max_hearts = 3;
    float heart_spacing = 67.0f;
    float heart_scale = 3.0f;
    float heart_ui_x = 200.0f;
    float heart_ui_y = 90.0f;
};

// ============================================================================
// CORNER TEXTURES
// ============================================================================
struct CornerTextureConfig {
    struct Option {
        int x = 0;
        int y = 0;
        int width = 234;
        int height = 156;
    };

    Option option_a = { 0, 0, 234, 156 };
    Option option_b = { 0, 156, 234, 155 };
    Option option_c = { 234, 0, 234, 155 };

    float scale_x = 960.0f / 234.0f;
    float scale_y = 540.0f / 156.0f;
};

// ============================================================================
// PROJECTILE TEXTURES
// ============================================================================
struct ProjectileTexturesConfig {
    struct Rect {
        int x = 0;
        int y = 0;
        int width = 16;
        int height = 16;
    };

    Rect isaac_tear = { 8, 39, 16, 16 };
    Rect demon_tear = { 8, 103, 16, 16 };
};

// ============================================================================
// MAIN CONFIG STRUCTURE
// ============================================================================
struct GameConfig {
    PlayerConfig player;
    DemonConfig demon;
    BishopConfig bishop;
    GameConfig_General game;
    UIConfig ui;
    CornerTextureConfig corners;
    ProjectileTexturesConfig projectile_textures;
};

// ============================================================================
// JSON DESERIALIZATION (with defaults)
// ============================================================================

// Player Animation
inline void from_json(const json& j, PlayerAnimationConfig& c) {
    c.frame_duration = j.value("frame_duration", 0.1f);
    c.frames_vertical = j.value("frames_vertical", 9);
    c.frames_horizontal = j.value("frames_horizontal", 6);
}

// Player
inline void from_json(const json& j, PlayerConfig& c) {
    // Stats
    c.initial_health = j.value("initial_health", 6);
    c.speed = j.value("speed", 350.0f);

    // Visual
    c.scale = j.value("scale", 3.0f);
    c.origin_x = j.value("origin_x", 13.5f);
    c.origin_y = j.value("origin_y", 17.0f);

    // Position
    c.start_position_x = j.value("start_position_x", 960.0f);
    c.start_position_y = j.value("start_position_y", 540.0f);

    // Combat
    c.attack_cooldown = j.value("attack_cooldown", 0.3f);
    c.projectile_speed = j.value("projectile_speed", 750.0f);
    c.projectile_max_distance = j.value("projectile_max_distance", 900.0f);
    c.projectile_scale = j.value("projectile_scale", 1.5f);
    c.projectile_origin_x = j.value("projectile_origin_x", 7.5f);
    c.projectile_origin_y = j.value("projectile_origin_y", 12.0f);

    // Effects
    c.hit_flash_duration = j.value("hit_flash_duration", 0.1f);

    // Animation
    if (j.contains("animation")) {
        c.animation = j["animation"].get<PlayerAnimationConfig>();
    }
}

// Demon
inline void from_json(const json& j, DemonConfig& c) {
    // Stats
    c.initial_health = j.value("initial_health", 13);
    c.max_health = j.value("max_health", 30);
    c.speed = j.value("speed", 150.0f);

    // Visual
    c.scale = j.value("scale", 3.0f);
    c.origin_x = j.value("origin_x", 14.0f);
    c.origin_y = j.value("origin_y", 16.0f);

    // Position
    c.start_position_x = j.value("start_position_x", 960.0f);
    c.start_position_y = j.value("start_position_y", 540.0f);

    // Combat
    c.fire_cooldown = j.value("fire_cooldown", 2.0f);
    c.attack_delay = j.value("attack_delay", 0.5f);
    c.projectile_count = j.value("projectile_count", 3);
    c.projectile_spread = j.value("projectile_spread", 20.0f);
    c.projectile_speed = j.value("projectile_speed", 700.0f);
    c.projectile_scale = j.value("projectile_scale", 2.0f);
    c.projectile_max_distance = j.value("projectile_max_distance", 800.0f);

    // Animation
    c.frame_duration = j.value("frame_duration", 0.12f);
    c.animation_frames = j.value("animation_frames", 8);
}

// Bishop
inline void from_json(const json& j, BishopConfig& c) {
    // Stats
    c.initial_health = j.value("initial_health", 20);
    c.speed = j.value("speed", 310.0f);

    // Visual
    c.scale = j.value("scale", 3.0f);
    c.origin_x = j.value("origin_x", 14.0f);
    c.origin_y = j.value("origin_y", 16.0f);

    // Position
    c.start_position_x = j.value("start_position_x", 1500.0f);
    c.start_position_y = j.value("start_position_y", 540.0f);

    // Healing
    c.heal_cooldown = j.value("heal_cooldown", 7.0f);
    c.heal_amount = j.value("heal_amount", 3);

    // Movement
    c.center_pull_weight = j.value("center_pull_weight", 0.1f);
    c.lateral_bias_frequency = j.value("lateral_bias_frequency", 1.5f);
    c.lateral_bias_strength = j.value("lateral_bias_strength", 0.5f);

    // Animation
    c.frame_duration = j.value("frame_duration", 0.08f);
    c.animation_frames = j.value("animation_frames", 14);
    c.heal_trigger_frame = j.value("heal_trigger_frame", 6);
}

// Game Bounds
inline void from_json(const json& j, GameBoundsConfig& c) {
    c.left = j.value("left", 213.33f);
    c.top = j.value("top", 179.80f);
    c.width = j.value("width", 1493.34f);
    c.height = j.value("height", 720.40f);
}

// Game
inline void from_json(const json& j, GameConfig_General& c) {
    c.window_width = j.value("window_width", 1920);
    c.window_height = j.value("window_height", 1080);
    c.window_title = j.value("window_title", std::string("Isaac Test"));

    if (j.contains("bounds")) {
        c.bounds = j["bounds"].get<GameBoundsConfig>();
    }
}

// UI
inline void from_json(const json& j, UIConfig& c) {
    c.max_hearts = j.value("max_hearts", 3);
    c.heart_spacing = j.value("heart_spacing", 67.0f);
    c.heart_scale = j.value("heart_scale", 3.0f);
    c.heart_ui_x = j.value("heart_ui_x", 200.0f);
    c.heart_ui_y = j.value("heart_ui_y", 90.0f);
}

// Corner Textures
inline void from_json(const json& j, CornerTextureConfig::Option& c) {
    c.x = j.value("x", 0);
    c.y = j.value("y", 0);
    c.width = j.value("width", 234);
    c.height = j.value("height", 156);
}

inline void from_json(const json& j, CornerTextureConfig& c) {
    if (j.contains("option_a")) c.option_a = j["option_a"].get<CornerTextureConfig::Option>();
    if (j.contains("option_b")) c.option_b = j["option_b"].get<CornerTextureConfig::Option>();
    if (j.contains("option_c")) c.option_c = j["option_c"].get<CornerTextureConfig::Option>();

    c.scale_x = j.value("scale_x", 960.0f / 234.0f);
    c.scale_y = j.value("scale_y", 540.0f / 156.0f);
}

// Projectile Textures
inline void from_json(const json& j, ProjectileTexturesConfig::Rect& c) {
    c.x = j.value("x", 0);
    c.y = j.value("y", 0);
    c.width = j.value("width", 16);
    c.height = j.value("height", 16);
}

inline void from_json(const json& j, ProjectileTexturesConfig& c) {
    if (j.contains("isaac_tear")) c.isaac_tear = j["isaac_tear"].get<ProjectileTexturesConfig::Rect>();
    if (j.contains("demon_tear")) c.demon_tear = j["demon_tear"].get<ProjectileTexturesConfig::Rect>();
}

// MAIN CONFIG (IMPORTANTE!)
inline void from_json(const json& j, GameConfig& c) {
    if (j.contains("player")) c.player = j["player"].get<PlayerConfig>();
    if (j.contains("demon")) c.demon = j["demon"].get<DemonConfig>();
    if (j.contains("bishop")) c.bishop = j["bishop"].get<BishopConfig>();
    if (j.contains("game")) c.game = j["game"].get<GameConfig_General>();
    if (j.contains("ui")) c.ui = j["ui"].get<UIConfig>();
    if (j.contains("corners")) c.corners = j["corners"].get<CornerTextureConfig>();
    if (j.contains("projectile_textures")) {
        c.projectile_textures = j["projectile_textures"].get<ProjectileTexturesConfig>();
    }
}

#endif // GAMECONFIG_HPP