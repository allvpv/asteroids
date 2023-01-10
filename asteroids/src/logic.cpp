#include <iostream>
#include <utility>

#include <d2d1_2.h>
#include <comdef.h>

#include "common.hpp"
#include "logic.hpp"
#include "window.hpp"
#include "math.hpp"
#include "spirits.hpp"

namespace {
    constexpr i32 MOVE_INTERVAL = 0'005;
    constexpr i32 BACKGROUND_INTERVAL = 4'000;
    constexpr i32 ASTEROID_INTERVAL = 0'600;
    constexpr i32 NEW_BULLET_INTERVAL = 0'200;
    constexpr f32 BULLET_SPEED = 3;

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

    controller_pos.x = size.width / 2;
    controller_pos.y = size.height - 60;

    hr = d2d_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(window.get_handle(), size),
        &render_target);

    if (hr != S_OK || !render_target) {
        ErrorCollection::render_target_crash(hr);
        return false;
    }

#ifdef PAINT_CONTOUR_DBG
    hr = render_target->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 0.f), &contour_brush);

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
        auto result = load_bitmap_from_file(*render_target.Get(), *imaging_factory.Get(), fname);

        if (result.first != S_OK || !result.second) {
            ErrorCollection::bitmap_crash(fname, result.first);
            return false;
        } else {
            bitmap = result.second;
        }

        return true;
    };

    if (!load_bitmap(controller_data.filename, controller_bitmap) ||
        !load_bitmap(asteroid_data.filename, asteroid_bitmap) ||
        !load_bitmap(bullet_data.filename, bullet_bitmap)) {
        return false;
    }

    return background_timer.Init(BACKGROUND_INTERVAL) &&
           new_asteroid_timer.Init(ASTEROID_INTERVAL) &&
           move_timer.Init(MOVE_INTERVAL) &&
           new_bullet_timer.Init(NEW_BULLET_INTERVAL);
}

void WindowLogic::new_bullets() {
    if (bullet_forbidden) {
        i32 ticks = new_bullet_timer.get_intervals_count(false);

        if (ticks) {
            bullet_forbidden = false;
        }
    }

    bool space_press = key_up(VK_SPACE);

    if (bullet_forbidden || !space_press || game_over)
        return;

    bullet_forbidden = true;
    new_bullet_timer.start_new_interval();

    Vector bullet_pos { controller_pos };

    bullet_pos.y -= controller_data.contour.half_of_sides.y;

    bullets.push_front(Bullet {
        .pos = bullet_pos,
        .destroyed = false,
        .size = 1.f,
    });
}

