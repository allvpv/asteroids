#pragma once
#include <random>
#include <deque>

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
    void update_asteroids();
    void paint_asteroids();

    WindowLogic(Window& window)
        : window(window)
        , norm_asteroid_x(0.5, 0.125) // For the most part (0, 1)
        , unif_asteroid_y(0., 1.)     // Always [0, 1)
        , gen(rd()) {}

private:
    Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> render_target;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> temp_brush;
    Window& window;

    Timer background_timer;
    Timer new_asteroid_timer;

    struct Asteroid {
        f32 x, y;
    };

    std::deque<Asteroid> asteroids;

    std::normal_distribution<double> norm_asteroid_x;
    std::uniform_real_distribution<double> unif_asteroid_y;

    std::random_device rd;
    std::mt19937 gen;
};
