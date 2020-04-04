/**
 * Copyright 2017-2020 Stefan Ascher
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

#include "stdafx.h"
#include "ItemFactory.h"
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include <AB/Entities/ItemChanceList.h>
#include <AB/Entities/TypedItemList.h>
#include "Player.h"
#include "ItemsCache.h"

namespace Game {

ItemFactory::ItemFactory() = default;

void ItemFactory::Initialize()
{
    // Upgrades are not dependent on the map
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    {
        AB::Entities::TypedItemsInsignia insignias;
        insignias.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(insignias))
        {
            std::vector<TypedListValue> values;
            values.reserve(insignias.items.size());
            for (const auto& i : insignias.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierInsignia, values);
        }
    }

    {
        AB::Entities::TypedItemsRunes runes;
        runes.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(runes))
        {
            std::vector<TypedListValue> values;
            values.reserve(runes.items.size());
            for (const auto& i : runes.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierRune, values);
        }
    }

    {
        AB::Entities::TypedItemsWeaponPrefix prefixes;
        prefixes.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(prefixes))
        {
            std::vector<TypedListValue> values;
            values.reserve(prefixes.items.size());
            for (const auto& i : prefixes.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierWeaponPrefix, values);
        }
    }

    {
        AB::Entities::TypedItemsWeaponSuffix suffixes;
        suffixes.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(suffixes))
        {
            std::vector<TypedListValue> values;
            values.reserve(suffixes.items.size());
            for (const auto& i : suffixes.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierWeaponSuffix, values);
        }
    }

    {
        AB::Entities::TypedItemsWeaponInscription inscriptions;
        inscriptions.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(inscriptions))
        {
            std::vector<TypedListValue> values;
            values.reserve(inscriptions.items.size());
            for (const auto& i : inscriptions.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierWeaponInscription, values);
        }
    }
}

void ItemFactory::CalculateValue(const AB::Entities::Item& item, uint32_t level, AB::Entities::ConcreteItem& result)
{
    assert(level <= LEVEL_CAP);
    const uint32_t l = (LEVEL_CAP + 1) - level;

    auto* rng = GetSubsystem<Crypto::Random>();
    if (item.type != AB::Entities::ItemTypeMoney && item.type != AB::Entities::ItemTypeMaterial)
    {
        result.count = 1;
        if (item.value == 0)
            result.value = Math::Clamp(static_cast<uint16_t>(rng->Get<uint16_t>(MIN_ITEM_VALUE, MAX_ITEM_VALUE) / l),
                static_cast<uint16_t>(5), static_cast<uint16_t>(1000));
        else
            result.value = item.value;
    }
    else
    {
        // Money or Material
        if (item.type == AB::Entities::ItemTypeMoney)
            // Money
            result.value = 1;
        else
            // The value in the items table is used for materials
            result.value = (item.value != 0) ? item.value : 1u;
        // Count also depends on the value of the item
        if (item.stackAble)
            // Only stackable items drop count > 1. And always <= MAX_INVENTORY_STACK_SIZE
            result.count = Math::Clamp((rng->Get(MIN_ITEM_VALUE, MAX_ITEM_VALUE) / l) / result.value,
                1u, MAX_INVENTORY_STACK_SIZE);
        else
            result.count = 1;
    }
}

bool ItemFactory::CreateDBItem(AB::Entities::ConcreteItem item)
{
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Create(item))
    {
        LOG_ERROR << "Unable to create concrete item" << std::endl;
        return false;
    }
    return true;
}

std::unique_ptr<Item> ItemFactory::CreateTempItem(const std::string& itemUuid)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item gameItem;
    gameItem.uuid = itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Unable to read item with UUID " << itemUuid << std::endl;
        return std::unique_ptr<Item>();
    }

    std::unique_ptr<Item> result = std::make_unique<Item>(gameItem);
    if (!result->LoadScript(result->data_.script))
        return std::unique_ptr<Item>();
    return result;
}

uint32_t ItemFactory::CreateItem(const std::string& itemUuid,
    const std::string& instanceUuid, const std::string& mapUuid,
    uint32_t level /* = LEVEL_CAP */,
    bool maxStats /* = false */,
    const std::string& accUuid /* = Utils::Uuid::EMPTY_UUID */,
    const std::string& playerUuid /* = Utils::Uuid::EMPTY_UUID */)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item gameItem;
    gameItem.uuid = itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Unable to read item with UUID " << itemUuid << std::endl;
        return 0;
    }

    std::unique_ptr<Item> result = std::make_unique<Item>(gameItem);
    if (!result->LoadScript(result->data_.script))
        return 0;

    AB::Entities::ConcreteItem ci;
    ci.uuid = Utils::Uuid::New();
    ci.itemUuid = gameItem.uuid;
    ci.accountUuid = accUuid;
    ci.playerUuid = playerUuid;
    ci.instanceUuid = instanceUuid;
    ci.mapUuid = mapUuid;
    ci.creation = Utils::Tick();
    CalculateValue(gameItem, level, ci);

    // Create item stats for this drop
    if (!result->GenerateConcrete(ci, level, maxStats))
        return 0;

    // Save the created stats
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&ItemFactory::CreateDBItem, this, result->concreteItem_))
    );
    auto* cache = GetSubsystem<ItemsCache>();
    return cache->Add(std::move(result));
}

