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

#pragma once

#include "Skill.h"
#include <queue>
#include <AB/Entities/Profession.h>
#include <abshared/Mechanic.h>
#include <abshared/TemplEncoder.h>
#include <sa/Noncopyable.h>
#include <eastl.hpp>

namespace Game {

class Actor;

typedef ea::array<ea::shared_ptr<Skill>, PLAYER_MAX_SKILLS> SkillsArray;

class SkillBar
{
    NON_COPYABLE(SkillBar)
    NON_MOVEABLE(SkillBar)
private:
    SkillsArray skills_;
    Attributes attributes_;
    Actor& owner_;
    int currentSkillIndex_{ -1 };
    int _LuaAddSkill(uint32_t skillIndex);
    bool _LuaSetSkill(int pos, uint32_t skillIndex);
    std::vector<uint32_t> _LuaGetSkillsWithEffect(uint32_t effect) const
    {
        const auto res = GetSkillsWithEffect(static_cast<SkillEffect>(effect));
        std::vector<uint32_t> luares;
        ea::copy(res.begin(), res.end(), std::back_inserter(luares));
        return luares;
    }
    std::vector<uint32_t> _LuaGetSkillsWithTarget(uint32_t target) const
    {
        const auto res = GetSkillsWithTarget(static_cast<SkillEffectTarget>(target));
        std::vector<uint32_t> luares;
        ea::copy(res.begin(), res.end(), std::back_inserter(luares));
        return luares;
    }
    std::vector<uint32_t> _LuaGetSkillsWithEffectTarget(uint32_t effect, uint32_t target) const
    {
        const auto res = GetSkillsWithEffectTarget(static_cast<SkillEffect>(effect), static_cast<SkillEffectTarget>(target));
        std::vector<uint32_t> luares;
        ea::copy(res.begin(), res.end(), std::back_inserter(luares));
        return luares;
    }
    Skill* _LuaGetSkill(int pos);
    void SetAttributes(const Attributes& attributes);
    void ResetAttributes();
    bool HaveAttribute(uint32_t index);
public:
    static void RegisterLua(kaguya::State& state);

    explicit SkillBar(Actor& owner) :
        owner_(owner)
    { }
    ~SkillBar() = default;

    /// 0 Based
    AB::GameProtocol::SkillError UseSkill(int index, ea::shared_ptr<Actor> target);
    Skill* GetCurrentSkill() const;
    void Update(uint32_t timeElapsed);
    std::string Encode();
    // Load skills from a template. if locked = true also locked skills are allowed.
    bool Load(const std::string& str, bool locked);
    bool SetSecondaryProfession(uint32_t index);

    void ResetSkills();
    ea::shared_ptr<Skill> GetSkill(int pos);
    uint32_t GetIndexOfSkill(int pos);
    bool SetSkill(int pos, ea::shared_ptr<Skill> skill);
    bool RemoveSkill(int pos)
    {
        if (pos < 0 || pos >= PLAYER_MAX_SKILLS)
            return false;
        skills_[static_cast<size_t>(pos)].reset();
        return true;
    }

    void InitAttributes();
    /// Get an attribute
    /// @param index The index of the attribute, not the index in the array
    /// @return The attribute or nullptr
    const AttributeValue* GetAttribute(Attribute index) const;
    const Attributes& GetAttributes() const { return attributes_; }
    bool SetAttributeRank(Attribute index, uint32_t value);
    uint32_t GetAttributeRank(Attribute index) const;
    int GetUsedAttributePoints() const;
    int GetAvailableAttributePoints() const;
    const SkillsArray& GetArray() const { return skills_; }
    ea::vector<uint32_t> GetSkillsWithEffect(SkillEffect effect, bool rechargedOnly = false) const;
    ea::vector<uint32_t> GetSkillsWithTarget(SkillEffectTarget target, bool rechargedOnly = false) const;
    ea::vector<uint32_t> GetSkillsWithEffectTarget(SkillEffect effect, SkillEffectTarget target, bool rechargedOnly = false) const;

    std::string GetClasses() const;

    Skill* operator[](uint32_t index)
    {
        return skills_[index].get();
    }
    template <typename Callback>
    void VisitSkills(Callback&& callback) const
    {
        int index = 0;
        for (const auto& skill : skills_)
        {
            if (skill)
                if (callback(index, *skill) != Iteration::Continue)
                    break;
            ++index;
        }
    }

    AB::Entities::Profession prof1_;
    AB::Entities::Profession prof2_;
};

}
