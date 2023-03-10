#pragma once
#include <cstdint>
#include <cmath>
#include <windows.h>
#include <wrl.h>
#include <wrl/client.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using u64 = uint64_t;
using i64 = int64_t;
using u32 = uint32_t;
using i32 = int32_t;
using u16 = uint16_t;
using i16 = int16_t;
using u8 = uint8_t;
using i8 = int8_t;

using f32 = float;
using f64 = double;

#define PHYSICAL_PIXELS(logical, dpi) i32(ceil(f32((logical) * (dpi)) / 96.))
#define LOGICAL_PIXELS(physical, dpi) i32(f32((physical) * 96) / f32(dpi))

// #define PAINT_CONTOUR_DBG

inline bool key_up(int vkey) {
    return !!(GetAsyncKeyState(vkey) & 0x8000);
}
