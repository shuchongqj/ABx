#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_PROFESSIONS = "game_professions";

struct Profession : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_PROFESSIONS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(name, Limits::MAX_PROFESSION_NAME);
        s.text1b(abbr, Limits::MAX_PROFESSION_ABBR);
        s.value4b(modelIndexFemale);
        s.value4b(modelIndexMale);
        s.value4b(attributeCount);
        s.container(attributeUuids, Limits::MAX_PROFESSION_ATTRIBUTES, [&s](std::string& a)
        {
            s.text1b(a, Limits::MAX_UUID);
        });
    }

    uint32_t index = INVALID_INDEX;
    std::string name;
    std::string abbr;
    uint32_t modelIndexFemale = 0;
    uint32_t modelIndexMale = 0;
    uint32_t attributeCount = 0;
    std::vector<std::string> attributeUuids;
};

}
}
