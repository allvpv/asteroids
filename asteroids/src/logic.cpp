#include <iostream>
#include <utility>

#include <d2d1_2.h>
#include <comdef.h>

#include "logic.hpp"
#include "window.hpp"

namespace {
    constexpr i32 MOVE_INTERVAL = 0'005;
    constexpr i32 BACKGROUND_INTERVAL = 4'000;
    constexpr i32 ASTEROID_INTERVAL = 0'600;
    constexpr i32 ASTEROID_RADIUS = 30;

    constexpr std::pair<i32, i32> CONTROLLER_ELLIPSE_RADIUS = { 30, 60 };

    const wchar_t* rocket_file = L"C:\\Dane\\directx_rust\\asteroids\\build\\rocket.png";

    namespace ErrorCollection {
        void com_crash(HRESULT hr) {
            std::wcout << L"Failed to initialize the COM library: " << _com_error(hr).ErrorMessage()
                       << L'\n';
        }

        void factory_crash(HRESULT hr) {
            std::wcout << L"Failed to initialize ID2D1Factory: " << _com_error(hr).ErrorMessage()
                       << L'\n';
        }

        void render_target_crash(HRESULT hr) {
            std::wcout << L"Failed to initialize HwndRenderTarget: "
                       << _com_error(hr).ErrorMessage() << L'\n';
        }

        void brush_crash(HRESULT hr) {
            std::wcout << L"Failed to create brush: " << _com_error(hr).ErrorMessage() << L'\n';
        }

        void imaging_factory_crash(HRESULT hr) {
            std::wcout << L"Failed to create ImagingFactory: " << _com_error(hr).ErrorMessage()
                       << L'\n';
        }

        void bitmap_crash(const wchar_t* filename, HRESULT hr) {
            std::wcout << L"umieram kurwa\n";
            std::wcout << L"Failed to load bitmap (" << filename << L"): "
                       << _com_error(hr).ErrorMessage() << L'\n';
        }
    }
}

bool WindowLogic::Init() {
    HRESULT hr;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (hr != S_OK) {
        ErrorCollection::com_crash(hr);
        return false;
    }

    D2D1_FACTORY_OPTIONS options = {};

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
                           __uuidof(ID2D1Factory2),
                           &options,
                           &d2d_factory);
    if (hr != S_OK) {
        ErrorCollection::factory_crash(hr);
        return false;
    }

    D2D1_SIZE_U size = D2D1::SizeU(window.get_inner_width(), window.get_inner_height());

    controller_x = size.width / 2;
    controller_y = size.height - 60;

    hr = d2d_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(window.get_handle(), size),
        &render_target);

    if (hr != S_OK || !render_target) {
        ErrorCollection::render_target_crash(hr);
        return false;
    }

    hr = render_target->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 0.f), &temp_asteroid_brush);

    if (hr != S_OK || !temp_asteroid_brush) {
        ErrorCollection::brush_crash(hr);
        return false;
    }

    hr = render_target->CreateSolidColorBrush(D2D1::ColorF(0.f, 1.f, 0.f), &temp_controller_brush);

    if (hr != S_OK || !temp_controller_brush) {
        ErrorCollection::brush_crash(hr);
        return false;
    }

    ComPtr<IWICImagingFactory2> imaging_factory;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory2),
        &imaging_factory
    );

    if (hr != S_OK || !imaging_factory) {
        ErrorCollection::imaging_factory_crash(hr);
        return false;
    }

    auto result = load_bitmap_from_file(*render_target.Get(), *imaging_factory.Get(), rocket_file);

    if (result.first != S_OK || !result.second) {
        ErrorCollection::bitmap_crash(rocket_file, hr);
        return false;
    } else {
        rocket_bitmap = result.second;
    }

    return background_timer.Init(BACKGROUND_INTERVAL) &&
           new_asteroid_timer.Init(ASTEROID_INTERVAL) &&
           move_asteroid_timer.Init(MOVE_INTERVAL);
}

void WindowLogic::paint_controller() {
    auto [width, height] = CONTROLLER_ELLIPSE_RADIUS;

    auto e = D2D1::Ellipse(D2D1::Point2F(controller_x, controller_y), width, height);
    render_target->FillEllipse(e, temp_controller_brush.Get());
}

