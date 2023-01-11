#include "text_helper.hpp"

#include <iostream>
#include <sstream>

bool TextTypewriterAnimation::Init(const wchar_t* animation_text_) {
    animation_text = animation_text_;
    chars_progress = 0;

    return true;
}

bool TextTypewriterAnimation::is_frame_left() {
    return animation_text[chars_progress] != L'\0';
}

void TextTypewriterAnimation::next_frame() {
    if (is_frame_left())
        ++chars_progress;
}

std::pair<const wchar_t*, size_t> TextTypewriterAnimation::get_text() {
    return { animation_text, chars_progress };
}

bool TextHelper::Init(ComPtr<ID2D1DeviceContext> main_target_) {
    main_target = main_target_;

    HRESULT hr;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        &write_factory
    );

    if (hr != S_OK || !write_factory)
        return false;

    hr = write_factory->CreateTextFormat(
        L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 25.f, L"", &regular_format
    );

    if (hr != S_OK || !regular_format)
        return false;

    hr = write_factory->CreateTextFormat(
        L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 25.f, L"", &bold_format
    );

    if (hr != S_OK || !bold_format)
        return false;

    hr = write_factory->CreateTextFormat(
        L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 50.f, L"", &game_over_format
    );

    if (hr != S_OK || !game_over_format)
        return false;

    hr = write_factory->CreateTextFormat(
        L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 30.f, L"", &choose_next_format
    );

    if (hr != S_OK || !choose_next_format)
        return false;


    hr = write_factory->CreateTextFormat(
        L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 70.f, L"", &next_format
    );

    if (hr != S_OK || !next_format)
        return false;

    regular_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    regular_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    bold_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    bold_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    game_over_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    game_over_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    choose_next_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    choose_next_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    next_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    next_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    hr = main_target->CreateEffect(CLSID_D2D1GaussianBlur, &blur_effect);

    if (hr != S_OK || !blur_effect)
        return false;

    hr = main_target->CreateCompatibleRenderTarget(&target);

    if (hr != S_OK || !target)
        return false;

    hr = target->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.6f, 0.6f), &text_pink_brush);

    if (hr != S_OK || !text_pink_brush)
        return false;

    hr = target->CreateSolidColorBrush(D2D1::ColorF(0.6f, 1.0f, 0.6f), &text_green_brush);

    if (hr != S_OK || !text_green_brush)
        return false;

    hr = target->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), &text_white_brush);

    if (hr != S_OK || !text_white_brush)
        return false;

    hr = target->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0.0f), &text_yellow_brush);

    if (hr != S_OK || !text_yellow_brush)
        return false;

    return true;
}

bool TextHelper::DrawNextTxt(const wchar_t* text, size_t len) {
    HRESULT hr;

    auto size = target->GetSize();
    auto rect = D2D1::RectF(size.width / 2. - 235.,
                            size.height / 2. - 100,
                            size.width,
                            size.height);

    target->BeginDraw();
    target->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

    text_yellow_brush->SetOpacity(1.f);

    std::wcout << text << L", " << len << L'\n';

    target->DrawText(text, len, choose_next_format.Get(), rect, text_yellow_brush.Get(),
                     D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);

    hr = target->EndDraw();

    if (hr != S_OK)
        return false;

    return DrawTargetWithGlow();
}

