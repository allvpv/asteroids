#pragma once
#include <dwrite.h>
#include <d2d1.h>
#include <d2d1_1.h>

#include "common.hpp"
#include "graphics.hpp"

struct TextHelper {
    TextHelper() {}

    bool Init(ComPtr<ID2D1DeviceContext> main_target_) {
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
            L"Helvetica",
            NULL,
            DWRITE_FONT_WEIGHT_MEDIUM,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16.f,
            L"",
            &scorewidget_format
        );

        if (hr != S_OK || !scorewidget_format)
            return false;

        scorewidget_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        scorewidget_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        hr = main_target->CreateEffect(CLSID_D2D1GaussianBlur, &blur_effect);

        if (hr != S_OK || !blur_effect)
            return false;

        hr = main_target->CreateCompatibleRenderTarget(&target);

        if (hr != S_OK || !target)
            return false;

        hr = target->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.6f, 1.0f), &text_brush);

        if (hr != S_OK || !text_brush)
            return false;

        return true;
    }

    bool Draw() {
        HRESULT hr;

        target->BeginDraw();

        target->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

        static const WCHAR sc_helloWorld[] = L"hujj!";
        D2D1_SIZE_F size = target->GetSize();

        auto rect = D2D1::RectF(0, 0, size.width, size.height);

        target->DrawText(
            sc_helloWorld,
            ARRAYSIZE(sc_helloWorld) - 1,
            scorewidget_format.Get(),
            rect,
            text_brush.Get(),
            D2D1_DRAW_TEXT_OPTIONS_NONE,
            DWRITE_MEASURING_MODE_NATURAL
        );

        hr = target->EndDraw();

        if (hr != S_OK)
            return false;

        ComPtr<ID2D1Bitmap> bitmap;
        hr = target->GetBitmap(&bitmap);

        if (hr != S_OK || !bitmap)
           return false;

        main_target->DrawBitmap(bitmap.Get());

        return true;
    }

private:
    ComPtr<ID2D1DeviceContext> main_target;
    ComPtr<ID2D1BitmapRenderTarget> target;

    ComPtr<IDWriteFactory> write_factory;
    ComPtr<IDWriteTextFormat> scorewidget_format;
    ComPtr<ID2D1Effect> blur_effect;
    ComPtr<ID2D1SolidColorBrush> text_brush;
};
