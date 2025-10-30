// Headers/GameConfig.hpp
#ifndef GAMECONFIG_HPP
#define GAMECONFIG_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// ============================================================================
// PLAYER
// ============================================================================

// Player Animation
struct PlayerAnimationConfig {
    float frame_duration = 0.1f;
    int frames_vertical = 9;
    int frames_horizontal = 6;
};

// Player Visual Config
struct PlayerVisualConfig {
    float scale = 3.0f;
    float origin_x = 13.5f;
    float origin_y = 17.0f;
    PlayerAnimationConfig animation;
};

// Player Stats Config
struct PlayerStatsConfig {
    int initial_health = 6;
    int damage = 1;
    float speed = 350.0f;
    float hit_flash_duration = 0.1f;
};

// Player Attack Config
struct PlayerAttackConfig {
    float cooldown = 0.3f;
    float projectile_speed = 750.0f;
    float projectile_max_distance = 900.0f;
};

// Player Projectile Visual Config
struct PlayerProjectileVisualConfig {
    float scale = 1.5f;
    float origin_x = 7.5f;
    float origin_y = 12.0f;
};

// Player Spawn Config
struct PlayerSpawnConfig {
    float start_position_x = 960.0f;
    float start_position_y = 540.0f;
};

// Main Player Config
struct PlayerConfig {
    PlayerStatsConfig stats;
    PlayerAttackConfig attack;
    PlayerVisualConfig visual;
    PlayerSpawnConfig spawn;
    PlayerProjectileVisualConfig projectile_visual;
};

// ============================================================================
// DEMON
// ============================================================================

// Demon Animation Config
struct DemonAnimationConfig {
    float frame_duration = 0.12f;
    int frames = 8;
};

// Demon Stats Config
struct DemonStatsConfig {
    int initial_health = 13;
    int max_health = 30;
    int damage = 1;
    float speed = 150.0f;
};

// Demon Attack Config
struct DemonAttackConfig {
    float fire_cooldown = 2.0f;
    float attack_delay = 0.5f;
    int projectile_count = 3;
    float projectile_spread = 20.0f;
    float projectile_speed = 700.0f;
    float projectile_max_distance = 800.0f;
};

// Demon Visual Config
struct DemonVisualConfig {
    float scale = 3.0f;
    float origin_x = 14.0f;
    float origin_y = 16.0f;
    DemonAnimationConfig animation;
};

// Demon Projectile Visual Config
struct DemonProjectileVisualConfig {
    float scale = 2.0f;
};

// Demon Spawn Config
struct DemonSpawnConfig {
    float start_position_x = 960.0f;
    float start_position_y = 540.0f;
};

// Main Demon Config
struct DemonConfig {
    DemonStatsConfig stats;
    DemonAttackConfig attack;
    DemonVisualConfig visual;
    DemonSpawnConfig spawn;
    DemonProjectileVisualConfig projectile_visual;
};

// ============================================================================
// BISHOP
// ============================================================================

// Bishop Animation Config
struct BishopAnimationConfig {
    float frame_duration = 0.08f;
    int frames = 14;
    int heal_trigger_frame = 6;
};

// Bishop Stats Config
struct BishopStatsConfig {
    int initial_health = 20;
    int damage = 1;
    float speed = 310.0f;
    int spawn_chance_percent = 30;
};

// Bishop Movement Config
struct BishopMovementConfig {
    float center_pull_weight = 0.1f;
    float lateral_bias_frequency = 1.5f;
    float lateral_bias_strength = 0.5f;
};

// Bishop Heal Config
struct BishopHealConfig {
    float cooldown = 7.0f;
    int amount = 3;
};

// Bishop Visual Config
struct BishopVisualConfig {
    float scale = 3.0f;
    float origin_x = 14.0f;
    float origin_y = 16.0f;
    BishopAnimationConfig animation;
};

// Bishop Spawn Config
struct BishopSpawnConfig {
    float start_position_x = 1500.0f;
    float start_position_y = 540.0f;
};

