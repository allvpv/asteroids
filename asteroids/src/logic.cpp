#include <iostream>
#include <d2d1_2.h>
#include <comdef.h>

#include "logic.hpp"
#include "window.hpp"

bool WindowLogic::Init(Window &window) {
    HRESULT hr;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (hr != S_OK) {
        std::wcout << L"Failed to initialize the COM library: " << _com_error(hr).ErrorMessage()
                   << '\n';
        return false;
    }

    D2D1_FACTORY_OPTIONS options = {};

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
                           __uuidof(ID2D1Factory2),
                           &options,
                           &d2d_factory);
    if (hr != S_OK) {
        std::wcout << L"Failed to initialize ID2D1Factory: " << _com_error(hr).ErrorMessage()
                   << '\n';
        return false;
    }

    return true;
}

bool WindowLogic::on_paint() {
    return true;
}

bool WindowLogic::on_resize() {
    return true;
}

bool WindowLogic::on_mousemove() {
    return true;
}
