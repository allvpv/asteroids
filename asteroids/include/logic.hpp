#pragma once
#include <windows.h>
#include "common.hpp"

struct Window;

struct WindowLogic {
    bool Init(Window &window);
    bool on_paint();
    bool on_resize();
    bool on_mousemove();
};
