#include <iostream>
#include <d2d1_2.h>
#include <comdef.h>

#include "logic.hpp"
#include "window.hpp"

constexpr i32 BACKGROUND_INTERVAL = 40;
constexpr i32 ASTEROID_INTERVAL = 5;

bool WindowLogic::Init() {
    HRESULT hr;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (hr != S_OK) {
        std::wcout << L"Failed to initialize the COM library: " << _com_error(hr).ErrorMessage()
                   << '\n';
        return false;
    }

    D2D1_FACTORY_OPTIONS options = {};

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
                           __uuidof(ID2D1Factory2),
                           &options,
                           &d2d_factory);
    if (hr != S_OK) {
        std::wcout << L"Failed to initialize ID2D1Factory: " << _com_error(hr).ErrorMessage()
                   << '\n';
        return false;
    }

    D2D1_SIZE_U size = D2D1::SizeU(window.get_inner_width(), window.get_inner_height());

    hr = d2d_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(window.get_handle(), size),
        &render_target);

    if (hr != S_OK) {
        std::wcout << L"Failed to initialize HwndRenderTarget: " << _com_error(hr).ErrorMessage()
                   << '\n';
        return false;
    }

    return background_timer.Init(BACKGROUND_INTERVAL) &&
           asteroid_timer.Init(ASTEROID_INTERVAL);
}

void WindowLogic::new_asteroids() {
    i32 ticks = asteroid_timer.get_intervals_count(true);

    for (int i = 0; i < ticks; ++i) {
        std::cout << "GEN\n";
    }
}

bool WindowLogic::on_paint() {
    asteroid_timer.update();
    background_timer.update();

    std::wcout << L"OP\n";
    render_target->BeginDraw();

    new_asteroids();

    f32 progress = 2.f * background_timer.get_interval_progress(true);

    if (progress > 1.f)
        progress = 2.f - progress;

    std::wcout << "PR: " << progress << '\n';

    D2D1_COLOR_F color {
        .r = 0.10f - 0.075f * progress,
        .g = 0.10f - 0.075f * progress,
        .b = 0.20f - 0.075f * progress,
        .a = 1.f,
    };

    render_target->Clear(&color);
    render_target->EndDraw();

    return true;
}

bool WindowLogic::on_resize() {
    return true;
}

bool WindowLogic::on_mousemove() {
    return true;
}
