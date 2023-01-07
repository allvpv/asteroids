#pragma once
#include <stdint.h>
#include <iostream>

using u32 = uint32_t;
using i32 = int32_t;
using u16 = uint16_t;
using i16 = int16_t;
using u8 = uint8_t;
using i8 = int8_t;

using f32 = float;
using f64 = double;

constexpr i32 physical_pixels(i32 logical, u32 dpi) {
    return i32(f32(logical * dpi) / 96.);
}

constexpr i32 logical_pixels(i32 physical, u32 dpi) {
    return u32(f32(physical * dpi) / f32(dpi));
}

