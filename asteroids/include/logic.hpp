#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wrl.h>
#include <wrl/client.h>

#include "common.hpp"

struct Window;

struct WindowLogic {
    bool Init(Window &window);
    bool on_paint();
    bool on_resize();
    bool on_mousemove();

private:
    Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> render_target;
};
