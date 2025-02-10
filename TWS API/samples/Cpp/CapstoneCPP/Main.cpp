/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#define UNICODE
#include <windows.h>

#include "SharedData.h"

#include "StdAfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <chrono>
#include <thread>

#include "CapstoneCppClient.h"

#include "message_queue.h"


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
#define ID_FRONT_STRIKE  14
#define ID_BACK_STRIKE   15
#define ID_TAKE_PROFIT   16
#define ID_STOP_LOSS     17
#define ID_ENTRY_PRICE   18
#define ID_FRONT_ACTION  19
#define ID_BACK_ACTION   20
#define ID_SAVE_BUTTON   21
#define ID_LOAD_BUTTON   22

const unsigned MAX_ATTEMPTS = 50;
const unsigned SLEEP_TIME = 10;

//MessageQueue messageQueue;
//////////////////

HWND hFrontDTE, hFrontStrike, hBackDTE, hBackStrike, hEntryHour, hEntryMin, hExitHour, hExitMin, hTP, hSL, hEntryPrice;
HWND hFrontOption, hFrontAction, hBackOption, hBackAction, hOrderType;

void SetComboBoxSelection(HWND hComboBox, const wchar_t* value) {
    int itemCount = (int)SendMessageW(hComboBox, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < itemCount; i++) {
        wchar_t itemText[50];
        SendMessageW(hComboBox, CB_GETLBTEXT, i, (LPARAM)itemText);
        if (wcscmp(itemText, value) == 0) {
            SendMessageW(hComboBox, CB_SETCURSEL, i, 0);
            break;
        }
    }
}

void SaveSettings() {
    _wremove(L"config.txt");


    FILE* file = NULL;
    errno_t err = _wfopen_s(&file, L"config.txt", L"w");
    if (err == 0 && file) {
        wchar_t buffer[100];

        // Save the text from the combo boxes
        GetWindowTextW(hFrontDTE, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hFrontStrike, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hBackDTE, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hBackStrike, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hEntryHour, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hEntryMin, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hExitHour, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hExitMin, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hTP, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hSL, buffer, 10); fwprintf(file, L"%s\n", buffer);
        GetWindowTextW(hEntryPrice, buffer, 10); fwprintf(file, L"%s\n", buffer);

        int index = SendMessageW(hFrontOption, CB_GETCURSEL, 0, 0);
        fwprintf(file, L"%d\n", index);

        index = SendMessageW(hFrontAction, CB_GETCURSEL, 0, 0);
        fwprintf(file, L"%d\n", index);

        index = SendMessageW(hBackOption, CB_GETCURSEL, 0, 0);
        fwprintf(file, L"%d\n", index);

        index = SendMessageW(hBackAction, CB_GETCURSEL, 0, 0);
        fwprintf(file, L"%d\n", index);

        index = SendMessageW(hOrderType, CB_GETCURSEL, 0, 0);
        fwprintf(file, L"%d\n", index);

        fclose(file);
    }
}

