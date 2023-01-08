#include <iostream>
#include <stdio.h>
#include <windows.h>
#include "window.hpp"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int nCmdShow) {
#ifndef NDEBUG
    AllocConsole();

    FILE* new_stream;
    if (freopen_s(&new_stream, "CONIN$", "r", stdin) != 0 || new_stream != stdin) {
        std::wcout << L"Cannot attach stdin to console";
        return -1;
    }

    if (freopen_s(&new_stream, "CONOUT$", "w", stdout) != 0 || new_stream != stdout) {
        std::wcout << L"Cannot attach stdout to console";
        return -1;
    }

    if (freopen_s(&new_stream, "CONOUT$", "w", stderr) != 0 || new_stream != stderr) {
        std::wcout << L"Cannot attach stderr to console";
        return -1;
    }
#endif

    Window window(640, 480);
    bool result = window.Init(L"Asteroids Class", L"Asteroids!");

    if (!result) {
        std::wcout << L"Cannot initialize window";
        return -1;
    }

    ShowWindow(window.get_handle(), nCmdShow);

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

