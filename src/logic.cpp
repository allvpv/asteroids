#include "logic.hpp"

#include <iostream>
#include <utility>
#include <cmath>

#include <d2d1_2.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d2d1helper.h>
#include <comdef.h>

#include "common.hpp"
#include "window.hpp"
#include "math.hpp"
#include "timer.hpp"
#include "bitmap_helper.hpp"

namespace {
    constexpr i32 MOVE_INTERVAL = 0'005;
    constexpr i32 BACKGROUND_INTERVAL = 4'000;
    constexpr i32 ASTEROID_INTERVAL = 0'600;
    constexpr i32 NEW_BULLET_INTERVAL = 0'200;
    constexpr i32 PENALTY_INTERVAL = 0'300;
    constexpr i32 FADE_OUT_INTERVAL = 2'000;
    constexpr f32 BULLET_SPEED = 3;
    constexpr i32 TYPE_SPEED = 0'100;

    constexpr D2D1_COLOR_F reference_bg {
        .r = 0.10f - 0.075f,
        .g = 0.10f - 0.075f,
        .b = 0.20f - 0.075f,
        .a = 1.f,
    };

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
            std::wcout << L"Failed to load bitmap (" << filename << L"): "
                       << _com_error(hr).ErrorMessage() << L'\n';
        }

        void text_helper_crash() {
            std::wcout << L"Failed to create text helper for drawing text\n";
        }
    }
}

void WindowLogic::reset_controller_pos() {
    controller_pos.x = size.width / 2;
    controller_pos.y = size.height - 60;
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
                           __uuidof(ID2D1Factory1),
                           &options,
                           &d2d_factory);
    if (hr != S_OK) {
        ErrorCollection::factory_crash(hr);
        return false;
    }

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    hr = D3D11CreateDevice(
        nullptr,                    // specify null to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        creationFlags,              // optionally set debug and Direct2D compatibility flags
        feature_levels,             // list of feature levels this app can support
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        &device,                    // returns the Direct3D device created
        nullptr,                    // returns feature level of device created
        &context                    // returns the device immediate context
    );

    if (hr != S_OK) {
        return false;
    }

    hr = device.As(&dxgi_device);

    if (hr != S_OK) {
        return false;
    }

    hr = d2d_factory->CreateDevice(dxgi_device.Get(), &d2d_device);

    if (hr != S_OK) {
        return false;
    }

    hr = d2d_device->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
            &target);

    if (hr != S_OK) {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
        .Width = 0, // use automatic sizing
        .Height = 0,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM, // this is the most common swapchain format
        .Stereo = false,
        .SampleDesc = {
            .Count = 1, // don't use multi-sampling
            .Quality = 0,
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2, // use double buffering to enable flip
        .Scaling = DXGI_SCALING_NONE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL, // all apps must use this SwapEffect
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
    };

    hr = dxgi_device->GetAdapter(&dxgi_adapter);

    if (hr != S_OK)
        return false;

    hr = dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));

    if (hr != S_OK)
        return false;

    hr = dxgi_factory->CreateSwapChainForHwnd(
        device.Get(),
        window.get_handle(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &dxgi_swapchain
    );

    if (hr != S_OK)
        return false;

    hr = dxgi_device->SetMaximumFrameLatency(1);

    if (hr != S_OK)
        return false;

    hr = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));

    if (hr != S_OK)
        return false;

    u32 dpi = GetDpiForWindow(window.get_handle());

    std::wcout << "DPI: " << dpi << '\n';

    D2D1_BITMAP_PROPERTIES1 bitmap_properties =
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
            (f32) 96,
            (f32) 96
        );

    hr = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&dxgi_backbuffer));

    if (hr != S_OK)
        return false;

    hr = target->CreateBitmapFromDxgiSurface(
        dxgi_backbuffer.Get(),
        &bitmap_properties,
        &target_bitmap
    );

    if (hr != S_OK)
        return false;

    target->SetTarget(target_bitmap.Get());


#ifdef PAINT_CONTOUR_DBG
    hr = target->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 0.f), &contour_brush);

    if (hr != S_OK || !contour_brush) {
        ErrorCollection::brush_crash(hr);
        return false;
    }