bool TextHelper::DrawChosenLevel(i32 level, f32 opacity) {
    HRESULT hr;

    auto size = target->GetSize();
    auto rect = D2D1::RectF(0,
                            size.height / 2. - 50.,
                            size.width / 2. + 50.,
                            size.height);

    std::wstring chosen_level = [level]() {
        std::wostringstream oss;
        if (level == -1)
            oss << "_";
        else
            oss << level;

        oss << " / 6";
        return oss.str();
    }();

    target->BeginDraw();
    target->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

    text_green_brush->SetOpacity(opacity);

    target->DrawText(chosen_level.data(), chosen_level.length(), next_format.Get(), rect,
                     text_green_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    hr = target->EndDraw();

    if (hr != S_OK)
        return false;

    return DrawTargetWithGlow();
}

bool TextHelper::DrawGameOver(f32 opacity) {
    HRESULT hr;

    static const WCHAR game_over[] = L"GAME OVER";

    auto size = target->GetSize();
    auto rect = D2D1::RectF(0, 0, size.width, size.height);

    target->BeginDraw();
    target->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

    text_yellow_brush->SetOpacity(opacity);

    target->DrawText(game_over, ARRAYSIZE(game_over) - 1, game_over_format.Get(),
                     rect, text_yellow_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    hr = target->EndDraw();

    if (hr != S_OK)
        return false;

    return DrawTargetWithGlow();
}

bool TextHelper::DrawPenalty(f32 opacity, i32 penalty) {
    HRESULT hr;

    target->BeginDraw();
    target->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

    static const WCHAR save_zone[] = L"Move to safe zone!";
    static const WCHAR lose_points[] = L"Penalty: ";

    D2D1_SIZE_F size = target->GetSize();

    auto rect_save_zone = D2D1::RectF(80, 50, size.width, 70);
    auto rect_lose_points = D2D1::RectF(80, 75, size.width, 95);
    auto rect_penalty_points = D2D1::RectF(80, 100, size.width, 120);

    text_white_brush->SetOpacity(opacity);

    target->DrawText(save_zone, ARRAYSIZE(save_zone) - 1, regular_format.Get(),
                     rect_save_zone, text_white_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    target->DrawText(lose_points, ARRAYSIZE(lose_points) - 1, regular_format.Get(),
                     rect_lose_points, text_white_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    std::wstring penalty_points = [penalty]() {
        std::wostringstream oss;
        oss << -penalty;
        return oss.str();
    }();

    target->DrawText(penalty_points.data(), penalty_points.length(), bold_format.Get(),
                     rect_penalty_points, text_white_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    hr = target->EndDraw();

    if (hr != S_OK)
        return false;

    return DrawTargetWithGlow();
}

bool TextHelper::DrawData(i32 score, u32 difficulty) {
    HRESULT hr;


    static const WCHAR sc_helloWorld[] = L"Score ";
    static const WCHAR sc_difficulty[] = L"Difficulty ";
    D2D1_SIZE_F size = target->GetSize();

    auto rect_score_text = D2D1::RectF(size.width - 240, 50, size.width - 135, 70);
    auto rect_difficulty_text = D2D1::RectF(size.width - 240, 80, size.width - 135, 100);

    auto rect_score_points = D2D1_RECT_F {
        .left = rect_score_text.right + 20,
        .top = rect_score_text.top,
        .right = size.width,
        .bottom = rect_score_text.bottom,
    };

    auto rect_difficulty_points = D2D1_RECT_F {
        .left = rect_difficulty_text.right + 20,
        .top = rect_difficulty_text.top,
        .right = size.width,
        .bottom = rect_difficulty_text.bottom,
    };

    target->BeginDraw();
    target->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

    text_green_brush->SetOpacity(1.f);
    text_pink_brush->SetOpacity(1.f);

    regular_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

    target->DrawText(sc_helloWorld, ARRAYSIZE(sc_helloWorld) - 1, regular_format.Get(),
                     rect_score_text, text_pink_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    target->DrawText(sc_difficulty, ARRAYSIZE(sc_difficulty) - 1, regular_format.Get(),
                     rect_difficulty_text, text_pink_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    regular_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

    std::wstring score_txt = [score]() {
        std::wostringstream oss;
        oss << score;
        return oss.str();
    }();

    std::wstring diff_txt = [difficulty]() {
        std::wostringstream oss;
        oss << difficulty << " / 6";
        return oss.str();
    }();

    target->DrawText(score_txt.data(), score_txt.length(), bold_format.Get(),
                     rect_score_points, text_green_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE,
                     DWRITE_MEASURING_MODE_NATURAL);

    target->DrawText(diff_txt.data(), diff_txt.length(), bold_format.Get(),
                     rect_difficulty_points, text_green_brush.Get(),
                     D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);

    hr = target->EndDraw();

    if (hr != S_OK)
        return false;

    return DrawTargetWithGlow();
}

bool TextHelper::DrawTargetWithGlow() {
    HRESULT hr;

    ComPtr<ID2D1Bitmap> bitmap;
    hr = target->GetBitmap(&bitmap);

    if (hr != S_OK || !bitmap)
       return false;

    main_target->DrawBitmap(bitmap.Get());

    blur_effect->SetInput(0, bitmap.Get());
    blur_effect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_SOFT);
    blur_effect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 6.0f);

    main_target->DrawImage(
        blur_effect.Get(),
        D2D1_INTERPOLATION_MODE_LINEAR
    );

    blur_effect->SetInput(0, bitmap.Get());
    blur_effect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_SOFT);
    blur_effect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 10.0f);

    main_target->DrawImage(
        blur_effect.Get(),
        D2D1_INTERPOLATION_MODE_LINEAR
    );

    return true;
}