void WindowLogic::new_asteroids() {
    const i32 ticks = new_asteroid_timer.get_intervals_count(true);

    if (ticks) {
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

void WindowLogic::controller_move() {
    bool keypress_left = key_up(VK_LEFT);
    bool keypress_right = key_up(VK_RIGHT);

    auto accelerate = [](f32& acceleration) {
        if (acceleration < 1.5)
            acceleration += 1.125;
        else if (acceleration < 3)
            acceleration += 0.75;
        else if (acceleration < 5)
            acceleration += 0.375;
        else if (acceleration < 15)
            acceleration += 0.25;
    };

    auto decelerate = [](f32& acceleration, f32 step) {
        acceleration = max(0.f, acceleration - step);
    };

    bound_left = size.width / 5;
    bound_right = size.width - bound_left;

    if (keypress_left && controller_x > bound_left) {
        accelerate(accel_left);
        decelerate(accel_right, 1.f);
    }
    else if (keypress_right && controller_x < bound_right) {
        accelerate(accel_right);
        decelerate(accel_left, 1.f);
    }
    else {
        decelerate(accel_right, 0.75f);
        decelerate(accel_left, 0.75f);
    }

    controller_x += accel_right - accel_left;
}

void WindowLogic::update_motion() {
    const i32 ticks = move_asteroid_timer.get_intervals_count(true);

    for (auto& a : asteroids) {
        a.y += ticks;
    }

    if (ticks) {
        controller_move();
    }
}

void WindowLogic::paint_asteroids() {
    for (auto& a : asteroids) {
        auto e = D2D1::Ellipse(D2D1::Point2F(a.x, a.y), ASTEROID_RADIUS, ASTEROID_RADIUS);
        render_target->FillEllipse(e, temp_asteroid_brush.Get());
    }

    while (!asteroids.empty()) {
        const Asteroid& last = asteroids.back();

        if (last.y > size.height + ASTEROID_RADIUS)
            asteroids.pop_back();
        else
            break;
    }
}

Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> WindowLogic::CreateBackgroundGradient() {
    D2D1_GRADIENT_STOP gradient_stops_arr[3];

    gradient_stops_arr[0].color = side_bg;
    gradient_stops_arr[0].position = 0.0f;
    gradient_stops_arr[1].color = middle_bg;
    gradient_stops_arr[1].position = 0.5f;
    gradient_stops_arr[2].color = side_bg;
    gradient_stops_arr[2].position = 1.0f;

    Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> gradient_stops;

    HRESULT hr;

    hr = render_target->CreateGradientStopCollection(
        gradient_stops_arr,
        3,
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &gradient_stops
    );

    if (hr != S_OK || !gradient_stops)
        return {};


    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> gradient_brush;

    hr = render_target->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(0, 0),
                D2D1::Point2F(size.width, 0)),
            gradient_stops.Get(),
            &gradient_brush
    );

    if (hr != S_OK || !gradient_brush)
        return {};

    return gradient_brush;
}

bool WindowLogic::on_paint() {
    size = render_target->GetSize();

    background_timer.update();
    new_asteroid_timer.update();
    move_asteroid_timer.update();

    std::wcout << L"maluje\n";

    update_motion();
    new_asteroids();

    f32 progress = 2.f * background_timer.get_interval_progress(true);

    if (progress > 1.f)
        progress = 2.f - progress;

    side_bg = reference_bg;
    middle_bg = reference_bg;

    f32 penalty = max(bound_left - controller_x, controller_x - bound_right);

    if (penalty > 0)
        side_bg.r += (penalty / bound_left);
    else {
        f32 reward = controller_x - size.width / 2;

        if (reward < 0)
            reward *= -1;

        f32 reward_bound = size.width / 5;

        if (reward < reward_bound)
            middle_bg.b += 0.3f * (1.f - reward / reward_bound);
    }

    std::wcout << L"malu2\n";

    D2D1_RECT_F plane = {
        .left = 0.,
        .top = 0.,
        .right = size.width,
        .bottom = size.height,
    };

    render_target->BeginDraw();
    auto bg_grad = CreateBackgroundGradient();

    if (!bg_grad) {
        std::wcout << "Cannot create background gradient\n";
        return false;
    }

    render_target->FillRectangle(&plane, bg_grad.Get());

    paint_asteroids();
    paint_controller();
    render_target->EndDraw();
    std::wcout << L"malu3\n";

    return true;
}

bool WindowLogic::on_resize() {
    return true;
}

bool WindowLogic::on_mousemove() {
    return true;
}

