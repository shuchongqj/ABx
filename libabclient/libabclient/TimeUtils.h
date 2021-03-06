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

#include <stdint.h>
#include <sys/timeb.h>
#include <time.h>
#include <string>
#include <chrono>

namespace Client {

typedef int64_t ticks_t;

ticks_t time();
ticks_t millis();
ticks_t micros();
void millisleep(size_t ms);
void microsleep(size_t us);
std::string format_tick(int64_t tick);

struct timer
{
private:
    ticks_t start_;
public:
    timer() :
        start_(0)
    {
        restart();
    }
    float elapsed_seconds() { return static_cast<float>(micros() - start_) / 1000000.0f; }
    ticks_t elapsed_millis() { return (micros() - start_) / 1000; }
    ticks_t elapsed_micros() { return micros() - start_; }
    void restart() { start_ = micros(); }
};

inline int64_t AbTick()
{
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    );
    return ms.count();
}

struct TimeSpan
{
    uint32_t months = 0;
    uint32_t days = 0;
    uint32_t hours = 0;
    uint32_t minutes = 0;
    uint32_t seconds = 0;
    TimeSpan(uint32_t sec)
    {
        time_t secs(sec); // you have to convert your input_seconds into time_t
#ifdef _MSC_VER
        struct tm p;
        errno_t err = gmtime_s(&p, &secs); // convert to broken down time
        if (err)
            return;

        if (p.tm_yday > 31)
        {
            months = p.tm_yday / 31;
            days = p.tm_yday - (months * 31);
        }
        else
            days = p.tm_yday;
        hours = p.tm_hour;
        minutes = p.tm_min;
        seconds = p.tm_sec;
#else
        struct tm* p;
        p = gmtime(&secs); // convert to broken down time
        if (!p)
            return;

        if (p->tm_yday > 31)
        {
            months = p->tm_yday / 31;
            days = p->tm_yday - (months * 31);
        }
        else
            days = p->tm_yday;
        hours = p->tm_hour;
        minutes = p->tm_min;
        seconds = p->tm_sec;
#endif
    }
};

}
