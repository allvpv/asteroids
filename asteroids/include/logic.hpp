#pragma once
#include <random>
#include <deque>

#include <windows.h>
#include <d2d1.h>
#include <wrl.h>
#include <wrl/client.h>

#include "common.hpp"
#include "timer.hpp"
#include "graphics.hpp"
#include "collision.hpp"

struct Window;

struct WindowLogic {
    bool Init();
    bool on_paint();
    bool on_resize();
    bool on_mousemove();

    WindowLogic(Window& window)
        : window(window)
        , norm_asteroid_x(0.5, 0.125) // Almost always (0, 1)
        , unif_asteroid_y(0., 1.)     // Always [0, 1)
        , unif_speed(1., 1.5)
        , gen(rd())
        , game_over(false) {}

private:
    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> CreateBackgroundGradient();

    void new_asteroids();
    void controller_move(const i32 ticks);

    void update_motion();
    void paint_asteroids();
    void paint_controller();

    bool is_there_collision();

#ifdef PAINT_CONTOUR_DBG
    void paint_contour_dbg(const ObjectContour& contour, const Vector& center);
#endif // PAINT_CONTOUR_DBG

    Microsoft::WRL::ComPtr<ID2D1Bitmap> rocket_bitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> asteroid_small_bitmap;

    Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> render_target;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> temp_asteroid_brush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> temp_controller_brush;
    Window& window;

    D2D1_COLOR_F reference_bg {
        .r = 0.10f - 0.075f,
        .g = 0.10f - 0.075f,
        .b = 0.20f - 0.075f,
        .a = 1.f,
    };

    D2D1_SIZE_F size;
    D2D1_COLOR_F side_bg;
    D2D1_COLOR_F middle_bg;

    bool game_over;

    Timer background_timer;
    Timer new_asteroid_timer;
    Timer move_asteroid_timer;

    f32 bound_left;
    f32 bound_right;

    struct Asteroid {
        Vector pos;
        f32 speed;
    };

    f32 accel_left = 0, accel_right = 0;
    f32 controller_downspeed = 0;
    Vector controller_pos;

    std::deque<Asteroid> asteroids;

    std::uniform_real_distribution<double> unif_speed;
    std::normal_distribution<double> norm_asteroid_x;
    std::uniform_real_distribution<double> unif_asteroid_y;

    std::random_device rd;
    std::mt19937 gen;
};

