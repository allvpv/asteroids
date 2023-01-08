#include <iostream>
#include <d2d1_2.h>
#include <comdef.h>

#include "logic.hpp"
#include "window.hpp"

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

    return true;
}

bool WindowLogic::on_paint() {
    std::wcout << L"OP\n";
    render_target->BeginDraw();

    constexpr i64 PERIOD_I = 2;
    constexpr f32 PERIOD_F = 2;

    f32 progress = window.get_timer().get_time(PERIOD_I * 2);
    f32 change;

    if (progress > PERIOD_F)
        change = 2.f * PERIOD_F - progress;
    else
        change = progress;

    change /= 2.f; // normalize

    D2D1_COLOR_F color {
        .r = 0.10f - 0.075f * change,
        .g = 0.10f - 0.075f * change,
        .b = 0.20f - 0.075f * change,
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
