#pragma once
#include <utility>
#include <windows.h>
#include <wincodec.h>
#include <d2d1.h>
#include <wrl.h>
#include <wrl/client.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

std::pair<HRESULT, ComPtr<ID2D1Bitmap>> load_bitmap_from_file(ID2D1HwndRenderTarget& render_target,
                                                              IWICImagingFactory2& imaging_factory,
                                                              const wchar_t* uri);