#endif // PAINT_CONTOUR_DBG

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

    auto load_bitmap = [&](const wchar_t* fname, Microsoft::WRL::ComPtr<ID2D1Bitmap>& bitmap) {
        auto result = load_bitmap_from_file(*target.Get(), *imaging_factory.Get(), fname);

        if (result.first != S_OK || !result.second) {
            ErrorCollection::bitmap_crash(fname, result.first);
            return false;
        } else {
            bitmap = result.second;
        }

        return true;
    };

    if (!load_bitmap(spirits.controller.filename, controller_bitmap) ||
        !load_bitmap(spirits.asteroid.filename, asteroid_bitmap) ||
        !load_bitmap(spirits.bullet.filename, bullet_bitmap)) {
        return false;
    }

    dpi_factor = (f32) GetDpiForWindow(window.get_handle()) / 96.f;

    spirits.controller.contour.update_for_dpi(dpi_factor);
    spirits.asteroid.contour.update_for_dpi(dpi_factor);
    spirits.bullet.contour.update_for_dpi(dpi_factor);

    auto dpi_scale = [dpi_factor = dpi_factor](D2D1_SIZE_F& size, f32 factor) {
        size.width = size.width * factor * dpi_factor;
        size.height = size.height * factor * dpi_factor;
    };

    controller_bmp_size = controller_bitmap->GetSize();
    asteroid_bmp_size = asteroid_bitmap->GetSize();
    bullet_bmp_size = bullet_bitmap->GetSize();

    dpi_scale(controller_bmp_size, spirits.controller.scale);
    dpi_scale(asteroid_bmp_size, spirits.asteroid.scale);
    dpi_scale(bullet_bmp_size, spirits.bullet.scale);

    if (!text_helper.Init(target)) {
        ErrorCollection::text_helper_crash();
        return false;
    }

    size = target->GetSize();
    reset_controller_pos();

    if (!set_asteroid_frequency())
        return false;

    D2D1_COLOR_F red_bg = reference_bg;
    D2D1_COLOR_F blue_bg = reference_bg;

    red_bg.r = 1.f;
    blue_bg.b += 0.3f;

    red_background_brush = create_gradient(red_bg, reference_bg);

    if (!red_background_brush) {
        std::wcout << L"Cannot create red gradient background brush\n";
    }

    blue_background_brush = create_gradient(reference_bg, blue_bg);

    if (!blue_background_brush) {
        std::wcout << L"Cannot create blue gradient background brush\n";
    }

    return background_timer.Init(BACKGROUND_INTERVAL) &&
           move_timer.Init(MOVE_INTERVAL) &&
           new_bullet_timer.Init(NEW_BULLET_INTERVAL) &&
           penalty_timer.Init(PENALTY_INTERVAL);
}

bool WindowLogic::set_asteroid_frequency() {
    return new_asteroid_timer.Init((7 - difficulty) * 100);
}

void WindowLogic::new_bullets() {
    if (bullet_forbidden) {
        i32 ticks = new_bullet_timer.get_intervals_count(false);

        if (ticks) {
            bullet_forbidden = false;
        }
    }

    bool space_press = key_up(VK_SPACE);

    if (bullet_forbidden || !space_press || State != GAME_PLAY)
        return;

    bullet_forbidden = true;
    new_bullet_timer.start_new_interval();

    Vector bullet_pos { controller_pos };

    bullet_pos.y -= spirits.controller.contour.half_of_sides.y;

    bullets.push_front(Bullet {
        .pos = bullet_pos,
        .destroyed = false,
        .size = 1.f,
    });
}

void WindowLogic::new_asteroids() {
    const i32 ticks = new_asteroid_timer.get_intervals_count(true);

    if (ticks && (State == GAME_PLAY || State == FADE_IN)) {
        const f32 asteroid_radius = spirits.asteroid.contour.half_of_sides.y;

        const f32 shift_x = norm_asteroid_x(gen);
        const f32 shift_y = unif_asteroid_y(gen);

        const f32 x_pos = shift_x * size.width;
        const f32 y_pos = -(shift_y * (300 - 2 * asteroid_radius) + asteroid_radius);

        const f32 speed = unif_speed(gen);

        asteroids.push_front(Asteroid {
            .pos = Vector(x_pos, y_pos),
            .speed = speed * dpi_factor,
            .destroyed = false,
            .size = 1.f,
        });
    }
}

bool WindowLogic::is_there_collision() {
    for (const auto& a : asteroids) {
        if (a.destroyed)
            continue;

        if (a.pos.y < size.height / 2)
            continue;

        if (intersect(spirits.asteroid.contour, spirits.controller.contour,
                      a.pos, controller_pos)) {
            controller_downspeed = 3.f * a.speed;
            return true;
        }
    }

    return false;
}

