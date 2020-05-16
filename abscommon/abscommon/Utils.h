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

#include <sys/timeb.h>
#include <time.h>
#include <random>
#include <iterator>
#include <chrono>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);

/// Check if value would exceed max if add was added
template <typename T>
inline bool WouldExceed(T value, T add, T max)
{
    // Don't do return value + add > max because max maybe the largest possible value,
    // e.g. std::numeric_limits<>::max()
    return (max - add) < value;
}

/// Count of elements in an array
template <typename T, int N>
constexpr size_t CountOf(T(&)[N])
{
    return N;
}

inline int64_t Tick()
{
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    );
    return ms.count();
}

inline bool IsExpired(int64_t expiresAt)
{
    return expiresAt < Tick();
}

/// Return the time that's elapsed since in ms
inline uint32_t TimeElapsed(int64_t since)
{
    auto tick = Tick();
    if (tick > since)
        return static_cast<uint32_t>(tick - since);
    return 0u;
}

}