void WindowLogic::new_asteroids() {
    const i32 ticks = new_asteroid_timer.get_intervals_count(true);

    if (ticks) {
        const f32 asteroid_radius = asteroid_data.contour.half_of_sides.y;

        const f64 shift_x = norm_asteroid_x(gen);
        const f64 shift_y = unif_asteroid_y(gen);

        const f32 x_pos = shift_x * size.width;
        const f32 y_pos = -(shift_y * (size.height - 2 * asteroid_radius) + asteroid_radius);

        const f32 speed = unif_speed(gen);

        asteroids.push_front(Asteroid {
            .pos = Vector(x_pos, y_pos),
            .speed = speed,
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

        if (intersect(asteroid_data.contour, controller_data.contour, a.pos, controller_pos)) {
            controller_downspeed = 3.f * a.speed;
            return true;
        }
    }

    return false;
}

void WindowLogic::controller_move(const i32 ticks) {
    if (game_over) {
        controller_pos.y += ticks * controller_downspeed;
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

    controller_pos.x += accel_right - accel_left;
}

void WindowLogic::asteroids_move(const i32 ticks) {
    for (auto& a : asteroids) {
        a.pos.y += ticks * a.speed;
    }
}

void WindowLogic::bullets_move(const i32 ticks) {
    for (auto& b : bullets) {
        b.pos.y -= ticks * BULLET_SPEED;
    }
}

void WindowLogic::update_motion() {
    const i32 ticks = move_timer.get_intervals_count(true);

    if (ticks) {
        asteroids_move(ticks);
        controller_move(ticks);
        bullets_move(ticks);
    }
}

void WindowLogic::paint_controller() {
    D2D1_SIZE_F bmp_size = controller_bitmap->GetSize();
    bmp_size.width /= 2;
    bmp_size.height /= 2;

    f32 hlfw_x = bmp_size.width / 2;
    f32 hlfw_y = bmp_size.height / 2;

    // Draw a bitmap.
    render_target->DrawBitmap(
        controller_bitmap.Get(),
        D2D1::RectF(
            controller_pos.x - hlfw_x,
            controller_pos.y - hlfw_y,
            controller_pos.x + hlfw_x,
            controller_pos.y + hlfw_y
        )
    );

#ifdef PAINT_CONTOUR_DBG
    paint_contour_dbg(controller_data.contour, controller_pos);
#endif // PAINT_CONTOUR_DBG
}

#ifdef PAINT_CONTOUR_DBG
void WindowLogic::paint_contour_dbg(const ObjectContour& contour, const Vector& center) {
    for (size_t i = 0; i < contour.vertices.size(); ++i) {
        const Vector& av = contour.vertices[i];
        const Vector& bv = contour.vertices[i + 1 < contour.vertices.size() ? i + 1 : 0];

        Vector suma = av + center;
        Vector sumb = bv + center;

        render_target->DrawLine(
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

        bool outside_view = last.pos.y > size.height + asteroid_data.contour.half_of_sides.y;
        bool very_small = last.size < 0.1f;

        if (outside_view || very_small)
            asteroids.pop_back();
        else
            break;
    }

    while (!bullets.empty()) {
        const Bullet& last = bullets.back();

        bool outside_view = last.pos.y + bullet_data.contour.half_of_sides.y < 0;
        bool very_small = last.size < 0.1f;

        if (outside_view || very_small)
            bullets.pop_back();
        else
            break;
    }

    std::wcout << L"GC: asters: " << asteroids.size()
               << L"bullets: " << bullets.size() << L'\n';
}

void WindowLogic::paint_asteroids() {
    D2D1_SIZE_F bmp_size = asteroid_bitmap->GetSize();
    bmp_size.width /= 10;
    bmp_size.height /= 10;

    f32 hlfw_x = bmp_size.width / 2;
    f32 hlfw_y = bmp_size.height / 2;

    for (auto& a : asteroids) {
        f32 this_hlfw_x = hlfw_x;
        f32 this_hlfw_y = hlfw_y;

        if (a.destroyed) {
            this_hlfw_x *= a.size;
            this_hlfw_y *= a.size;
        }

        render_target->DrawBitmap(
            asteroid_bitmap.Get(),
            D2D1::RectF(
                a.pos.x - this_hlfw_x,
                a.pos.y - this_hlfw_y,
                a.pos.x + this_hlfw_x,
                a.pos.y + this_hlfw_y
            )
        );

#ifdef PAINT_CONTOUR_DBG
        paint_contour_dbg(asteroid_data.contour, a.pos);
#endif // PAINT_CONTOUR_DBG
    }
}

void WindowLogic::paint_bullets() {
    D2D1_SIZE_F bmp_size = bullet_bitmap->GetSize();
    bmp_size.width /= 4;
    bmp_size.height /= 4;

    f32 hlfw_x = bmp_size.width / 2;
    f32 hlfw_y = bmp_size.height / 2;

    for (const auto& bullet : bullets) {
        f32 this_hlfw_x = hlfw_x;
        f32 this_hlfw_y = hlfw_y;

        if (bullet.destroyed) {
            this_hlfw_x *= bullet.size;
            this_hlfw_y *= bullet.size;
        }

        // Draw a bitmap.
        render_target->DrawBitmap(
            bullet_bitmap.Get(),
            D2D1::RectF(
                bullet.pos.x - this_hlfw_x,
                bullet.pos.y - this_hlfw_y,
                bullet.pos.x + this_hlfw_x,
                bullet.pos.y + this_hlfw_y
            )
        );

#ifdef PAINT_CONTOUR_DBG
        paint_contour_dbg(bullet_data.contour, bullet.pos);
#endif // PAINT_CONTOUR_DBG
    }
}

Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> WindowLogic::create_background_gradient() {
    auto side_bg = reference_bg;
    auto middle_bg = reference_bg;

    f32 penalty = max(bound_left - controller_pos.x, controller_pos.x - bound_right);

    if (penalty > 0)
        side_bg.r += (penalty / bound_left);
    else {
        f32 reward = controller_pos.x - size.width / 2;

        if (reward < 0)
            reward *= -1;

        f32 reward_bound = size.width / 5;

        if (reward < reward_bound)
            middle_bg.b += 0.3f * (1.f - reward / reward_bound);
    }

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
            if (b.destroyed)
                continue;

            if (intersect(asteroid_data.contour, bullet_data.contour, a.pos, b.pos)) {
                a.destroyed = true;
                b.destroyed = true;
                break;
            }
        }
    }
}

bool WindowLogic::on_paint() {
    size = render_target->GetSize();

    background_timer.update();
    new_asteroid_timer.update();
    new_bullet_timer.update();
    move_timer.update();

    update_motion();
    new_asteroids();
    new_bullets();

    if (game_over)
        std::wcout << L"Game Over!\n";

    auto bg_gradient = create_background_gradient();

    if (!bg_gradient) {
        std::wcout << L"Cannot create background gradient\n";
        return false;
    }

    if (!game_over)
        game_over = is_there_collision();

    destroy_asteroids();
    collect_garbage();

    // Proper drawing.
    render_target->BeginDraw();

    // Paint background.
    render_target->FillRectangle(D2D1_RECT_F {
        .left = 0.,
        .top = 0.,
        .right = size.width,
        .bottom = size.height,
    }, bg_gradient.Get());

    paint_controller();
    paint_asteroids();
    paint_bullets();

    render_target->EndDraw();

    return true;
}

bool WindowLogic::on_resize() {
    return true;
}

bool WindowLogic::on_mousemove() {
    return true;
}