void WindowLogic::controller_move(const f32 shift) {
    if (State == GAME_OVER) {
        controller_pos.y += shift * controller_downspeed;
        return;
    }

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

    if (keypress_left && controller_pos.x > bound_left) {
        accelerate(accel_left);
        decelerate(accel_right, 1.f);
    }
    else if (keypress_right && controller_pos.x < bound_right) {
        accelerate(accel_right);
        decelerate(accel_left, 1.f);
    }
    else {
        decelerate(accel_right, 0.75f);
        decelerate(accel_left, 0.75f);
    }

    controller_pos.x += (accel_right - accel_left) * shift * 0.4f * dpi_factor;
}

void WindowLogic::asteroids_move(const f32 shift) {
    for (auto& a : asteroids) {
        a.pos.y += shift * a.speed;
    }
}

void WindowLogic::bullets_move(const f32 shift) {
    for (auto& b : bullets) {
        b.pos.y -= shift * BULLET_SPEED * dpi_factor;
    }
}

void WindowLogic::game_over_move(const f32 shift) {
    if (State == GAME_OVER && game_over_progress < 1.f)
        game_over_progress += shift / 128.f;

    if (State == FADE_IN && fade_in_progress < 1.f)
        fade_in_progress += shift / 128.f;
}


void WindowLogic::update_motion() {
    const f32 shift = move_timer.get_intervals_continuous();
    move_timer.start_new_interval();

    asteroids_move(shift);
    controller_move(shift);
    bullets_move(shift);
    game_over_move(shift);
}

void WindowLogic::paint_controller() {
    f32 hlfw_x = controller_bmp_size.width / 2;
    f32 hlfw_y = controller_bmp_size.height / 2;

    // Draw a bitmap.
    target->DrawBitmap(
        controller_bitmap.Get(),
        D2D1::RectF(
            controller_pos.x - hlfw_x,
            controller_pos.y - hlfw_y,
            controller_pos.x + hlfw_x,
            controller_pos.y + hlfw_y
        )
    );

#ifdef PAINT_CONTOUR_DBG
    paint_contour_dbg(spirits.controller.contour, controller_pos);
#endif // PAINT_CONTOUR_DBG
}

#ifdef PAINT_CONTOUR_DBG
void WindowLogic::paint_contour_dbg(const ObjectContour& contour, const Vector& center) {
    for (size_t i = 0; i < contour.vertices.size(); ++i) {
        const Vector& av = contour.vertices[i];
        const Vector& bv = contour.vertices[i + 1 < contour.vertices.size() ? i + 1 : 0];

        Vector suma = av + center;
        Vector sumb = bv + center;

        target->DrawLine(
            D2D1_POINT_2F(suma.x, suma.y),
            D2D1_POINT_2F(sumb.x, sumb.y),
            contour_brush.Get(),
            2.,
            NULL);
    }
}
#endif // PAINT_CONTOUR_DBG

void WindowLogic::collect_garbage() {
    while (!asteroids.empty()) {
        const Asteroid& last = asteroids.back();

        bool very_small = last.size < 0.1f;

        if (!asteroid_visible(last) || very_small)
            asteroids.pop_back();
        else
            break;
    }

    while (!bullets.empty()) {
        const Bullet& last = bullets.back();

        bool very_small = last.size < 0.1f;

        if (!bullet_visible(last) || very_small)
            bullets.pop_back();
        else
            break;
    }
}

void WindowLogic::paint_asteroids() {
    f32 hlfw_x = asteroid_bmp_size.width / 2;
    f32 hlfw_y = asteroid_bmp_size.height / 2;

    for (auto& a : asteroids) {
        f32 this_hlfw_x = hlfw_x;
        f32 this_hlfw_y = hlfw_y;

        if (a.destroyed) {
            this_hlfw_x *= a.size;
            this_hlfw_y *= a.size;
        }

        target->DrawBitmap(
            asteroid_bitmap.Get(),
            D2D1::RectF(
                a.pos.x - this_hlfw_x,
                a.pos.y - this_hlfw_y,
                a.pos.x + this_hlfw_x,
                a.pos.y + this_hlfw_y
            )
        );

#ifdef PAINT_CONTOUR_DBG
        paint_contour_dbg(spirits.asteroid.contour, a.pos);
#endif // PAINT_CONTOUR_DBG
    }
}

