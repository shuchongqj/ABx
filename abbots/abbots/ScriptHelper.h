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

#include <kaguya/kaguya.hpp>
#include <string>

void InitLuaState(kaguya::State& state);
std::string GetDataDir();
std::string GetDataFile(const std::string& name);

inline bool IsFunction(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TFUNCTION;
}

inline bool IsVariable(kaguya::State& state, const std::string& name)
{
    auto t = state[name].type();
    return t == LUA_TBOOLEAN || t == LUA_TNUMBER || t == LUA_TSTRING;
}

inline bool IsString(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TSTRING;
}

inline bool IsBool(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TBOOLEAN;
}

inline bool IsNumber(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TNUMBER;
}

inline bool IsNil(kaguya::State& state, const std::string& name)
{
    return state[name].type() == LUA_TNIL;
}