// Main Bishop Config
struct BishopConfig {
    BishopStatsConfig stats;
    BishopMovementConfig movement;
    BishopHealConfig heal;
    BishopVisualConfig visual;
    BishopSpawnConfig spawn;
};

// ============================================================================
// GAME
// ============================================================================

// Game Bounds Config
struct GameBoundsConfig {
    float left = 213.33f;
    float top = 179.80f;
    float width = 1493.34f;
    float height = 720.40f;
};

// Dungeon Config - ATUALIZADO COM MIN/MAX ROOMS
struct DungeonConfig {
    int min_rooms = 6;      // NOVO
    int max_rooms = 16;     // NOVO
    float transition_duration = 0.49f;
    float player_spawn_offset = 60.0f;
    float door_offset = 1.0f;

    struct ExtraDoorChance {
        bool is_safe_zone = false;
        std::vector<int> chances = { 70, 50, 30, 15, 5 };
    };
    std::vector<ExtraDoorChance> extra_door_chances;
};

// UI Config
struct UIConfig {
    int max_hearts = 6;
    float heart_spacing = 67.0f;
    float heart_scale = 3.0f;
    float heart_ui_x = 200.0f;
    float heart_ui_y = 90.0f;
};

// Menu Config
struct MenuButtonConfig {
    float position_x = 150.0f;
    float position_y = 170.0f;
    float scale_x = 0.5f;
    float scale_y = 0.5f;
};

struct MenuConfig {
    MenuButtonConfig play_button;
    MenuButtonConfig exit_button;
    float background_scale_x = 1.25f;
    float background_scale_y = 1.05571847f;
};

// Door Visual Config
struct DoorVisualConfig {
    int texture_width = 48;
    int texture_height = 32;
    float origin_x = 24.0f;
    float origin_y = 31.0f;
    float scale_x = 3.0f;
    float scale_y = 3.0f;
};

// Minimap Config
struct MinimapConfig {
    float size = 150.0f;
    float room_size = 15.0f;
    float room_spacing = 18.0f;
    float connection_thickness = 2.0f;
    float offset_x = 20.0f;
    float offset_y = 20.0f;
    int background_color_alpha = 100;
    int outline_color_alpha = 200;
    float outline_thickness = 2.0f;
    int connection_color_alpha = 200;
    float connection_thickness_mini = 2.0f;
    float current_room_outline_thickness = 1.5f;
    float cleared_room_outline_thickness = 1.0f;
};

