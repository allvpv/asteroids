#include <iostream>

#include "window.hpp"
#include "logic.hpp"
#include "common.hpp"
#include "timer.hpp"

bool Window::Init(const wchar_t* class_name, const wchar_t* title) {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    if (!hInstance) {
        std::wcout << L"Failed to get module handle";
        return false;
    }

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;

    if (!RegisterClass(&wc)) {
        std::wcout << L"Failed to register class";
        return false;
    }

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,          // Optional window styles.
        class_name, // Window class
        title,      // Window title
        WS_OVERLAPPEDWINDOW, // Window style
        CW_USEDEFAULT,  // X
        CW_USEDEFAULT,  // Y
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        this        // Additional application data
    );


    if (!hwnd || handle != hwnd) {
        std::wcout << L"Failed to create window";
        return false;
    }

    u32 dpi = GetDpiForWindow(handle);
    i32 outer_width, outer_height;

    ComputeOuterSize(outer_width, outer_height, dpi);
    SetOuterSize(outer_width, outer_height, dpi);

    return timer.Init() &&
           logic.Init();
}

bool Window::ComputeOuterSize(i32 &outer_width, i32 &outer_height, u32 dpi) {
    auto client_rect = RECT {
        .left = 0,
        .top = 0,
        .right = (i32) inner_width,
        .bottom = (i32) inner_height,
    };

    bool result = AdjustWindowRectEx(
        &client_rect,
        WS_OVERLAPPEDWINDOW,
        false,
        NULL
    );

    if (!result) {
        std::wcout << L"Cannot get size of window for (" << inner_width << L", " << inner_height
                   << L") inner area dimensions\n";
        return false;
    }

    // Adjusted size with outer border taken into account (title bar etc.)
    outer_width = client_rect.right - client_rect.left;
    outer_height = client_rect.bottom - client_rect.top;

    return true;
}

bool Window::SetOuterSize(i32 outer_width, i32 outer_height, u32 dpi) {
    i32 physical_width = PHYSICAL_PIXELS(outer_width, dpi);
    i32 physical_height = PHYSICAL_PIXELS(outer_height, dpi);

    return SetWindowPos(
        handle,
        NULL,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        physical_width,
        physical_height,
        SWP_NOMOVE
    );
}

void Window::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    timer.update();

    switch(uMsg) {
        case WM_PAINT: {
            if (!logic.on_paint())
                PostQuitMessage(0);
            break;
        }

        case WM_SIZE: {
            if (!logic.on_resize())
                PostQuitMessage(0);
            break;
        }

        case WM_MOUSEMOVE: {
            if (!logic.on_mousemove())
                PostQuitMessage(0);
            break;
        }

        case WM_DESTROY: {
            /* Post quit message with status 0 to main process loop */
            PostQuitMessage(0);
            break;
        }
    }
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_NCCREATE: {
            auto create_data = (const CREATESTRUCT*) lParam;
            auto self = (Window*) create_data->lpCreateParams;

            if (!self)
                break;

            self->handle = hwnd;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) self);

            break;
        }

        default: {
            auto self = (Window*) GetWindowLongPtrA(hwnd, GWLP_USERDATA);

            if (!self)
                break;

            self->MessageHandler(uMsg, wParam, lParam);

            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
