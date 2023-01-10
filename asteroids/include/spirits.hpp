#pragma once
#include "math.hpp"

struct SpiritData {
    const wchar_t* filename;
    const ObjectContour contour;
};

inline SpiritData controller_data {
    .filename = L"rocket.png",
    .contour = {
        .vertices = {
            { 0.0, -56.0 },
            { 19.0, -25.5 },
            { 22.0, 11.5 },
            { 30.5, 24.0 },
            { 29.5, 46.5 },
            { 14.5, 37.5 },
            { 8.0, 53.0 },
            { -8.5, 53.0 },
            { -15.0, 37.5 },
            { -29.0, 45.5 },
            { -30.5, 24.0 },
            { -22.0, 11.0 },
            { -19.0, -24.5 },
        },
        .half_of_sides = { 32.0, 56.0 },
    },
};

inline SpiritData asteroid_data {
    .filename = L"asteroid_small.png",
    .contour = {
        .vertices = {
            { -28.2, -35.45 },
            { 15.6, -40.85 },
            { 37.8, -23.65 },
            { 39.0, -3.85 },
            { 44.0, 8.15 },
            { 31.0, 37.35 },
            { -11.2, 38.75 },
            { -29.6, 29.75 },
            { -45.4, -2.25 },
        },
        .half_of_sides = { 50.0, 43.85 },
    },
};

inline SpiritData bullet_data {
    .filename = L"bullet.png",
    .contour = {
        .vertices = {
            { 0.5, -54.12 },
            { 4.25, -42.62 },
            { 5.0, -17.62 },
            { -2.75, -17.62 },
            { -2.25, -42.62 },
        },
        .half_of_sides = { 12.75, 71.38 },
    }
};
