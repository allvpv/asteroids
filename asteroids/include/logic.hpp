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
#include "math.hpp"

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
    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> create_background_gradient();

    void new_asteroids();
    void new_bullets();

    void asteroids_move(const i32 ticks);
    void controller_move(const i32 ticks);
    void bullets_move(const i32 ticks);

    void update_motion();

    void paint_asteroids();
    void paint_controller();
    void paint_bullets();

    void collect_garbage();
    void destroy_asteroids();

    bool is_there_collision();

#ifdef PAINT_CONTOUR_DBG
    void paint_contour_dbg(const ObjectContour& contour, const Vector& center);
#endif // PAINT_CONTOUR_DBG

    Microsoft::WRL::ComPtr<ID2D1Bitmap> controller_bitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> asteroid_bitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> bullet_bitmap;

    Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> render_target;

#ifdef PAINT_CONTOUR_DBG
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> contour_brush;
#endif // PAINT_CONTOUR_DBG
    Window& window;

    D2D1_SIZE_F size;

    bool game_over = false;
    bool bullet_forbidden = false;

    Timer background_timer;
    Timer new_asteroid_timer;
    Timer move_timer;
    Timer new_bullet_timer;

    f32 bound_left;
    f32 bound_right;

    struct Asteroid {
        Vector pos;
        f32 speed;
        bool destroyed;
        f32 size;
    };

    struct Bullet {
        Vector pos;
        bool destroyed;
        f32 size;
    };

    f32 accel_left = 0, accel_right = 0;
    f32 controller_downspeed = 0;
    Vector controller_pos;

    std::deque<Asteroid> asteroids;
    std::deque<Bullet> bullets;


    std::uniform_real_distribution<double> unif_speed;
    std::normal_distribution<double> norm_asteroid_x;
    std::uniform_real_distribution<double> unif_asteroid_y;

    std::random_device rd;
    std::mt19937 gen;
};

