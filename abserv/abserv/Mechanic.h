#pragma once

#include <stdint.h>

// Game mechanic constants

namespace Game {

namespace {
template<typename T>
constexpr T GetPercent(T max, T percent)
{
    return (max / static_cast<T>(100)) * percent;
}
}

// Max level a player can reach
static constexpr uint32_t LEVEL_CAP = 20;
static constexpr uint32_t SKILLPOINT_ADVANCE_XP = 15000;

// Base move speed
static constexpr float BASE_SPEED = 150.0f;
static constexpr float BASE_TURN_SPEED = 2000.0f;

// Attack speeds in ms
static constexpr uint32_t ATTACK_SPEED_AXE        = 1330;
static constexpr uint32_t ATTACK_SPEED_SWORD      = 1330;
static constexpr uint32_t ATTACK_SPEED_HAMMER     = 1750;
static constexpr uint32_t ATTACK_SPEED_FLATBOW    = 2000;
static constexpr uint32_t ATTACK_SPEED_HORNBOW    = 2700;
static constexpr uint32_t ATTACK_SPEED_SHORTBOW   = 2000;
static constexpr uint32_t ATTACK_SPEED_LONGBOW    = 2400;
static constexpr uint32_t ATTACK_SPEED_RECURVEBOW = 2400;
static constexpr uint32_t ATTACK_SPEED_STAFF      = 1750;
static constexpr uint32_t ATTACK_SPEED_WAND       = 1750;
static constexpr uint32_t ATTACK_SPEED_DAGGERS    = 1330;
static constexpr uint32_t ATTACK_SPEED_SCYTE      = 1500;
static constexpr uint32_t ATTACK_SPEED_SPEAR      = 1500;

static constexpr float MAX_IAS = 1.33f;                    // Increased Attack Speed
static constexpr float MAX_DAS = 0.5f;                     // Decreased Attack Speed

// Ranges
static constexpr float RANGE_BASE         = 80.0f;
static constexpr float RANGE_AGGRO        = GetPercent(RANGE_BASE, 24.0f);
static constexpr float RANGE_COMPASS      = GetPercent(RANGE_BASE, 95.0f);
static constexpr float RANGE_HALF_COMPASS = RANGE_COMPASS / 2.0f;
static constexpr float RANGE_SPIRIT       = RANGE_AGGRO * 1.6f;                   // Longbow, spirits
static constexpr float RANGE_EARSHOT      = RANGE_AGGRO;
static constexpr float RANGE_CASTING      = RANGE_AGGRO * 1.35f;
static constexpr float RANGE_PROJECTILE   = RANGE_AGGRO;

static constexpr float RANGE_LONGBOW      = RANGE_AGGRO * 1.6f;
static constexpr float RANGE_FLATBOW      = RANGE_AGGRO * 1.6f;
static constexpr float RANGE_HORNBOW      = RANGE_AGGRO * 1.35f;
static constexpr float RANGE_RECURVEBOW   = RANGE_AGGRO * 1.35f;
static constexpr float RANGE_SHORTBOW     = RANGE_AGGRO * 1.05f;
static constexpr float RANGE_SPEAR        = RANGE_AGGRO * 1.05f;

// Close range
static constexpr float RANGE_TOUCH        = 1.5f;
static constexpr float RANGE_ADJECENT     = GetPercent(RANGE_BASE, 3.0f);
static constexpr float RANGE_VISIBLE      = RANGE_AGGRO;

enum class Ranges : uint8_t
{
    Aggro = 0,
    Compass,
    Spirit,
    Earshot,
    Casting,
    Projectile,
    HalfCompass,
    Touch,
    Adjecent,
    Visible,
    Map                        // Whole map, must be last
};

static constexpr float RangeDistances[] = {
    RANGE_AGGRO,
    RANGE_COMPASS,
    RANGE_SPIRIT,
    RANGE_EARSHOT,
    RANGE_CASTING,
    RANGE_PROJECTILE,
    RANGE_HALF_COMPASS,
    RANGE_TOUCH,
    RANGE_ADJECENT,
    RANGE_VISIBLE,
    std::numeric_limits<float>::max()
};

// https://wiki.guildwars.com/wiki/Armor_rating
static constexpr float DamagePosChances[] = {
    0.375f,                                // Chest
    0.25f,                                 // Legs
    0.125f,                                // Head
    0.125f,                                // Hands
    0.125f,                                // Feet
};

// If the rotation of 2 actors is smaller than this, an attack is from behind
static constexpr float BEHIND_ANGLE = 0.52f;
static constexpr uint32_t DEFAULT_KNOCKDOWN_TIME = 2000;

}
