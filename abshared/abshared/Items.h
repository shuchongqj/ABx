/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

namespace Game {

enum class ItemStatIndex : size_t
{
    None = 0,
    MinDamage = 1,
    MaxDamage,
    DamageType,
    Attribute,           // Weapon requires this attribute
    AttributeValue,      // Weapon requires this Rank of the attribute
    Armor,               // General armor
    HealthRegen,
    EnergyRegen,
    Health,              // +/- Health
    Energy,              // +/- Energy

    PhysicalDamageReduction,
    HexDurationReduction,
    ConditionDurationReduction,
    BlindnessDurationReduction,
    WeaknessDurationReduction,
    DeseaseDurationReduction,
    PoisionDurationReduction,
    DazedDurationReduction,
    DeepWoundDurationReduction,
    BleedingDurationReduction,
    CrippledDurationReduction,

    // Type specific armor
    ArmorElemental,
    ArmorFire,
    ArmorCold,
    ArmorLightning,
    ArmorEarth,
    ArmorPhysical,
    // Special
    ArmorHoly,
    ArmorShadow,
    ArmorTypeless,
    // Other
    ArmorDark,
    ArmorChaos,

    // Crafing price, salvage outcome
    Material1Index = 500,
    Material1Count,
    Material2Index,
    Material2Count,
    Material3Index,
    Material3Count,
    Material4Index,
    Material4Count,

    // Items may increase the attribute rank by 1. Bellow a list of attributes it increases.
    AttributeOffset = 1000
};

}