uint32_t ItemFactory::CreatePlayerItem(Player& forPlayer, const std::string& itemUuid, uint32_t count /* = 1 */)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item gameItem;
    gameItem.uuid = itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Unable to read item with UUID " << itemUuid << std::endl;
        return 0;
    }
    std::unique_ptr<Item> result = std::make_unique<Item>(gameItem);
    if (!result->LoadScript(result->data_.script))
        return 0;

    AB::Entities::ConcreteItem ci;
    ci.uuid = Utils::Uuid::New();
    ci.itemUuid = gameItem.uuid;
    ci.accountUuid = forPlayer.account_.uuid;
    ci.playerUuid = forPlayer.data_.uuid;
    ci.instanceUuid = forPlayer.GetGame()->instanceData_.uuid;
    ci.mapUuid = forPlayer.GetGame()->data_.uuid;
    ci.creation = Utils::Tick();

    // Create item stats for this drop
    if (!result->GenerateConcrete(ci, forPlayer.data_.level, true))
        return 0;
    ci.count = count;

    // Save the created stats
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&ItemFactory::CreateDBItem, this, result->concreteItem_))
    );
    auto* cache = GetSubsystem<ItemsCache>();
    return cache->Add(std::move(result));
}

uint32_t ItemFactory::CreatePlayerMoneyItem(Player& forPlayer, uint32_t count)
{
    return CreatePlayerItem(forPlayer, MONEY_ITEM_UUID, count);
}

std::unique_ptr<Item> ItemFactory::LoadConcrete(const std::string& concreteUuid)
{
    AB::Entities::ConcreteItem ci;
    auto* client = GetSubsystem<IO::DataClient>();
    ci.uuid = concreteUuid;
    if (!client->Read(ci))
    {
        LOG_ERROR << "Error loading concrete item " << concreteUuid << std::endl;
        return std::unique_ptr<Item>();
    }
    if (ci.deleted != 0)
    {
        LOG_INFO << "Item " << concreteUuid << " was deleted" << std::endl;
        return std::unique_ptr<Item>();
    }

    AB::Entities::Item gameItem;
    gameItem.uuid = ci.itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Error loading item " << ci.itemUuid << std::endl;
        return std::unique_ptr<Item>();
    }
    std::unique_ptr<Item> result = std::make_unique<Item>(gameItem);
    if (!result->LoadConcrete(ci))
        return std::unique_ptr<Item>();
    if (!result->LoadScript(result->data_.script))
        return std::unique_ptr<Item>();

    return result;
}

