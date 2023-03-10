#pragma once
#include <windows.h>
#include "common.hpp"
#include "logic.hpp"

struct Window {
    Window(u32 inner_width, u32 inner_height)
        : inner_width(inner_width)
        , inner_height(inner_height)
        , logic(*this) {}

    bool Init(const wchar_t* class_name, const wchar_t* title);
    bool ComputeOuterSize(i32 &outer_width, i32 &outer_height);
    bool SetOuterSize(i32 outer_width, i32 outer_height, u32 dpi);
    void MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND get_handle() { return handle; }
    i32 get_inner_width() { return inner_width; }
    i32 get_inner_height() { return inner_height; }

    bool update();

private:
    WindowLogic logic;
    HWND handle;
    i32 inner_width;
    i32 inner_height;
};