void WindowLogic::paint_bullets() {
    f32 hlfw_x = bullet_bmp_size.width / 2;
    f32 hlfw_y = bullet_bmp_size.height / 2;

    for (const auto& bullet : bullets) {
        f32 this_hlfw_x = hlfw_x;
        f32 this_hlfw_y = hlfw_y;

        if (bullet.destroyed) {
            this_hlfw_x *= bullet.size;
            this_hlfw_y *= bullet.size;
        }

        // Draw a bitmap.
        target->DrawBitmap(
            bullet_bitmap.Get(),
            D2D1::RectF(
                bullet.pos.x - this_hlfw_x,
                bullet.pos.y - this_hlfw_y,
                bullet.pos.x + this_hlfw_x,
                bullet.pos.y + this_hlfw_y
            )
        );

#ifdef PAINT_CONTOUR_DBG
        paint_contour_dbg(spirits.bullet.contour, bullet.pos);
#endif // PAINT_CONTOUR_DBG
    }
}

ComPtr<ID2D1LinearGradientBrush> WindowLogic::create_gradient(D2D1_COLOR_F side_bg,
                                                              D2D1_COLOR_F middle_bg) {
    D2D1_GRADIENT_STOP gradient_stops_arr[3];

    gradient_stops_arr[0].color = side_bg;
    gradient_stops_arr[0].position = 0.0f;
    gradient_stops_arr[1].color = middle_bg;
    gradient_stops_arr[1].position = 0.5f;
    gradient_stops_arr[2].color = side_bg;
    gradient_stops_arr[2].position = 1.0f;

    Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> gradient_stops;

    HRESULT hr;

    hr = target->CreateGradientStopCollection(
        gradient_stops_arr,
        3,
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &gradient_stops
    );

    if (hr != S_OK || !gradient_stops)
        return {};

    ComPtr<ID2D1LinearGradientBrush> brush;

    hr = target->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(0, 0),
                D2D1::Point2F(size.width, 0)),
            gradient_stops.Get(),
            &brush
    );

    if (hr != S_OK || !brush)
        return {};

    return brush;
}

void WindowLogic::compute_penalty() {
    f32 paint_blue_bound = size.width / 5;
    f32 penalty_bound = size.width / 3;

    penalty = max(penalty_bound - controller_pos.x, controller_pos.x + penalty_bound - size.width);
    paint_blue = fabsf(controller_pos.x - size.width / 2);

    if (penalty > 0.f)
        penalty = penalty / penalty_bound;
    else
        penalty = 0.f;

    if (paint_blue < paint_blue_bound)
        paint_blue = 1.f - paint_blue / paint_blue_bound;
    else
        paint_blue = 0.f;

    if (penalty_timer.get_intervals_count(true)) {
        if (penalty == 0)
            penalty_points_total = 0;
        else if (State == GAME_PLAY) {
            i32 penalty_points_unit = (i32) std::floor(penalty * 5.f);
            penalty_points_total += penalty_points_unit;
            score -= penalty_points_unit;
        }
    }
}

void WindowLogic::destroy_asteroids() {
    for (auto& a : asteroids) {
        if (a.destroyed) {
            if (a.size > 0)
                a.size -= 0.05f;
        }
    }

    for (auto& b : bullets) {
        if (b.destroyed) {
            if (b.size > 0)
                b.size -= 0.05f;
        }
    }

    for (auto& a : asteroids) {
        if (a.destroyed)
            continue;

        for (auto& b : bullets) {
            if (b.destroyed || !bullet_can_destroy(b))
                continue;

            if (intersect(spirits.asteroid.contour, spirits.bullet.contour, a.pos, b.pos)) {
                a.destroyed = true;
                b.destroyed = true;
                score += 5;
                break;
            }
        }
    }
}

bool WindowLogic::update_scene() {
    size = target->GetSize();

    background_timer.update();
    new_asteroid_timer.update();
    new_bullet_timer.update();
    move_timer.update();
    penalty_timer.update();

    update_motion();
    new_asteroids();
    new_bullets();

    compute_penalty();

    if (State == GAME_PLAY)
        if (is_there_collision()) {
            State = GAME_OVER;
        }

    destroy_asteroids();
    collect_garbage();

    return true;
}