uint32_t ItemFactory::GetConcreteId(const std::string& concreteUuid)
{
    auto* cache = GetSubsystem<ItemsCache>();
    uint32_t result = cache->GetConcreteId(concreteUuid);
    if (result != 0)
        return result;
    std::unique_ptr<Item> item = LoadConcrete(concreteUuid);
    if (!item)
        return 0;
    return cache->Add(std::move(item));
}

void ItemFactory::IdentifyArmor(Item* item, Player* player)
{
    uint32_t insignia = CreateModifier(AB::Entities::ItemTypeModifierInsignia, item,
        player->GetLevel(), false, player->data_.uuid);
    if (insignia != 0)
        item->SetUpgrade(ItemUpgrade::Pefix, insignia);
    uint32_t rune = CreateModifier(AB::Entities::ItemTypeModifierRune, item,
        player->GetLevel(), false, player->data_.uuid);
    if (rune != 0)
        item->SetUpgrade(ItemUpgrade::Suffix, rune);
}

void ItemFactory::IdentifyWeapon(Item* item, Player* player)
{
    uint32_t prefix = CreateModifier(AB::Entities::ItemTypeModifierWeaponPrefix, item,
        player->GetLevel(), false, player->data_.uuid);
    if (prefix != 0)
        item->SetUpgrade(ItemUpgrade::Pefix, prefix);
    uint32_t suffix = CreateModifier(AB::Entities::ItemTypeModifierWeaponSuffix, item,
        player->GetLevel(), false, player->data_.uuid);
    if (suffix != 0)
        item->SetUpgrade(ItemUpgrade::Suffix, suffix);
    uint32_t inscr = CreateModifier(AB::Entities::ItemTypeModifierWeaponInscription, item,
        player->GetLevel(), false, player->data_.uuid);
    if (inscr != 0)
        item->SetUpgrade(ItemUpgrade::Inscription, inscr);
}

void ItemFactory::IdentifyOffHandWeapon(Item* item, Player* player)
{
    // Offhead weapons do not have a prefix
    uint32_t suffix = CreateModifier(AB::Entities::ItemTypeModifierWeaponSuffix, item,
        player->GetLevel(), false, player->data_.uuid);
    if (suffix != 0)
        item->SetUpgrade(ItemUpgrade::Suffix, suffix);
    uint32_t inscr = CreateModifier(AB::Entities::ItemTypeModifierWeaponInscription, item,
        player->GetLevel(), false, player->data_.uuid);
    if (inscr != 0)
        item->SetUpgrade(ItemUpgrade::Inscription, inscr);
}

uint32_t ItemFactory::CreateModifier(AB::Entities::ItemType modType, Item* forItem,
    uint32_t level, bool maxStats, const std::string& playerUuid)
{
    auto it = typedItems_.find(modType);
    if (it == typedItems_.end())
        return 0;
    std::vector<std::vector<TypedListValue>::iterator> result;
    Utils::SelectIterators((*it).second.begin(), (*it).second.end(),
        std::back_inserter(result),
        [forItem](const TypedListValue& current)
    {
        if (!forItem->IsArmor())
            return (current.second == forItem->data_.type);
        // Armor can have only runes and insignias
        return current.second == AB::Entities::ItemTypeModifierRune ||
            current.second == AB::Entities::ItemTypeModifierInsignia;
    });

    if (result.size() == 0)
        return 0;
    auto selIt = Utils::SelectRandomly(result.begin(), result.end());
    if (selIt == result.end())
        return 0;

    return CreateItem((*(*selIt)).first,
        forItem->concreteItem_.instanceUuid, forItem->concreteItem_.mapUuid,
        level, maxStats, Utils::Uuid::EMPTY_UUID, playerUuid);
}