// Main Game Config
struct GameConfig_General {
    int window_width = 1920;
    int window_height = 1080;
    std::string window_title = "Isaac Test";
    GameBoundsConfig bounds;
    DungeonConfig dungeon;
    UIConfig ui;
    MenuConfig menu;
    DoorVisualConfig door_visual;
    MinimapConfig minimap;
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
    Option option_b = { 0, 156, 234, 156 };
    Option option_c = { 234, 0, 234, 156 };

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

// Player Visual
inline void from_json(const json& j, PlayerVisualConfig& c) {
    c.scale = j.value("scale", 3.0f);
    c.origin_x = j.value("origin_x", 13.5f);
    c.origin_y = j.value("origin_y", 17.0f);
    if (j.contains("animation")) c.animation = j["animation"].get<PlayerAnimationConfig>();
}

// Player Stats
inline void from_json(const json& j, PlayerStatsConfig& c) {
    c.initial_health = j.value("initial_health", 6);
    c.damage = j.value("damage", 1);
    c.speed = j.value("speed", 350.0f);
    c.hit_flash_duration = j.value("hit_flash_duration", 0.1f);
}

// Player Attack
inline void from_json(const json& j, PlayerAttackConfig& c) {
    c.cooldown = j.value("cooldown", 0.3f);
    c.projectile_speed = j.value("projectile_speed", 750.0f);
    c.projectile_max_distance = j.value("projectile_max_distance", 900.0f);
}

// Player Projectile Visual
inline void from_json(const json& j, PlayerProjectileVisualConfig& c) {
    c.scale = j.value("scale", 1.5f);
    c.origin_x = j.value("origin_x", 7.5f);
    c.origin_y = j.value("origin_y", 12.0f);
}

// Player Spawn
inline void from_json(const json& j, PlayerSpawnConfig& c) {
    c.start_position_x = j.value("start_position_x", 960.0f);
    c.start_position_y = j.value("start_position_y", 540.0f);
}

// Player
inline void from_json(const json& j, PlayerConfig& c) {
    if (j.contains("stats")) c.stats = j["stats"].get<PlayerStatsConfig>();
    if (j.contains("attack")) c.attack = j["attack"].get<PlayerAttackConfig>();
    if (j.contains("visual")) c.visual = j["visual"].get<PlayerVisualConfig>();
    if (j.contains("spawn")) c.spawn = j["spawn"].get<PlayerSpawnConfig>();
    if (j.contains("projectile_visual")) c.projectile_visual = j["projectile_visual"].get<PlayerProjectileVisualConfig>();
}

// Demon Animation
inline void from_json(const json& j, DemonAnimationConfig& c) {
    c.frame_duration = j.value("frame_duration", 0.12f);
    c.frames = j.value("frames", 8);
}

// Demon Stats
inline void from_json(const json& j, DemonStatsConfig& c) {
    c.initial_health = j.value("initial_health", 13);
    c.max_health = j.value("max_health", 30);
    c.damage = j.value("damage", 1);
    c.speed = j.value("speed", 150.0f);
}

// Demon Attack
inline void from_json(const json& j, DemonAttackConfig& c) {
    c.fire_cooldown = j.value("fire_cooldown", 2.0f);
    c.attack_delay = j.value("attack_delay", 0.5f);
    c.projectile_count = j.value("projectile_count", 3);
    c.projectile_spread = j.value("projectile_spread", 20.0f);
    c.projectile_speed = j.value("projectile_speed", 700.0f);
    c.projectile_max_distance = j.value("projectile_max_distance", 800.0f);
}

// Demon Visual
inline void from_json(const json& j, DemonVisualConfig& c) {
    c.scale = j.value("scale", 3.0f);
    c.origin_x = j.value("origin_x", 14.0f);
    c.origin_y = j.value("origin_y", 16.0f);
    if (j.contains("animation")) c.animation = j["animation"].get<DemonAnimationConfig>();
}

// Demon Projectile Visual
inline void from_json(const json& j, DemonProjectileVisualConfig& c) {
    c.scale = j.value("scale", 2.0f);
}

// Demon Spawn
inline void from_json(const json& j, DemonSpawnConfig& c) {
    c.start_position_x = j.value("start_position_x", 960.0f);
    c.start_position_y = j.value("start_position_y", 540.0f);
}

// Demon
inline void from_json(const json& j, DemonConfig& c) {
    if (j.contains("stats")) c.stats = j["stats"].get<DemonStatsConfig>();
    if (j.contains("attack")) c.attack = j["attack"].get<DemonAttackConfig>();
    if (j.contains("visual")) c.visual = j["visual"].get<DemonVisualConfig>();
    if (j.contains("spawn")) c.spawn = j["spawn"].get<DemonSpawnConfig>();
    if (j.contains("projectile_visual")) c.projectile_visual = j["projectile_visual"].get<DemonProjectileVisualConfig>();
}

// Bishop Animation
inline void from_json(const json& j, BishopAnimationConfig& c) {
    c.frame_duration = j.value("frame_duration", 0.08f);
    c.frames = j.value("frames", 14);
    c.heal_trigger_frame = j.value("heal_trigger_frame", 6);
}

// Bishop Stats
inline void from_json(const json& j, BishopStatsConfig& c) {
    c.initial_health = j.value("initial_health", 20);
    c.damage = j.value("damage", 1);
    c.speed = j.value("speed", 310.0f);
    c.spawn_chance_percent = j.value("spawn_chance_percent", 30);
}

// Bishop Movement
inline void from_json(const json& j, BishopMovementConfig& c) {
    c.center_pull_weight = j.value("center_pull_weight", 0.1f);
    c.lateral_bias_frequency = j.value("lateral_bias_frequency", 1.5f);
    c.lateral_bias_strength = j.value("lateral_bias_strength", 0.5f);
}

// Bishop Heal
inline void from_json(const json& j, BishopHealConfig& c) {
    c.cooldown = j.value("cooldown", 7.0f);
    c.amount = j.value("amount", 3);
}

// Bishop Visual
inline void from_json(const json& j, BishopVisualConfig& c) {
    c.scale = j.value("scale", 3.0f);
    c.origin_x = j.value("origin_x", 14.0f);
    c.origin_y = j.value("origin_y", 16.0f);
    if (j.contains("animation")) c.animation = j["animation"].get<BishopAnimationConfig>();
}

// Bishop Spawn
inline void from_json(const json& j, BishopSpawnConfig& c) {
    c.start_position_x = j.value("start_position_x", 1500.0f);
    c.start_position_y = j.value("start_position_y", 540.0f);
}

// Bishop
inline void from_json(const json& j, BishopConfig& c) {
    if (j.contains("stats")) c.stats = j["stats"].get<BishopStatsConfig>();
    if (j.contains("movement")) c.movement = j["movement"].get<BishopMovementConfig>();
    if (j.contains("heal")) c.heal = j["heal"].get<BishopHealConfig>();
    if (j.contains("visual")) c.visual = j["visual"].get<BishopVisualConfig>();
    if (j.contains("spawn")) c.spawn = j["spawn"].get<BishopSpawnConfig>();
}

// Game Bounds
inline void from_json(const json& j, GameBoundsConfig& c) {
    c.left = j.value("left", 213.33f);
    c.top = j.value("top", 179.80f);
    c.width = j.value("width", 1493.34f);
    c.height = j.value("height", 720.40f);
}

// Dungeon Extra Door Chance
inline void from_json(const json& j, DungeonConfig::ExtraDoorChance& c) {
    c.is_safe_zone = j.value("is_safe_zone", false);
    if (j.contains("chances") && j["chances"].is_array()) {
        c.chances = j["chances"].get<std::vector<int>>();
    }
}

// Dungeon - ATUALIZADO COM MIN/MAX ROOMS
inline void from_json(const json& j, DungeonConfig& c) {
    c.min_rooms = j.value("min_rooms", 6);
    c.max_rooms = j.value("max_rooms", 16);
    c.transition_duration = j.value("transition_duration", 0.49f);
    c.player_spawn_offset = j.value("player_spawn_offset", 60.0f);
    c.door_offset = j.value("door_offset", 1.0f);
    if (j.contains("extra_door_chances") && j["extra_door_chances"].is_array()) {
        c.extra_door_chances = j["extra_door_chances"].get<std::vector<DungeonConfig::ExtraDoorChance>>();
    }
}

// UI
inline void from_json(const json& j, UIConfig& c) {
    c.max_hearts = j.value("max_hearts", 6);
    c.heart_spacing = j.value("heart_spacing", 67.0f);
    c.heart_scale = j.value("heart_scale", 3.0f);
    c.heart_ui_x = j.value("heart_ui_x", 200.0f);
    c.heart_ui_y = j.value("heart_ui_y", 90.0f);
}

// Menu Button
inline void from_json(const json& j, MenuButtonConfig& c) {
    c.position_x = j.value("position_x", 150.0f);
    c.position_y = j.value("position_y", 170.0f);
    c.scale_x = j.value("scale_x", 0.5f);
    c.scale_y = j.value("scale_y", 0.5f);
}

// Menu
inline void from_json(const json& j, MenuConfig& c) {
    if (j.contains("play_button")) c.play_button = j["play_button"].get<MenuButtonConfig>();
    if (j.contains("exit_button")) c.exit_button = j["exit_button"].get<MenuButtonConfig>();
    c.background_scale_x = j.value("background_scale_x", 1.25f);
    c.background_scale_y = j.value("background_scale_y", 1.05571847f);
}

// Door Visual
inline void from_json(const json& j, DoorVisualConfig& c) {
    c.texture_width = j.value("texture_width", 48);
    c.texture_height = j.value("texture_height", 32);
    c.origin_x = j.value("origin_x", 24.0f);
    c.origin_y = j.value("origin_y", 31.0f);
    c.scale_x = j.value("scale_x", 3.0f);
    c.scale_y = j.value("scale_y", 3.0f);
}

// Minimap
inline void from_json(const json& j, MinimapConfig& c) {
    c.size = j.value("size", 150.0f);
    c.room_size = j.value("room_size", 15.0f);
    c.room_spacing = j.value("room_spacing", 18.0f);
    c.connection_thickness = j.value("connection_thickness", 2.0f);
    c.offset_x = j.value("offset_x", 20.0f);
    c.offset_y = j.value("offset_y", 20.0f);
    c.background_color_alpha = j.value("background_color_alpha", 100);
    c.outline_color_alpha = j.value("outline_color_alpha", 200);
    c.outline_thickness = j.value("outline_thickness", 2.0f);
    c.connection_color_alpha = j.value("connection_color_alpha", 200);
    c.connection_thickness_mini = j.value("connection_thickness_mini", 2.0f);
    c.current_room_outline_thickness = j.value("current_room_outline_thickness", 1.5f);
    c.cleared_room_outline_thickness = j.value("cleared_room_outline_thickness", 1.0f);
}

// Game
inline void from_json(const json& j, GameConfig_General& c) {
    c.window_width = j.value("window_width", 1920);
    c.window_height = j.value("window_height", 1080);
    c.window_title = j.value("window_title", std::string("Isaac Test"));

    if (j.contains("bounds")) c.bounds = j["bounds"].get<GameBoundsConfig>();
    if (j.contains("dungeon")) c.dungeon = j["dungeon"].get<DungeonConfig>();
    if (j.contains("ui")) c.ui = j["ui"].get<UIConfig>();
    if (j.contains("menu")) c.menu = j["menu"].get<MenuConfig>();
    if (j.contains("door_visual")) c.door_visual = j["door_visual"].get<DoorVisualConfig>();
    if (j.contains("minimap")) c.minimap = j["minimap"].get<MinimapConfig>();
}

// Corner Textures Option
inline void from_json(const json& j, CornerTextureConfig::Option& c) {
    c.x = j.value("x", 0);
    c.y = j.value("y", 0);
    c.width = j.value("width", 234);
    c.height = j.value("height", 156);
}

// Corner Textures
inline void from_json(const json& j, CornerTextureConfig& c) {
    if (j.contains("option_a")) c.option_a = j["option_a"].get<CornerTextureConfig::Option>();
    if (j.contains("option_b")) c.option_b = j["option_b"].get<CornerTextureConfig::Option>();
    if (j.contains("option_c")) c.option_c = j["option_c"].get<CornerTextureConfig::Option>();

    c.scale_x = j.value("scale_x", 960.0f / 234.0f);
    c.scale_y = j.value("scale_y", 540.0f / 156.0f);
}

// Projectile Textures Rect
inline void from_json(const json& j, ProjectileTexturesConfig::Rect& c) {
    c.x = j.value("x", 0);
    c.y = j.value("y", 0);
    c.width = j.value("width", 16);
    c.height = j.value("height", 16);
}

// Projectile Textures
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
    if (j.contains("corners")) c.corners = j["corners"].get<CornerTextureConfig>();
    if (j.contains("projectile_textures")) {
        c.projectile_textures = j["projectile_textures"].get<ProjectileTexturesConfig>();
    }
}

#endif // GAMECONFIG_HPP