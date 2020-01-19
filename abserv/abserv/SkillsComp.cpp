#include "stdafx.h"
#include "SkillsComp.h"
#include "Actor.h"
#include "Skill.h"
#include "Attributes.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

#define DEBUG_AI

namespace Game {
namespace Components {

SkillsComp::SkillsComp(Actor& owner) :
    owner_(owner),
    lastError_(AB::GameProtocol::SkillErrorNone),
    lastSkillIndex_(-1),
    startDirty_(false),
    endDirty_(false),
    usingSkill_(false),
    newRecharge_(0),
    lastSkillTime_(0)
{
    owner_.SubscribeEvent<void(int)>(EVENT_ON_INCMORALE, std::bind(&SkillsComp::OnIncMorale, this, std::placeholders::_1));
}

void SkillsComp::OnIncMorale(int)
{
    owner_.skills_->VisitSkills([](int, Skill& current) {
        current.SetRecharged(0);
        return Iteration::Continue;
    });
}

void SkillsComp::Update(uint32_t timeElapsed)
{
    SkillBar* sb = owner_.GetSkillBar();
    sb->Update(timeElapsed);
    if (!usingSkill_)
        return;

    if (auto ls = lastSkill_.lock())
    {
        if (!ls->IsUsing())
        {
            usingSkill_ = false;
            endDirty_ = true;
            lastError_ = ls->GetLastError();
            newRecharge_ = ls->recharge_;
        }
    }
}

AB::GameProtocol::SkillError SkillsComp::UseSkill(int index, bool ping)
{
    if (index == -1)
    {
        lastError_ = AB::GameProtocol::SkillErrorInvalidSkill;
        return lastError_;
    }
    std::shared_ptr<Actor> target;
    if (auto selObj = owner_.GetSelectedObject())
        target = selObj->GetPtr<Actor>();

    // Can use skills only on Creatures not all GameObjects.
    // But a target is not mandatory, the Skill script will decide
    // if it needs a target, and may fail.
    SkillBar* sb = owner_.GetSkillBar();
    auto skill = sb->GetSkill(index);
    assert(skill);
    lastError_ = sb->UseSkill(index, target);
    usingSkill_ = lastError_ == AB::GameProtocol::SkillErrorNone;
    lastSkillIndex_ = index;
    lastSkillTime_ = Utils::Tick();
    lastSkill_ = skill;
    startDirty_ = true;
    endDirty_ = false;
    if (usingSkill_ && ping)
    {
        uint32_t targetId = 0;
        if (skill->HasTarget(SkillTargetTarget))
            targetId = target ? target->id_ : 0;
        owner_.CallEvent<void(uint32_t,AB::GameProtocol::ObjectCallType,int)>(EVENT_ON_PINGOBJECT,
            targetId, AB::GameProtocol::ObjectCallTypeUseSkill, lastSkillIndex_ + 1);
    }
#ifdef DEBUG_AI
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
    {
        LOG_DEBUG << owner_.GetName() << " using invalid skill " <<
                     skill->data_.name <<
                     " on target " << (target ? target->GetName() : "(none)") <<
                     " error = " <<
                     static_cast<int>(lastError_) << std::endl;
    }
#endif
    return lastError_;
}

void SkillsComp::Cancel()
{
    if (!usingSkill_)
        return;

    if (auto ls = lastSkill_.lock())
    {
        if (ls->IsUsing())
        {
            usingSkill_ = false;
            endDirty_ = true;
            newRecharge_ = 0;
            ls->CancelUse();
        }
    }
}

void SkillsComp::CancelWhenChangingState()
{
    if (!usingSkill_)
        return;

    if (auto ls = lastSkill_.lock())
    {
        if (ls->IsUsing() && ls->IsChangingState())
        {
            usingSkill_ = false;
            endDirty_ = true;
            newRecharge_ = 0;
            ls->CancelUse();
        }
    }
}

bool SkillsComp::IsUsing()
{
    if (auto ls = lastSkill_.lock())
        return ls->IsUsing();
    return false;
}

void SkillsComp::Write(Net::NetworkMessage& message)
{
    // The server has 0 based skill indices but for the client these are 1 based
    if (startDirty_)
    {
        if (lastError_ == AB::GameProtocol::SkillErrorNone)
        {
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectUseSkill);
            AB::Packets::Server::ObjectUseSkill packet;
            packet.id = owner_.id_;
            packet.skillIndex = static_cast<int8_t>(lastSkillIndex_ + 1);
            if (auto ls = lastSkill_.lock())
            {
                packet.energy = static_cast<uint16_t>(ls->GetRealEnergy());
                packet.adrenaline = static_cast<uint16_t>(ls->GetRealAdrenaline());
                packet.activation = static_cast<uint16_t>(ls->GetRealActivation());
                packet.overcast = static_cast<uint16_t>(ls->GetRealOvercast());
                packet.hp = static_cast<uint16_t>(ls->GetRealHp());
            }
            else
            {
                packet.energy = 0;
                packet.adrenaline = 0;
                packet.activation = 0;
                packet.overcast = 0;
                packet.hp = 0;
            }
            AB::Packets::Add(packet, message);
        }
        else
        {
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectSkillFailure);
            AB::Packets::Server::ObjectSkillFailure packet = {
                owner_.id_,
                static_cast<int8_t>(lastSkillIndex_ + 1),
                lastError_
            };
            AB::Packets::Add(packet, message);
        }
        startDirty_ = false;
    }

