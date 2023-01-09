#pragma once
#include <iostream>
#include <windows.h>
#include "common.hpp"

struct Timer {
    bool Init(i64 interval_in_milliseconds) {
        if (!QueryPerformanceFrequency((LARGE_INTEGER*) &frequency) ||
            !QueryPerformanceCounter((LARGE_INTEGER*) &last_time)) {
            return false;
        }

        std::wcout << "FREQ: " << frequency << '\n';

        reference_time = last_time;
        interval = (frequency * interval_in_milliseconds) / 1000;

        return true;
    }

    bool update() {
        return QueryPerformanceCounter((LARGE_INTEGER*) &last_time);
    }

    // Returns [0, 1)
    f32 get_interval_progress(bool forget_elapsed_intervals) {
        i64 delta = last_time - reference_time;
        i64 count = delta / interval;
        i64 modulo = delta % interval;

        if (forget_elapsed_intervals)
            reference_time += count * interval;

        return f32(modulo) / f32(interval);
    }

    // How many intervals elapsed since last call? This method does not start a new interval.
    i32 get_intervals_count(bool forget_elapsed_intervals) {
        i64 delta = last_time - reference_time;
        i64 count = delta / interval;

        if (forget_elapsed_intervals)
            reference_time += count * interval;

        return i32 (count);
    }

protected:
    i64 last_time;
    i64 reference_time;
    i64 frequency;
    i64 interval;
};

