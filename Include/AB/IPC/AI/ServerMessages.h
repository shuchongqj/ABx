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

#include <string>

namespace AI {

struct GameAdd
{
    uint32_t id;
    std::string name;
    std::string mapUuid;
    std::string instanceUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(name);
        ar.value(mapUuid);
        ar.value(instanceUuid);
    }
};

struct GameRemove
{
    uint32_t id;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
    }
};

struct ObjectUpdate
{
    enum class ObjectType : uint8_t
    {
        Unknown,
        Npc,
        Player
    };

    uint32_t id;
    uint32_t gameId{ 0 };
    ObjectType objectType{ ObjectType::Unknown };
    std::string name;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(objectType);
        ar.value(gameId);
        ar.value(name);
    }
};

}