#pragma once
#include <dwrite.h>
#include <d2d1_1.h>

#include "common.hpp"
#include "graphics.hpp"

struct TextWidget {
    TextWidget(ID2D1HwndRenderTarget& target, IDWriteFactory& write_factory)
        : write_factory(write_factory)
        , main_target(target) {}

    bool Init() {
        HRESULT hr;

        hr = write_factory.CreateTextFormat(
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

        hr = main_target.QueryInterface(__uuidof(D2D1DeviceContext), &device);

        if (hr != S_OK || !device)
            return false;

        hr = device->CreateEffect(CLSID_D2D1GaussianBlur, &blur_effect);

        if (hr != S_OK || !blur_effect)
            return false;

        hr = main_target.CreateCompatibleRenderTarget(&target);

        if (hr != S_OK || !target)
            return false;

        target->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.6f, 1.0f), &text_brush);
    }

    bool Draw() {
        HRESULT hr;

        target->BeginDraw();
        target->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

        static const WCHAR sc_helloWorld[] = L"hujj!";
        auto size = target->GetSize();

        target->DrawText(
            L"hujj",
            ARRAYSIZE(sc_helloWorld) - 1,
            m_pTextFormat,
            D2D1::RectF(0, 0, size.width, size.height),
            text_brush
        );

        bitmapRenderTarget->EndDraw();

        ComPtr<ID2D1Bitmap> bitmap;
        hr = main_target->GetBitmap(&bitmap);

        if (hr != S_OK)
            return false;

        main_target.DrawBitmap(bitmap);

        return true;
    }

private:
    ComPtr<IDWriteTextFormat> scorewidget_format;
    ComPtr<ID2D1DeviceContext> device;
    ComPtr<ID2D1Effect> blur_effect;
    ComPtr<ID2D1BitmapRenderTarget> target;
    ComPtr<ID2D1SolidColorBrush> text_brush;

    IDWriteFactory& write_factory;
    ID2D1HwndRenderTarget& main_target;
};
