#pragma once
#include <random>
#include <deque>

#include <windows.h>
#include <d2d1.h>
#include <d2d1_2.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include "common.hpp"
#include "timer.hpp"
#include "math.hpp"
#include "spirits_gen.hpp"
#include "text_helper.hpp"

struct Window;

struct WindowLogic {
    bool Init();

    bool set_asteroid_frequency();

    void reset_controller_pos();
    bool update_scene();
    bool paint();

    bool on_resize();
    bool on_mousemove();
    bool on_keypress(u16 vkey);

    WindowLogic(Window& window)
        : window(window)
        , norm_asteroid_x(0.5f, 0.125f) // Almost always (0, 1)
        , unif_asteroid_y(0.f, 1.f)     // Always [0, 1)
        , unif_speed(1.f, 1.5f)
        , gen(rd())
        , State(GAME_PLAY) {}

private:
    void compute_penalty();

    void new_asteroids();
    void new_bullets();

    void asteroids_move(const f32 shift);
    void controller_move(const f32 shift);
    void bullets_move(const f32 shift);
    void game_over_move(const f32 shift);

    void update_motion();

    void paint_asteroids();
    void paint_controller();
    void paint_bullets();

    void collect_garbage();
    void destroy_asteroids();

    bool is_there_collision();

    ComPtr<ID2D1Bitmap> create_gradient(D2D1_COLOR_F side_bg, D2D1_COLOR_F middle_bg);

#ifdef PAINT_CONTOUR_DBG
    void paint_contour_dbg(const ObjectContour& contour, const Vector& center);
#endif // PAINT_CONTOUR_DBG

    ComPtr<ID2D1Bitmap> controller_bitmap;
    ComPtr<ID2D1Bitmap> asteroid_bitmap;
    ComPtr<ID2D1Bitmap> bullet_bitmap;

    ComPtr<ID2D1Factory1> d2d_factory;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGIDevice1> dxgi_device;
    ComPtr<ID2D1DeviceContext> target;

    ComPtr<IDXGIAdapter> dxgi_adapter;
    ComPtr<IDXGIFactory2> dxgi_factory;
    ComPtr<IDXGISwapChain1> dxgi_swapchain;
    ComPtr<ID3D11Texture2D> backbuffer;
    ComPtr<IDXGISurface> dxgi_backbuffer;
    ComPtr<ID2D1Bitmap1> target_bitmap;

    ComPtr<ID2D1Device> d2d_device;
    ComPtr<ID2D1DeviceContext> d2d_context;

    ComPtr<ID2D1Bitmap> red_background_bitmap;
    ComPtr<ID2D1Bitmap> blue_background_bitmap;

#ifdef PAINT_CONTOUR_DBG
    ComPtr<ID2D1SolidColorBrush> contour_brush;
#endif // PAINT_CONTOUR_DBG
    Window& window;

    TextHelper text_helper;

    D2D1_SIZE_F size;

    enum {
        FADE_IN,
        GAME_PLAY,
        GAME_OVER,
        FADE_OUT,
        CHOOSE_NEW_LEVEL,
    } State;

    //
    // GAME_PLAY, GAME_OVER
    //
    Spirits spirits;
    u32 difficulty = 1;
    bool bullet_forbidden = false;

    f32 dpi_factor;

    Timer background_timer;
    Timer new_asteroid_timer;
    Timer move_timer;
    Timer new_bullet_timer;
    Timer penalty_timer;

    f32 bound_left;
    f32 bound_right;

    f32 penalty;
    f32 paint_blue;

    i32 penalty_points_total;
    i32 score = 0;

    D2D1_SIZE_F controller_bmp_size;
    D2D1_SIZE_F asteroid_bmp_size;
    D2D1_SIZE_F bullet_bmp_size;

    struct Asteroid {
        Vector pos;
        f32 speed;
        bool destroyed;
        f32 size;
    };

    bool asteroid_visible(const Asteroid& asteroid) {
        return asteroid.pos.y <= size.height + spirits.asteroid.contour.half_of_sides.y;
    }

    struct Bullet {
        Vector pos;
        bool destroyed;
        f32 size;
    };

    bool bullet_visible(const Bullet& bullet) {
        return bullet.pos.y + spirits.bullet.contour.half_of_sides.y >= 0;
    }

    bool bullet_can_destroy(const Bullet& bullet) {
        return bullet.pos.y > 0;
    }

    f32 accel_left = 0, accel_right = 0;
    f32 controller_downspeed = 0;
    Vector controller_pos;

    std::deque<Asteroid> asteroids;
    std::deque<Bullet> bullets;

    std::uniform_real_distribution<float> unif_speed;
    std::normal_distribution<float> norm_asteroid_x;
    std::uniform_real_distribution<float> unif_asteroid_y;

    std::random_device rd;
    std::mt19937 gen;

    f32 game_over_progress = 0;

    //
    // FADE_OUT
    //
    Timer fade_out_timer;

    //
    // CHOOSE_NEW_LEVEL
    //
    Timer typewriter_timer;
    TextTypewriterAnimation typewriter_animation;
    inline static const wchar_t choose_difficulty[] = L"Choose new level difficulty:";
    i32 chosen_next_difficulty = -1;

    //
    // FADE_IN
    //
    f32 fade_in_progress;
};