bool WindowLogic::paint() {
    // Proper drawing.
    target->BeginDraw();
    target->Clear(D2D1::ColorF(0.f, 0.f, 0.f));

    if (State == GAME_PLAY || State == GAME_OVER || State == FADE_IN) {
        f32 gameplay_opacity;

        if (State == GAME_OVER)
            gameplay_opacity = 1.f - game_over_progress;
        else if (State == FADE_IN)
            gameplay_opacity = fade_in_progress;
        else /* GAME_PLAY */
            gameplay_opacity = 1.f;

        auto background_rect = D2D1_RECT_F {
            .left = 0.,
            .top = 0.,
            .right = size.width,
            .bottom = size.height,
        };

        // Paint background.
        if (paint_blue > 0.f) {
            blue_background_brush->SetOpacity(paint_blue * gameplay_opacity);
            target->FillRectangle(background_rect, blue_background_brush.Get());
        } else {
            red_background_brush->SetOpacity(penalty * gameplay_opacity);
            target->FillRectangle(background_rect, red_background_brush.Get());
        }

        paint_controller();
        paint_asteroids();
        paint_bullets();

        if (State == FADE_IN) {
            text_helper.DrawChosenLevel(chosen_next_difficulty, 1.f - fade_in_progress);
        }

        if (State == GAME_OVER) {
            if (!text_helper.DrawGameOver(game_over_progress)) {
                std::wcout << L"Failed to draw game_over\n";
                return false;
            }
        }

        if (!text_helper.DrawData(score, difficulty)) {
            std::wcout << L"Failed to draw score\n";
            return false;
        }

        if (penalty > 0.f) {
            if (!text_helper.DrawPenalty(penalty * gameplay_opacity, penalty_points_total)) {
                std::wcout << L"Failed to draw penalty text\n";
                return false;
            }
        }

        if (game_over_progress >= 1.f) {
            State = FADE_OUT;

            if (!fade_out_timer.Init(FADE_OUT_INTERVAL)) {
                std::wcout << L"Cannot init fade-out timer\n";
                return false;
            }
        }

        if (fade_in_progress >= 1.f) {
            State = GAME_PLAY;
            fade_in_progress = 0.f;
        }
    }
    else if (State == CHOOSE_NEW_LEVEL) {
        paint_asteroids();
        paint_bullets();

        text_helper.DrawData(score, difficulty);
        typewriter_timer.update();

        if (typewriter_timer.get_intervals_count(true)) {
            typewriter_animation.next_frame();
        }

        auto [choose_next_txt, choose_next_len] = typewriter_animation.get_text();
        text_helper.DrawNextTxt(choose_next_txt, choose_next_len);
        text_helper.DrawChosenLevel(chosen_next_difficulty, 1.f);

        if (!typewriter_animation.is_frame_left()) {
            if (chosen_next_difficulty != -1) {
                State = FADE_IN;
                reset_controller_pos();
                asteroids.clear();
                bullets.clear();
                game_over_progress = 0.f;
                fade_in_progress = 0.f;
                controller_downspeed = 0.f;
                penalty_points_total = 0;
                score = 0;
                bullet_forbidden = false;
                difficulty = chosen_next_difficulty;

                if (!set_asteroid_frequency())
                    return false;
            }
        }

    } else if (State == FADE_OUT) {
        paint_asteroids();
        paint_bullets();

        text_helper.DrawGameOver(1.f);
        text_helper.DrawData(score, difficulty);

        fade_out_timer.update();

        if (fade_out_timer.get_intervals_count(false)) {
            State = CHOOSE_NEW_LEVEL;
            typewriter_animation.Init(L"CHOOSE NEXT LEVEL DIFFICULTY");
            typewriter_timer.Init(TYPE_SPEED);
            chosen_next_difficulty = -1;
        }

    }

    HRESULT hr;
    hr = target->EndDraw();

    if (hr != S_OK) {
        std::wcout << L"Error while drawing\n";
        return false;
    }

    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    hr = dxgi_swapchain->Present(1, 0);
    if (hr != S_OK) {
        return false;
    }

    return true;
}

bool WindowLogic::on_resize() {
    return true;
}

bool WindowLogic::on_mousemove() {
    return true;
}

bool WindowLogic::on_keypress(u16 vkey) {
    if (State == CHOOSE_NEW_LEVEL) {
        if (vkey >= 0x31 && vkey <= 0x36)
            chosen_next_difficulty = vkey - 0x30;
    }

    return true;
}
