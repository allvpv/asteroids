#pragma once
#include <windows.h>
#include "common.hpp"

struct Timer {
    bool Init() {
        return QueryPerformanceFrequency((LARGE_INTEGER*) &frequency)
            && QueryPerformanceCounter((LARGE_INTEGER*) &start_time);
    }

    f32 get_time(i64 period_in_seconds) {
        i64 delta = update_time - start_time;
        i64 period = frequency * period_in_seconds;
        i64 time = delta % period;

        return f32(time) / f32(frequency);
    }

    bool update() {
        return QueryPerformanceCounter((LARGE_INTEGER*) &update_time);
    }

    i64 get_start_time() { return start_time; }
    i64 get_update_time() { return update_time; }
    i64 get_frequency() { return frequency; }

private:
    i64 start_time;
    i64 update_time;
    i64 frequency;
};
