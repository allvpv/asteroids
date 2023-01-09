#include <iostream>

#include <d2d1_2.h>
#include <comdef.h>

#include "logic.hpp"
#include "window.hpp"

namespace {
    constexpr i32 BACKGROUND_INTERVAL = 40;
    constexpr i32 ASTEROID_INTERVAL = 3;
    constexpr i32 ASTEROID_RADIUS = 30;
}

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

    hr = render_target->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0), &temp_brush);

    if (hr != S_OK) {
        std::wcout << L"Failed to create brush: " << _com_error(hr).ErrorMessage() << '\n';
        return false;
    }

    return background_timer.Init(BACKGROUND_INTERVAL) &&
           new_asteroid_timer.Init(ASTEROID_INTERVAL);
}

void WindowLogic::new_asteroids() {
    const i32 ticks = new_asteroid_timer.get_intervals_count(true);

    for (int i = 0; i < ticks; ++i) {
        const D2D1_SIZE_F size = render_target->GetSize();

        const f64 shift_x = norm_asteroid_x(gen);
        const f64 shift_y = unif_asteroid_y(gen);

        const f32 x_pos = shift_x * size.width;
        const f32 y_pos = -(shift_y * (size.height - 2*ASTEROID_RADIUS) + ASTEROID_RADIUS);

        asteroids.push_front(Asteroid {
            .x = x_pos,
            .y = y_pos,
        });
    }
}

void WindowLogic::update_asteroids() {
    for (auto& a : asteroids) {
        a.y += 5;
    }
}

void WindowLogic::paint_asteroids() {
    for (auto& a : asteroids) {
        std::cout << "Painting ellipse(" << a.x << ", " << a.y << ")\n";
        auto e = D2D1::Ellipse(D2D1::Point2F(a.x, a.y), ASTEROID_RADIUS, ASTEROID_RADIUS);
        render_target->FillEllipse(e, temp_brush.Get());
    }

    const D2D1_SIZE_F size = render_target->GetSize();

    while (!asteroids.empty()) {
        const Asteroid& last = asteroids.back();

        if (last.y > size.height + ASTEROID_RADIUS)
            asteroids.pop_back();
        else
            break;
    }
}

bool WindowLogic::on_paint() {
    new_asteroid_timer.update();
    background_timer.update();

    std::wcout << L"OP\n";
    render_target->BeginDraw();

    f32 progress = 2.f * background_timer.get_interval_progress(true);

    if (progress > 1.f)
        progress = 2.f - progress;

    D2D1_COLOR_F color {
        .r = 0.10f - 0.075f * progress,
        .g = 0.10f - 0.075f * progress,
        .b = 0.20f - 0.075f * progress,
        .a = 1.f,
    };

    render_target->Clear(&color);

    update_asteroids();
    new_asteroids();
    paint_asteroids();

    render_target->EndDraw();

    return true;
}

bool WindowLogic::on_resize() {
    return true;
}

bool WindowLogic::on_mousemove() {
    return true;
}