void ItemFactory::IdentiyItem(Item* item, Player* player)
{
    assert(item);
    switch (item->data_.type)
    {
    case AB::Entities::ItemTypeArmorHead:
    case AB::Entities::ItemTypeArmorChest:
    case AB::Entities::ItemTypeArmorHands:
    case AB::Entities::ItemTypeArmorLegs:
    case AB::Entities::ItemTypeArmorFeet:
        // Armor
        IdentifyArmor(item, player);
        break;
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeHammer:
    case AB::Entities::ItemTypeWand:
    case AB::Entities::ItemTypeSpear:
        // Lead hand weapon
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
    case AB::Entities::ItemTypeDaggers:
    case AB::Entities::ItemTypeScyte:
    case AB::Entities::ItemTypeStaff:
        // Two handed
        IdentifyWeapon(item, player);
        break;
    case AB::Entities::ItemTypeFocus:
    case AB::Entities::ItemTypeShield:
        // Off hand
        IdentifyOffHandWeapon(item, player);
        break;
    default:
        return;
    }
}

void ItemFactory::DeleteConcrete(const std::string& uuid)
{
    AB::Entities::ConcreteItem ci;
    auto* client = GetSubsystem<IO::DataClient>();
    ci.uuid = uuid;
    if (!client->Read(ci))
    {
        LOG_WARNING << "Unable to  read concrete item " << uuid << std::endl;
        return;
    }
    ci.deleted = Utils::Tick();
    if (client->Update(ci))
    {
        auto* cache = GetSubsystem<ItemsCache>();
        cache->RemoveConcrete(uuid);
        client->Invalidate(ci);
    }
    else
        LOG_WARNING << "Error deleting concrete item " << uuid << std::endl;
}

void ItemFactory::DeleteItem(Item* item)
{
    if (!item)
        return;
    {
        auto upg = item->GetUpgrade(ItemUpgrade::Pefix);
        if (upg)
        {
            DeleteConcrete(upg->concreteItem_.uuid);
            item->RemoveUpgrade(ItemUpgrade::Pefix);
        }
    }
    {
        auto upg = item->GetUpgrade(ItemUpgrade::Suffix);
        if (upg)
        {
            DeleteConcrete(upg->concreteItem_.uuid);
            item->RemoveUpgrade(ItemUpgrade::Suffix);
        }
    }
    {
        auto upg = item->GetUpgrade(ItemUpgrade::Inscription);
        if (upg)
        {
            DeleteConcrete(upg->concreteItem_.uuid);
            item->RemoveUpgrade(ItemUpgrade::Inscription);
        }
    }
    DeleteConcrete(item->concreteItem_.uuid);
}

void ItemFactory::LoadDropChances(const std::string mapUuid)
{
    auto it = dropChances_.find(mapUuid);
    if (it != dropChances_.end())
        // Already loaded
        return;

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::ItemChanceList il;
    il.uuid = mapUuid;
    if (!client->Read(il))
    {
        LOG_ERROR << "Failed to read Item list for map " << mapUuid << std::endl;
        return;
    }

    std::unique_ptr<ItemSelector> selector = std::make_unique<ItemSelector>();
    for (const auto& i : il.items)
    {
        selector->Add(i.first, i.second);
    }
    selector->Update();

    {
        std::scoped_lock lock(lock_);
        dropChances_.emplace(mapUuid, std::move(selector));
    }
}

void ItemFactory::DeleteMap(const std::string& uuid)
{
    auto it = dropChances_.find(uuid);
    if (it != dropChances_.end())
        dropChances_.erase(it);
}

uint32_t ItemFactory::CreateDropItem(const std::string& instanceUuid, const std::string& mapUuid,
    uint32_t level, Player* target)
{
    AB_PROFILE;
    auto it = dropChances_.find(mapUuid);
    if (it == dropChances_.end())
        // Maybe outpost -> no drops
        return 0;
    if (!(*it).second || (*it).second->Count() == 0)
        // No drops on this map :(
        return 0;

    auto rng = GetSubsystem<Crypto::Random>();
    const float rnd1 = rng->GetFloat();
    const float rnd2 = rng->GetFloat();
    const std::string& itemUuid = (*it).second->Get(rnd1, rnd2);
    if (uuids::uuid(itemUuid).nil())
        // There is a chance that nothing drops
        return 0;

    return CreateItem(itemUuid, instanceUuid, mapUuid, level, false, target->account_.uuid, target->data_.uuid);
}

}