    if (endDirty_)
    {
        if (lastError_ == AB::GameProtocol::SkillErrorNone)
        {
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectEndUseSkill);
            AB::Packets::Server::ObjectSkillSuccess packet = {
                owner_.id_,
                static_cast<int8_t>(lastSkillIndex_ + 1),
                newRecharge_
            };
            AB::Packets::Add(packet, message);
        }
        else
        {
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectSkillFailure);
            AB::Packets::Server::ObjectSkillFailure packet = {
                owner_.id_,
                static_cast<int8_t>(lastSkillIndex_ + 1),
                lastError_
            };
            AB::Packets::Add(packet, message);
        }
        endDirty_ = false;
    }
}

bool SkillsComp::Interrupt(AB::Entities::SkillType type)
{
    if (auto ls = lastSkill_.lock())
    {
        if (ls->IsUsing() && ls->IsType(type))
            return ls->Interrupt();
    }
    return false;
}

Skill* SkillsComp::GetCurrentSkill()
{
    if (auto ls = lastSkill_.lock())
    {
        if (ls->IsUsing())
            return ls.get();
    }
    return nullptr;
}

void SkillsComp::GetResources(int&, int &maxEnergy)
{
    uint32_t estorage = owner_.GetAttributeValue(static_cast<uint32_t>(AttributeIndices::EnergyStorage));
    maxEnergy += (estorage * 3);
}

void SkillsComp::GetSkillRecharge(Skill* skill, uint32_t& recharge)
{
    if (!skill->IsType(AB::Entities::SkillTypeSpell))
        return;
    // Recharge time of all Spells is decreased by the Fastcast attribute in PvE and PvP
    const uint32_t fastcast = owner_.GetAttributeValue(static_cast<uint32_t>(AttributeIndices::FastCast));
    if (fastcast == 0)
        return;
    float reduce = static_cast<float>(recharge) / 100.0f * (3.0f * static_cast<float>(fastcast));
    if (reduce < recharge)
        recharge -= static_cast<uint32_t>(reduce);
    else
        recharge = 0;
}

void SkillsComp::GetSkillCost(Skill* skill,
    int32_t& activation, int32_t& energy, int32_t&, int32_t&, int32_t&)
{
    // Expertise reduces energy cost of ranger skills
    const uint32_t expertise = owner_.GetAttributeValue(static_cast<uint32_t>(AttributeIndices::Expertise));
    if (expertise != 0)
    {
        // Must be a ranger
        if (skill->data_.professionUuid.compare(owner_.skills_->prof1_.uuid) == 0)
        {
            // Seems to be a ranger skill
            float reduce = static_cast<float>(energy) / 100.0f * (4.0f * static_cast<float>(expertise));
            energy -= static_cast<int32_t>(reduce);
            if (energy < 0)
                energy = 0;
        }
    }

    const uint32_t fastcast = owner_.GetAttributeValue(static_cast<uint32_t>(AttributeIndices::FastCast));
    if (fastcast != 0)
    {
        if (skill->IsType(AB::Entities::SkillTypeSignet))
            activation = static_cast<int32_t>(static_cast<float>(activation) * (1.0f - (0.03f * static_cast<float>(fastcast))));
        else if (skill->IsType(AB::Entities::SkillTypeSpell))
            activation = static_cast<int32_t>(static_cast<float>(activation) * powf(0.5f, static_cast<float>(fastcast) / 15.0f));
    }
}

}
}