void LoadSettings() {
    FILE* file = NULL;
    errno_t err = _wfopen_s(&file, L"config.txt", L"r");
    if (err == 0 && file) {
        wchar_t buffer[100];

        // Load values into edit controls
        if (fgetws(buffer, 10, file)) SetWindowTextW(hFrontDTE, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hFrontStrike, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hBackDTE, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hBackStrike, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hEntryHour, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hEntryMin, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hExitHour, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hExitMin, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hTP, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hSL, buffer);
        if (fgetws(buffer, 10, file)) SetWindowTextW(hEntryPrice, buffer);

        int index;
        if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hFrontOption, CB_SETCURSEL, index, 0);
        if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hFrontAction, CB_SETCURSEL, index, 0);
        if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hBackOption, CB_SETCURSEL, index, 0);
        if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hBackAction, CB_SETCURSEL, index, 0);
        if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hOrderType, CB_SETCURSEL, index, 0);

        fclose(file);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        CreateWindowW(L"STATIC", L"Capstone SPX Options Bot Trader GUI (prototype)", WS_CHILD | WS_VISIBLE,
            20, 10, 350, 20, hwnd, NULL, NULL, NULL);

        // Front Leg Group
        CreateWindowW(L"BUTTON", L"Front Leg", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 40, 350, 80, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Option", WS_CHILD | WS_VISIBLE,
            30, 60, 50, 20, hwnd, NULL, NULL, NULL);
        hFrontOption = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            80, 60, 100, 100, hwnd, (HMENU)ID_FRONT_OPTION, NULL, NULL);
        SendMessageW(hFrontOption, CB_ADDSTRING, 0, (LPARAM)L"CALL");
        SendMessageW(hFrontOption, CB_ADDSTRING, 0, (LPARAM)L"PUT");
        SendMessageW(hFrontOption, CB_SETCURSEL, 0, 0);
        CreateWindowW(L"STATIC", L"Expiry Date:", WS_CHILD | WS_VISIBLE,
            190, 60, 80, 20, hwnd, NULL, NULL, NULL);
        hFrontDTE = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            270, 60, 80, 20, hwnd, (HMENU)ID_FRONT_DTE, NULL, NULL);


        CreateWindowW(L"STATIC", L"Strike+/-", WS_CHILD | WS_VISIBLE,
            30, 90, 50, 20, hwnd, NULL, NULL, NULL);
        hFrontStrike = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            80, 90, 60, 20, hwnd, (HMENU)ID_FRONT_STRIKE, NULL, NULL);
        CreateWindowW(L"STATIC", L"Action", WS_CHILD | WS_VISIBLE,
            190, 90, 50, 20, hwnd, NULL, NULL, NULL);
        hFrontAction = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            240, 90, 100, 100, hwnd, (HMENU)ID_FRONT_ACTION, NULL, NULL);
        SendMessageW(hFrontAction, CB_ADDSTRING, 0, (LPARAM)L"BUY");
        SendMessageW(hFrontAction, CB_ADDSTRING, 0, (LPARAM)L"SELL");
        SendMessageW(hFrontAction, CB_SETCURSEL, 0, 0);

        // Back Leg Group
        CreateWindowW(L"BUTTON", L"Back Leg", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 130, 350, 80, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Option", WS_CHILD | WS_VISIBLE,
            30, 150, 50, 20, hwnd, NULL, NULL, NULL);
        hBackOption = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            80, 150, 100, 100, hwnd, (HMENU)ID_BACK_OPTION, NULL, NULL);
        SendMessageW(hBackOption, CB_ADDSTRING, 0, (LPARAM)L"CALL");
        SendMessageW(hBackOption, CB_ADDSTRING, 0, (LPARAM)L"PUT");
        SendMessageW(hBackOption, CB_SETCURSEL, 1, 0);
        CreateWindowW(L"STATIC", L"Expiry Date:", WS_CHILD | WS_VISIBLE,
            190, 150, 80, 20, hwnd, NULL, NULL, NULL);
        hBackDTE = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            270, 150, 80, 20, hwnd, (HMENU)ID_BACK_DTE, NULL, NULL);

        CreateWindowW(L"STATIC", L"Strike+/-", WS_CHILD | WS_VISIBLE,
            30, 180, 50, 20, hwnd, NULL, NULL, NULL);
        hBackStrike = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            80, 180, 60, 20, hwnd, (HMENU)ID_FRONT_STRIKE, NULL, NULL);
        CreateWindowW(L"STATIC", L"Action", WS_CHILD | WS_VISIBLE,
            190, 180, 50, 20, hwnd, NULL, NULL, NULL);
        hBackAction = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            240, 180, 100, 100, hwnd, (HMENU)ID_BACK_ACTION, NULL, NULL);
        SendMessageW(hBackAction, CB_ADDSTRING, 0, (LPARAM)L"BUY");
        SendMessageW(hBackAction, CB_ADDSTRING, 0, (LPARAM)L"SELL");
        SendMessageW(hBackAction, CB_SETCURSEL, 1, 0);

        // Position Settings Group
        CreateWindowW(L"BUTTON", L"Position Settings", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 210, 250, 80, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"STATIC", L"Entry Time", WS_CHILD | WS_VISIBLE,
            30, 230, 80, 20, hwnd, NULL, NULL, NULL);
        hEntryHour = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            120, 230, 30, 20, hwnd, (HMENU)ID_ENTRY_HOUR, NULL, NULL);
        CreateWindowW(L"STATIC", L":", WS_CHILD | WS_VISIBLE,
            157, 230, 5, 20, hwnd, NULL, NULL, NULL);
        hEntryMin = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            170, 230, 30, 20, hwnd, (HMENU)ID_ENTRY_MIN, NULL, NULL);

        CreateWindowW(L"STATIC", L"Exit Time", WS_CHILD | WS_VISIBLE,
            30, 260, 80, 20, hwnd, NULL, NULL, NULL);
        hExitHour = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            120, 260, 30, 20, hwnd, (HMENU)ID_EXIT_HOUR, NULL, NULL);
        CreateWindowW(L"STATIC", L":", WS_CHILD | WS_VISIBLE,
            157, 260, 5, 20, hwnd, NULL, NULL, NULL);
        hExitMin = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            170, 260, 30, 20, hwnd, (HMENU)ID_EXIT_MIN, NULL, NULL);

        // TP/SL Order Group
        CreateWindowW(L"BUTTON", L"Exit Orders", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 300, 350, 80, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Orders Type:", WS_CHILD | WS_VISIBLE,
            30, 320, 90, 20, hwnd, NULL, NULL, NULL);
        hOrderType = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            120, 320, 100, 100, hwnd, (HMENU)ID_ORDER_TYPE, NULL, NULL);
        SendMessageW(hOrderType, CB_ADDSTRING, 0, (LPARAM)L"OCO");
        SendMessageW(hOrderType, CB_ADDSTRING, 0, (LPARAM)L"Etc");
        SendMessageW(hOrderType, CB_SETCURSEL, 0, 0);
        CreateWindowW(L"STATIC", L"Take Profit:", WS_CHILD | WS_VISIBLE,
            30, 350, 90, 20, hwnd, NULL, NULL, NULL);
        hTP = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            120, 350, 30, 20, hwnd, (HMENU)ID_TAKE_PROFIT, NULL, NULL);
        CreateWindowW(L"STATIC", L"Stop Loss:", WS_CHILD | WS_VISIBLE,
            160, 350, 90, 20, hwnd, NULL, NULL, NULL);
        hSL = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            250, 350, 30, 20, hwnd, (HMENU)ID_STOP_LOSS, NULL, NULL);

        // Trigger Method
        CreateWindowW(L"BUTTON", L"Entry Orders", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            20, 390, 350, 80, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Entry Price:", WS_CHILD | WS_VISIBLE,
            30, 410, 90, 20, hwnd, NULL, NULL, NULL);
        hEntryPrice = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            120, 410, 80, 20, hwnd, (HMENU)ID_ENTRY_PRICE, NULL, NULL);


        // Buttons
        CreateWindowW(L"BUTTON", L"START", WS_CHILD | WS_VISIBLE,
            20, 500, 100, 30, hwnd, (HMENU)ID_START_BUTTON, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE,
            140, 500, 100, 30, hwnd, (HMENU)ID_SAVE_BUTTON, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Load", WS_CHILD | WS_VISIBLE,
            260, 500, 100, 30, hwnd, (HMENU)ID_LOAD_BUTTON, NULL, NULL);

    }
                  break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_START_BUTTON) {
            wchar_t buffer[10];
            Message mesg;

            // Read values
            GetWindowTextW(hFrontDTE, buffer, 10);
            int frontDTE = _wtoi(buffer);
            mesg.frontDTE = frontDTE;

            GetWindowTextW(hFrontStrike, buffer, 10);
            int frontStrikeRelative = _wtoi(buffer);
            mesg.frontStrikeChangeAmt = frontStrikeRelative;

            GetWindowTextW(hBackDTE, buffer, 10);
            int backDTE = _wtoi(buffer);

            GetWindowTextW(hEntryHour, buffer, 10);
            int entryHour = _wtoi(buffer);

            GetWindowTextW(hEntryMin, buffer, 10);
            int entryMin = _wtoi(buffer);

            GetWindowTextW(hExitHour, buffer, 10);
            int exitHour = _wtoi(buffer);

            GetWindowTextW(hExitMin, buffer, 10);
            int exitMin = _wtoi(buffer);

            GetWindowTextW(hTP, buffer, 10);
            double takeProfit = _wtof(buffer);

            GetWindowTextW(hSL, buffer, 10);
            double stopLoss = _wtof(buffer);

            GetWindowTextW(hEntryPrice, buffer, 10);
            double entryPrice = _wtof(buffer);

            // Get selected option from Front Option Combo Box
            int frontOptionIndex = SendMessageW(GetDlgItem(hwnd, ID_FRONT_OPTION), CB_GETCURSEL, 0, 0);
            SendMessageW(GetDlgItem(hwnd, ID_FRONT_OPTION), CB_GETLBTEXT, frontOptionIndex, (LPARAM)buffer);
            wchar_t frontOption[50];
            wcscpy_s(frontOption, buffer);

            mesg.frontOption.assign(frontOption);

            // Get selected option from Back Option Combo Box
            int backOptionIndex = SendMessageW(GetDlgItem(hwnd, ID_BACK_OPTION), CB_GETCURSEL, 0, 0);
            SendMessageW(GetDlgItem(hwnd, ID_BACK_OPTION), CB_GETLBTEXT, backOptionIndex, (LPARAM)buffer);
            wchar_t backOption[50];
            wcscpy_s(backOption, buffer);

            // Get selected action from Front Action Combo Box
            int frontActionIndex = SendMessageW(GetDlgItem(hwnd, ID_FRONT_ACTION), CB_GETCURSEL, 0, 0);
            SendMessageW(GetDlgItem(hwnd, ID_FRONT_ACTION), CB_GETLBTEXT, frontActionIndex, (LPARAM)buffer);
            wchar_t frontAction[50];
            wcscpy_s(frontAction, buffer);

            // Get selected action from Back Action Combo Box
            int backActionIndex = SendMessageW(GetDlgItem(hwnd, ID_BACK_ACTION), CB_GETCURSEL, 0, 0);
            SendMessageW(GetDlgItem(hwnd, ID_BACK_ACTION), CB_GETLBTEXT, backActionIndex, (LPARAM)buffer);
            wchar_t backAction[50];
            wcscpy_s(backAction, buffer);

            // Get Order Type
            int orderTypeIndex = SendMessageW(GetDlgItem(hwnd, ID_ORDER_TYPE), CB_GETCURSEL, 0, 0);
            SendMessageW(GetDlgItem(hwnd, ID_ORDER_TYPE), CB_GETLBTEXT, orderTypeIndex, (LPARAM)buffer);
            wchar_t orderType[50];
            wcscpy_s(orderType, buffer);

            // Example: Show values in a message box
            wchar_t msg[512];
            swprintf(msg, 512,
                L"Front Option: %s\nBack Option: %s\nFront Action: %s\nBack Action: %s\nOrder Type: %s\n\n"
                L"Front DTE: %d\nBack DTE: %d\nEntry: %02d:%02d\nExit: %02d:%02d\nTP: %.2f\nSL: %.2f\nEntry Price: %.2f",
                frontOption, backOption, frontAction, backAction, orderType,
                frontDTE, backDTE, entryHour, entryMin, exitHour, exitMin, takeProfit, stopLoss, entryPrice);
            MessageBoxW(hwnd, msg, L"Bot Settings", MB_OK);

            messageQueue.push(mesg);

        }
        else if (LOWORD(wParam) == ID_SAVE_BUTTON) {
            SaveSettings();
        }
        else if (LOWORD(wParam) == ID_LOAD_BUTTON) {
            LoadSettings();
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
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}


