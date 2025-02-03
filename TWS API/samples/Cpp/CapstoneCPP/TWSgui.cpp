#define UNICODE
#include <windows.h>

#define ID_START_BUTTON  1
#define ID_FRONT_OPTION  2
#define ID_BACK_OPTION   3
#define ID_FRONT_DTE     4
#define ID_BACK_DTE      5
#define ID_ENTRY_HOUR    6
#define ID_ENTRY_MIN     7
#define ID_EXIT_HOUR     8
#define ID_EXIT_MIN      9
#define ID_TP_VALUE      10
#define ID_SL_VALUE      11
#define ID_ORDER_TYPE    12
#define ID_LOG_BOX       13

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        CreateWindowW(L"STATIC", L"Capstone SPX Options Bot Trader GUI (prototype)", WS_CHILD | WS_VISIBLE,
            20, 10, 500, 20, hwnd, NULL, NULL, NULL);

        // Front Leg Group
        CreateWindowW(L"BUTTON", L"Front Leg", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 40, 250, 80, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Option", WS_CHILD | WS_VISIBLE,
            30, 60, 50, 20, hwnd, NULL, NULL, NULL);
        HWND hFrontOption = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            80, 60, 100, 100, hwnd, (HMENU)ID_FRONT_OPTION, NULL, NULL);
        SendMessageW(hFrontOption, CB_ADDSTRING, 0, (LPARAM)L"Long Call");
        SendMessageW(hFrontOption, CB_ADDSTRING, 0, (LPARAM)L"Long Put");
        SendMessageW(hFrontOption, CB_SETCURSEL, 0, 0);
        CreateWindowW(L"STATIC", L"DTE", WS_CHILD | WS_VISIBLE,
            190, 60, 30, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            220, 60, 30, 20, hwnd, (HMENU)ID_FRONT_DTE, NULL, NULL);

        // Back Leg Group
        CreateWindowW(L"BUTTON", L"Back Leg", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 130, 250, 80, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Option", WS_CHILD | WS_VISIBLE,
            30, 150, 50, 20, hwnd, NULL, NULL, NULL);
        HWND hBackOption = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            80, 150, 100, 100, hwnd, (HMENU)ID_BACK_OPTION, NULL, NULL);
        SendMessageW(hBackOption, CB_ADDSTRING, 0, (LPARAM)L"Long Call");
        SendMessageW(hBackOption, CB_ADDSTRING, 0, (LPARAM)L"Long Put");
        SendMessageW(hBackOption, CB_SETCURSEL, 1, 0);
        CreateWindowW(L"STATIC", L"DTE", WS_CHILD | WS_VISIBLE,
            190, 150, 30, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            220, 150, 30, 20, hwnd, (HMENU)ID_BACK_DTE, NULL, NULL);

        // Position Settings Group
        CreateWindowW(L"BUTTON", L"Position Settings", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 210, 250, 80, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"STATIC", L"Entry Time", WS_CHILD | WS_VISIBLE,
            30, 230, 80, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            120, 230, 30, 20, hwnd, (HMENU)ID_ENTRY_HOUR, NULL, NULL);
        CreateWindowW(L"STATIC", L":", WS_CHILD | WS_VISIBLE,
            157, 230, 5, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            170, 230, 30, 20, hwnd, (HMENU)ID_ENTRY_MIN, NULL, NULL);

        CreateWindowW(L"STATIC", L"Exit Time", WS_CHILD | WS_VISIBLE,
            30, 260, 80, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            120, 260, 30, 20, hwnd, (HMENU)ID_EXIT_HOUR, NULL, NULL);
        CreateWindowW(L"STATIC", L":", WS_CHILD | WS_VISIBLE,
            157, 260, 5, 20, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            170, 260, 30, 20, hwnd, (HMENU)ID_EXIT_MIN, NULL, NULL);

        // TP/SL Order Group
        CreateWindowW(L"BUTTON", L"Exit Orders", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 300, 250, 80, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Orders Type:", WS_CHILD | WS_VISIBLE,
            30, 320, 90, 20, hwnd, NULL, NULL, NULL);
        HWND hOrderType = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            120, 320, 100, 100, hwnd, (HMENU)ID_ORDER_TYPE, NULL, NULL);
        SendMessageW(hOrderType, CB_ADDSTRING, 0, (LPARAM)L"OCO");
        SendMessageW(hOrderType, CB_ADDSTRING, 0, (LPARAM)L"Etc");
        SendMessageW(hOrderType, CB_SETCURSEL, 0, 0);



        // Start Button
        CreateWindowW(L"BUTTON", L"START", WS_CHILD | WS_VISIBLE,
            80, 400, 100, 30, hwnd, (HMENU)ID_START_BUTTON, NULL, NULL);
    }
                  break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_START_BUTTON) {
            MessageBoxW(hwnd, L"Bot Started!", L"Status", MB_OK);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"OptionsBotGUI";

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"OptionsBotGUI", L"Options Trading Bot", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 600, NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}