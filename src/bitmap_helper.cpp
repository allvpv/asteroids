#include "bitmap_helper.hpp"

#include <iostream>
#include <utility>

std::pair<HRESULT, ComPtr<ID2D1Bitmap>> load_bitmap_from_file(ID2D1DeviceContext& target,
                                                              IWICImagingFactory2& imaging_factory,
                                                              const wchar_t* uri) {
    HRESULT hr;
    ComPtr<IWICBitmapDecoder> decoder;
    ComPtr<IWICBitmapFrameDecode> frame_decoder;
    ComPtr<IWICFormatConverter> converter;
    ComPtr<ID2D1Bitmap> bitmap;

    hr = imaging_factory.CreateDecoderFromFilename(
        uri,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    if (hr != S_OK || !decoder)
        return { hr, {} };

    hr = decoder->GetFrame(0, &frame_decoder);

    if (hr != S_OK || !frame_decoder)
        return { hr, {} };

    hr = imaging_factory.CreateFormatConverter(&converter);

    if (hr != S_OK || !converter)
        return { hr, {} };

    hr = converter->Initialize(
        frame_decoder.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.f,
        WICBitmapPaletteTypeMedianCut
    );

    if (hr != S_OK)
        return { hr, {} };

    hr = target.CreateBitmapFromWicBitmap(converter.Get(), NULL, &bitmap);

    if (hr != S_OK)
        return { hr, {} };
    else
        return { hr, bitmap };
}
