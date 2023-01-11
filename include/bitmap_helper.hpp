#pragma once
#include <utility>
#include <windows.h>
#include <wincodec.h>
#include <d2d1.h>
#include <d2d1_1.h>

#include "common.hpp"

std::pair<HRESULT, ComPtr<ID2D1Bitmap>> load_bitmap_from_file(ID2D1DeviceContext& target,
                                                              IWICImagingFactory2& imaging_factory,
                                                              const wchar_t* uri);