void StartGui() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    wWinMain(hInstance, NULL, NULL, SW_SHOWDEFAULT);
}

///////////////////

/* IMPORTANT: always use your paper trading account. The code below will submit orders as part of the demonstration. */
/* IB will not be responsible for accidental executions on your live account. */
/* Any stock or option symbols displayed are for illustrative purposes only and are not intended to portray a recommendation. */
/* Before contacting our API support team please refer to the available documentation. */
int main(int argc, char** argv)
{
	const char* host = argc > 1 ? argv[1] : "";
	int port = argc > 2 ? atoi(argv[2]) : 0;
	if (port <= 0)
		port = 7497;
	const char* connectOptions = argc > 3 ? argv[3] : "+PACEAPI";
	int clientId = 0;

	unsigned attempt = 0;
	printf( "Start of IB C++ Socket Client For Capstone Project %u\n", attempt);

	printf("Starting GUI\n");
    std::thread guiThread(StartGui);


	//TWS Message Loop below
	for (;;) {
		++attempt;
		printf( "Attempt %u of %u\n", attempt, MAX_ATTEMPTS);

		CapstoneCppClient client;

		// Run time error will occur (here) if TestCppClient.exe is compiled in debug mode but TwsSocketClient.dll is compiled in Release mode
		// TwsSocketClient.dll (in Release Mode) is copied by API installer into SysWOW64 folder within Windows directory 
		
		if( connectOptions) {
			client.setConnectOptions( connectOptions);
		}
		
		client.connect( host, port, clientId);
		
		while( client.isConnected()) {
			client.processMessages();
		}
		if( attempt >= MAX_ATTEMPTS) {
			break;
		}

		printf( "Sleeping %u seconds before next attempt\n", SLEEP_TIME);
		std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
	}
    guiThread.join();
	printf ( "End of C++ Socket Client Test\n");
}

