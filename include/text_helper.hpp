#pragma once
#include "common.hpp"

#include <dwrite.h>
#include <d2d1.h>
#include <d2d1_1.h>


struct TextTypewriterAnimation {
    bool Init(const wchar_t* animation_text_);
    bool is_frame_left();
    void next_frame();

    std::pair<const wchar_t*, size_t> get_text();

private:
    const wchar_t* animation_text;
    size_t chars_progress;
};

struct TextHelper {
    TextHelper() {}

    bool Init(ComPtr<ID2D1DeviceContext> main_target_);
    bool DrawNextTxt(const wchar_t* text, size_t len);
    bool DrawChosenLevel(i32 level, f32 opacity);
    bool DrawGameOver(f32 opacity);
    bool DrawPenalty(f32 opacity, i32 penalty);
    bool DrawData(i32 score, u32 difficulty);

    bool Flush();
    void Start();

    bool data_included = false;

    bool penalty_included = false;
    f32 penalty_opacity;

    bool gameover_included = false;
    f32 gameover_opacity;

private:
    bool PrerenderData();
    bool PrerenderPenalty();
    bool PrerenderGameOver();

    D2D1_RECT_F rect_score_text;
    D2D1_RECT_F rect_difficulty_text;

    ComPtr<ID2D1Bitmap> data_prerendered;
    ComPtr<ID2D1Bitmap> penalty_prerendered;
    ComPtr<ID2D1Bitmap> gameover_prerendered;

    ComPtr<ID2D1DeviceContext> main_target;
    ComPtr<ID2D1BitmapRenderTarget> target;

    ComPtr<IDWriteFactory> write_factory;
    ComPtr<ID2D1Effect> blur_effect;

    ComPtr<IDWriteTextFormat> regular_format;
    ComPtr<IDWriteTextFormat> bold_format;
    ComPtr<IDWriteTextFormat> game_over_format;
    ComPtr<IDWriteTextFormat> choose_next_format;
    ComPtr<IDWriteTextFormat> next_format;

    ComPtr<ID2D1SolidColorBrush> text_pink_brush;
    ComPtr<ID2D1SolidColorBrush> text_green_brush;
    ComPtr<ID2D1SolidColorBrush> text_white_brush;
    ComPtr<ID2D1SolidColorBrush> text_yellow_brush;
};
