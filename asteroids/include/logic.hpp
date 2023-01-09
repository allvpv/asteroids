#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wrl.h>
#include <wrl/client.h>

#include "common.hpp"
#include "timer.hpp"

struct Window;

struct WindowLogic {
    bool Init();
    bool on_paint();
    bool on_resize();
    bool on_mousemove();

    void new_asteroids();

    WindowLogic(Window& window) : window(window) {}

private:
    Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> render_target;
    Window& window;

    Timer background_timer;
    Timer asteroid_timer;

    i64 last_asteroid_update;
};
