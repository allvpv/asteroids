#include <stdio.h>
#include <windows.h>
#include "window.hpp"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int nCmdShow) {
#ifndef NDEBUG
    AllocConsole();

    FILE* new_stream;
    if (freopen_s(&new_stream, "CONIN$", "r", stdin) != 0 || new_stream != stdin) {
        std::cout << "Cannot attach stdin to console";
        return -1;
    }

    if (freopen_s(&new_stream, "CONOUT$", "w", stdout) != 0 || new_stream != stdout) {
        std::cout << "Cannot attach stdout to console";
        return -1;
    }

    if (freopen_s(&new_stream, "CONOUT$", "w", stderr) != 0 || new_stream != stderr) {
        std::cout << "Cannot attach stderr to console";
        return -1;
    }
#endif

    Window window(640, 480);
    bool result = window.Init(L"Asteroids Class", L"Asteroids!");

    if (!result) {
        while (true);
        return -1;
    }

    ShowWindow(window.get_handle(), nCmdShow);

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    std::cout << "UU\n";
    while (true);
}

