#include "SharedData.h"

#include "StdAfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <chrono>
#include <thread>

#include "CapstoneCppClient.h"

#include "message_queue.h"

// Position Creator IDs
#define ID_FRONT_OPTION  2
#define ID_BACK_OPTION   3
#define ID_FRONT_DTE     4
#define ID_BACK_DTE      5
#define ID_ENTRY_TIME    6
#define ID_ORDER_TYPE    7
#define ID_FRONT_STRIKE  8
#define ID_BACK_STRIKE   9
#define ID_TAKE_PROFIT   10
#define ID_STOP_LOSS     11
#define ID_FRONT_ACTION  12
#define ID_BACK_ACTION   13
#define ID_SAVE_BUTTON   14
#define ID_LOAD_BUTTON   15
#define ID_DELETE_BUTTON 16
#define ID_FILE_LIST     17

// Option Bot IDs
#define ID_SAVE_BUTTON_BOT     18
#define ID_ADD_BUTTON_BOT      19
#define ID_CLEAR_BUTTON_BOT    20
#define ID_BOT_DETAILS_LIST    21
#define ID_POSITION_LIST       22
#define ID_BOT_LIST            23
#define ID_ACTIVATE_BUTTON_BOT 24
#define ID_REMOVE_BUTTON_BOT   25
#define ID_NUMBER_LOTS         26
#define ID_DELETE_BUTTON_BOT   27


const unsigned MAX_ATTEMPTS = 50;
const unsigned SLEEP_TIME = 10;

//MessageQueue messageQueue;
//////////////////

// Postion Creator Variables
HWND hFrontDTE, hFrontStrike, hBackDTE, hBackStrike, hTP, hSL;
HWND hFrontOption, hFrontAction, hBackOption, hBackAction, hOrderType, hFileList;

// Option Bot Variables
HWND hPositionList, hBotDetails, hEntryTime, hBotList, hNumberLots;

// Resest Working Directory to .EXE home
void resetWorkingDirectory() {
	wchar_t exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	PathRemoveFileSpec(exePath);
	SetCurrentDirectory(exePath);
}

// Load position settings into position creator
void LoadPositionSettingsFromFile(const wchar_t* filePath) {
	FILE* file = NULL;
	errno_t err = _wfopen_s(&file, filePath, L"r");
	if (err == 0 && file) {
		wchar_t buffer[100];

		// Read First Line of CSV File
		if (fgetws(buffer, 100, file)) {
			wchar_t* context = NULL;
			wchar_t* token = wcstok_s(buffer, L",", &context);

			// Load Text Values Into the GUI Elements
#define LOAD_TEXT(hwnd) \
                if (token) { \
                    if (wcscmp(token, L"NULL") == 0) \
                        SetWindowTextW(hwnd, L""); \
                    else \
                        SetWindowTextW(hwnd, token); \
                    token = wcstok_s(NULL, L",", &context); \
                }

			LOAD_TEXT(hFrontDTE);
			LOAD_TEXT(hFrontStrike);
			LOAD_TEXT(hBackDTE);
			LOAD_TEXT(hBackStrike);
			LOAD_TEXT(hTP);
			LOAD_TEXT(hSL);

			// Load Combo Box Selections
#define LOAD_INDEX(hwnd) \
                if (token) { \
                    int index = _wtoi(token); \
                    SendMessageW(hwnd, CB_SETCURSEL, index, 0); \
                    token = wcstok_s(NULL, L",", &context); \
                }

			LOAD_INDEX(hFrontOption);
			LOAD_INDEX(hFrontAction);
			LOAD_INDEX(hBackOption);
			LOAD_INDEX(hBackAction);
			LOAD_INDEX(hOrderType);
		}

		fclose(file);
	}
	else {
		// Display Loading Failed Message
		MessageBoxW(NULL, L"Failed to load settings from the file.", L"Error", MB_ICONERROR);
	}
}

void LoadBotDataFromFile(const wchar_t* filePath, HWND listbox) {
	FILE* file = NULL;
	errno_t err = _wfopen_s(&file, filePath, L"r, ccs=UTF-16LE");  // Open the file as UTF-16

	if (err == 0 && file) {
		wchar_t buffer[1024];  // Buffer to hold each line from the file
		BOOL isFirstLine = TRUE;

		// Read each line from the file
		while (fgetws(buffer, sizeof(buffer) / sizeof(wchar_t), file)) {
			// Remove BOM on the first line if it exists (UTF-16 BOM is 0xFEFF)
			if (isFirstLine && buffer[0] == 0xFEFF) {
				memmove(buffer, buffer + 1, (wcslen(buffer) + 1) * sizeof(wchar_t)); // Remove BOM
				isFirstLine = FALSE;
			}

			// Trim newline character from the line
			size_t len = wcslen(buffer);
			if (len > 0 && buffer[len - 1] == L'\n') {
				buffer[len - 1] = L'\0';
			}

			// Add the line to the listbox
			SendMessageW(listbox, LB_ADDSTRING, 0, (LPARAM)buffer);
		}

		fclose(file);
	}
	else {
		// Display error if the file couldn't be opened
		MessageBoxW(NULL, L"Failed to load bot data from the file.", L"Error", MB_ICONERROR);
	}
}

void LoadSavedPositions(HWND currentWindow) {

	resetWorkingDirectory();

	// Create "Presets" directory if it doesn't exist
	if (_wmkdir(L"Presets") != 0 && errno != EEXIST) {

		// Error Handle
		MessageBoxW(NULL, L"Failed to create Presets directory.", L"Error", MB_ICONERROR);
		return;
	}

	// Clear the List Box First
	SendMessageW(currentWindow, LB_RESETCONTENT, 0, 0);

	// Find File Data
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(L"Presets\\*.csv", &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		if (error == 2) {
			// Handles no Files Found
			/*
			* wchar_t errorMsg[256];
			swprintf(errorMsg, 256, L"Could not find any files, error code: %lu", error);
			MessageBoxW(NULL, errorMsg, L"Error", MB_ICONERROR);
			*/
		}
		else {
			// Handles Other Errors
			wchar_t errorMsg[256];
			swprintf(errorMsg, 256, L"FindFirstFile failed with error code: %lu", error);
			MessageBoxW(NULL, errorMsg, L"Error", MB_ICONERROR);
		}

		return;
	}

	do {
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			// Add the file name to the list box
			SendMessageW(currentWindow, LB_ADDSTRING, 0, (LPARAM)findFileData.cFileName);
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
	resetWorkingDirectory();
}

void SavePosition() {
	resetWorkingDirectory();

	// Create "Presets" directory if it doesn't exist
	if (_wmkdir(L"Presets") != 0 && errno != EEXIST) {
		// Handle directory creation error
		MessageBoxW(NULL, L"Failed to create Presets directory.", L"Error", MB_ICONERROR);
		return;
	}

	// Ask the user to select a file name
	OPENFILENAMEW ofn;
	wchar_t szFile[260];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"CSV Files\0*.csv\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile[0] = '\0';
	ofn.lpstrDefExt = L"csv";
	ofn.lpstrInitialDir = L"Presets\\";

	// Display Save As dialog box
	if (GetSaveFileNameW(&ofn) == 0) {
		return; // User cancelled the file dialog
	}

	// Create the full file path in the "Presets" directory
	wchar_t filePath[MAX_PATH];
	wcscpy_s(filePath, L"Presets\\");
	PathAppendW(filePath, ofn.lpstrFile);

	FILE* file = NULL;
	errno_t err = _wfopen_s(&file, filePath, L"w");
	if (err == 0 && file) {
		wchar_t buffer[100];

		// Save the text from the text boxes
#define SAVE_TEXT(hwnd) \
            memset(buffer, 0, sizeof(buffer)); \
            GetWindowTextW(hwnd, buffer, 10); \
            fwprintf(file, L"%s,", buffer[0] ? buffer : L"NULL");

		SAVE_TEXT(hFrontDTE);
		SAVE_TEXT(hFrontStrike);
		SAVE_TEXT(hBackDTE);
		SAVE_TEXT(hBackStrike);
		SAVE_TEXT(hTP);
		SAVE_TEXT(hSL);

		// Save the index from the drop boxes
		int index;
#define SAVE_INDEX(hwnd) \
            index = SendMessageW(hwnd, CB_GETCURSEL, 0, 0); \
            fwprintf(file, L"%d,", (index != CB_ERR) ? index : -1);


		SAVE_INDEX(hFrontOption);
		SAVE_INDEX(hFrontAction);
		SAVE_INDEX(hBackOption);
		SAVE_INDEX(hBackAction);
		SAVE_INDEX(hOrderType);

		fclose(file);
		resetWorkingDirectory();

		// Update both lists that show all positions
		LoadSavedPositions(hFileList);
		LoadSavedPositions(hPositionList);
	}
	else {
		// Display save failed Message
		MessageBoxW(NULL, L"Failed to save settings.", L"Error", MB_ICONERROR);
		resetWorkingDirectory();
	}
}

void LoadSavedBots(HWND currentWindow) {

	resetWorkingDirectory();

	// Create "Bots" directory if it doesn't exist
	if (_wmkdir(L"Bots") != 0 && errno != EEXIST) {

		// Error Handle
		MessageBoxW(NULL, L"Failed to create Presets directory.", L"Error", MB_ICONERROR);
		return;
	}

	// Clear the List Box First
	SendMessageW(currentWindow, LB_RESETCONTENT, 0, 0);

	// Find File Data
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(L"Bots\\*.bot", &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		if (error == 2) {
			// Handles no Files Found
			/*
			* wchar_t errorMsg[256];
			swprintf(errorMsg, 256, L"Could not find any files, error code: %lu", error);
			MessageBoxW(NULL, errorMsg, L"Error", MB_ICONERROR);
			*/
			
		}
		else {
			// Handles Other Errors
			wchar_t errorMsg[256];
			swprintf(errorMsg, 256, L"FindFirstFile failed with error code: %lu", error);
			MessageBoxW(NULL, errorMsg, L"Error", MB_ICONERROR);
		}

		return;
	}

	do {
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			// Add the file name to the list box
			SendMessageW(currentWindow, LB_ADDSTRING, 0, (LPARAM)findFileData.cFileName);
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
	resetWorkingDirectory();
}

void SaveBot() {

	resetWorkingDirectory();

	// Ensure "Bots" directory exists
	if (_wmkdir(L"Bots") != 0 && errno != EEXIST) {
		MessageBoxW(NULL, L"Failed to create Presets directory.", L"Error", MB_ICONERROR);
		return;
	}

	OPENFILENAMEW ofn;
	wchar_t szFile[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Bot Files (*.bot)\0*.bot\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = L"bot";
	ofn.lpstrInitialDir = L"Bots\\";

	// Show save file dialog
	if (GetSaveFileNameW(&ofn) == 0) {
		return;
	}

	wchar_t filePath[MAX_PATH];
	wcscpy_s(filePath, L"Bots\\");
	PathAppendW(filePath, ofn.lpstrFile);

	// Ensure the filename has the correct extension
	if (wcsstr(filePath, L".bot") == NULL) {
		wcscat_s(filePath, L".bot");
	}

	FILE* file = NULL;
	errno_t err = _wfopen_s(&file, filePath, L"w, ccs=UTF-8");
	if (err == 0 && file) {
		int count = SendMessageW(hBotDetails, LB_GETCOUNT, 0, 0);

		// Iterate through the list box and write each line to the file
		for (int i = 0; i < count; i++) {
			wchar_t buffer[256];
			SendMessageW(hBotDetails, LB_GETTEXT, i, (LPARAM)buffer);
			fwprintf(file, L"%s\n", buffer);
		}

		fclose(file);
		MessageBoxW(NULL, L"Bot saved successfully!", L"Success", MB_OK | MB_ICONINFORMATION);

		resetWorkingDirectory();
		SendMessageW(hBotDetails, LB_RESETCONTENT, 0, 0);
	}
	else {
		MessageBoxW(NULL, L"Failed to save bot file.", L"Error", MB_ICONERROR);
	}
}

void LoadSettings() {
	// Ask the user to select a preset file
	OPENFILENAMEW ofn;
	wchar_t szFile[260];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"CSV Files\0*.csv\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile[0] = '\0';
	ofn.lpstrDefExt = L"csv";
	ofn.lpstrInitialDir = L"Presets\\";

	// Display open file dialog box
	if (GetOpenFileNameW(&ofn) == 0) {
		return; // User cancelled the file dialog
	}

	// Create the full file path in the "Presets" directory
	wchar_t filePath[MAX_PATH];
	wcscpy_s(filePath, L"Presets\\");
	PathAppend(filePath, ofn.lpstrFile);

	FILE* file = NULL;
	errno_t err = _wfopen_s(&file, filePath, L"r");
	if (err == 0 && file) {
		wchar_t buffer[100];

		// Read CSV line
		if (fgetws(buffer, 100, file)) {
			wchar_t* context = NULL;
			wchar_t* token = wcstok_s(buffer, L",", &context);


			// Load text from CSV
			#define LOAD_TEXT(hwnd) \
                if (token) { \
                    if (wcscmp(token, L"NULL") == 0) \
                        SetWindowTextW(hwnd, L""); \
                    else \
                        SetWindowTextW(hwnd, token); \
                    token = wcstok_s(NULL, L",", &context); \
                }

			LOAD_TEXT(hFrontDTE);
			LOAD_TEXT(hFrontStrike);
			LOAD_TEXT(hBackDTE);
			LOAD_TEXT(hBackStrike);
			LOAD_TEXT(hTP);
			LOAD_TEXT(hSL);


			// Handle Loading Indicies
			int index;
			#define LOAD_CHOICE(hwnd) \
                if (token && swscanf_s(token, L"%d", &index) == 1) { \
                    SendMessageW(hwnd, CB_SETCURSEL, index, 0); \
                    token = wcstok_s(NULL, L",", &context); \
                }

			LOAD_CHOICE(hFrontOption);
			LOAD_CHOICE(hFrontAction);
			LOAD_CHOICE(hBackOption);
			LOAD_CHOICE(hBackAction);
			LOAD_CHOICE(hOrderType);
		}

		fclose(file);
		resetWorkingDirectory();

		// Update both lists that show all positions
		LoadSavedPositions(hFileList);
		LoadSavedPositions(hPositionList);
	}
	else {
		// Diplay loading failed message
		MessageBoxW(NULL, L"Failed to load settings.", L"Error", MB_ICONERROR);
		resetWorkingDirectory();
	}
}

void DeleteSelectedFile(const wchar_t* filePath) {
	if (DeleteFileW(filePath)) {
		MessageBoxW(NULL, L"File deleted successfully.", L"Success", MB_OK);
	}
	else {
		MessageBoxW(NULL, L"Failed to delete file.", L"Error", MB_OK);
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
		// Call / Putt Dropbox
		CreateWindowW(L"STATIC", L"Option", WS_CHILD | WS_VISIBLE,
			30, 60, 50, 20, hwnd, NULL, NULL, NULL);
		hFrontOption = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			80, 60, 100, 100, hwnd, (HMENU)ID_FRONT_OPTION, NULL, NULL);
		SendMessageW(hFrontOption, CB_ADDSTRING, 0, (LPARAM)L"Call");
		SendMessageW(hFrontOption, CB_ADDSTRING, 0, (LPARAM)L"Put");
		SendMessageW(hFrontOption, CB_SETCURSEL, 0, 0);
		// DTE of Back Leg
		CreateWindowW(L"STATIC", L"Expiry Date:", WS_CHILD | WS_VISIBLE,
			190, 60, 80, 20, hwnd, NULL, NULL, NULL);
		hFrontDTE = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER,
			270, 60, 80, 20, hwnd, (HMENU)ID_FRONT_DTE, NULL, NULL);
		// Strike Price Title and Edit Box
		CreateWindowW(L"STATIC", L"Strike Offset (+/-)", WS_CHILD | WS_VISIBLE,
			30, 90, 120, 20, hwnd, NULL, NULL, NULL);
		hFrontStrike = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER,
			140, 90, 40, 20, hwnd, (HMENU)ID_FRONT_STRIKE, NULL, NULL);
		CreateWindowW(L"STATIC", L"Action", WS_CHILD | WS_VISIBLE,
			190, 90, 50, 20, hwnd, NULL, NULL, NULL);
		// Indicate Buy / Sell Drop Box
		hFrontAction = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			240, 90, 100, 100, hwnd, (HMENU)ID_FRONT_ACTION, NULL, NULL);
		SendMessageW(hFrontAction, CB_ADDSTRING, 0, (LPARAM)L"BUY");
		SendMessageW(hFrontAction, CB_ADDSTRING, 0, (LPARAM)L"SELL");
		SendMessageW(hFrontAction, CB_SETCURSEL, 0, 0);

		// Back Leg Group
		CreateWindowW(L"BUTTON", L"Back Leg", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			20, 130, 350, 80, hwnd, NULL, NULL, NULL);
		// Call / Putt Dropbox
		CreateWindowW(L"STATIC", L"Option", WS_CHILD | WS_VISIBLE,
			30, 150, 50, 20, hwnd, NULL, NULL, NULL);
		hBackOption = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			80, 150, 100, 100, hwnd, (HMENU)ID_BACK_OPTION, NULL, NULL);
		SendMessageW(hBackOption, CB_ADDSTRING, 0, (LPARAM)L"Call");
		SendMessageW(hBackOption, CB_ADDSTRING, 0, (LPARAM)L"Put");
		SendMessageW(hBackOption, CB_SETCURSEL, 0, 0);
		// DTE of Back Leg
		CreateWindowW(L"STATIC", L"Expiry Date:", WS_CHILD | WS_VISIBLE,
			190, 150, 80, 20, hwnd, NULL, NULL, NULL);
		hBackDTE = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER,
			270, 150, 80, 20, hwnd, (HMENU)ID_BACK_DTE, NULL, NULL);
		// Strike Price Title and Edit Box
		CreateWindowW(L"STATIC", L"Strike Spread (+/-)", WS_CHILD | WS_VISIBLE,
			30, 180, 120, 20, hwnd, NULL, NULL, NULL);
		hBackStrike = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER,
			150, 180, 40, 20, hwnd, (HMENU)ID_FRONT_STRIKE, NULL, NULL);
		CreateWindowW(L"STATIC", L"Action", WS_CHILD | WS_VISIBLE,
			200, 180, 50, 20, hwnd, NULL, NULL, NULL);
		// Indicate Buy / Sell Drop Box
		hBackAction = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			250, 180, 100, 100, hwnd, (HMENU)ID_BACK_ACTION, NULL, NULL);
		SendMessageW(hBackAction, CB_ADDSTRING, 0, (LPARAM)L"BUY");
		SendMessageW(hBackAction, CB_ADDSTRING, 0, (LPARAM)L"SELL");
		SendMessageW(hBackAction, CB_SETCURSEL, 0, 0);

		// TP/SL Order Box
		CreateWindowW(L"BUTTON", L"Exit Orders", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			20, 220, 250, 120, hwnd, NULL, NULL, NULL);
		// Drop box to select either percentage input or whole number input
		CreateWindowW(L"STATIC", L"Orders Type:", WS_CHILD | WS_VISIBLE,
			30, 240, 90, 20, hwnd, NULL, NULL, NULL);
		hOrderType = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			120, 240, 125, 100, hwnd, (HMENU)ID_ORDER_TYPE, NULL, NULL);
		SendMessageW(hOrderType, CB_ADDSTRING, 0, (LPARAM)L"Percentage");
		SendMessageW(hOrderType, CB_ADDSTRING, 0, (LPARAM)L"Whole Number");
		SendMessageW(hOrderType, CB_SETCURSEL, 0, 0);
		// Take Profit
		CreateWindowW(L"STATIC", L"Take Profit:", WS_CHILD | WS_VISIBLE,
			30, 270, 90, 20, hwnd, NULL, NULL, NULL);
		hTP = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER,
			120, 270, 100, 20, hwnd, (HMENU)ID_TAKE_PROFIT, NULL, NULL);
		// Stop Loss
		CreateWindowW(L"STATIC", L"Stop Loss:", WS_CHILD | WS_VISIBLE,
			30, 300, 90, 20, hwnd, NULL, NULL, NULL);
		hSL = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER,
			120, 300, 100, 20, hwnd, (HMENU)ID_STOP_LOSS, NULL, NULL);

		// Save, Load, and Delete Buttons
		CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE,
			20, 500, 100, 30, hwnd, (HMENU)ID_SAVE_BUTTON, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Load", WS_CHILD | WS_VISIBLE,
			140, 500, 100, 30, hwnd, (HMENU)ID_LOAD_BUTTON, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Delete", WS_CHILD | WS_VISIBLE,
			260, 500, 100, 30, hwnd, (HMENU)ID_DELETE_BUTTON, NULL, NULL);

		// List of Saved Presets
		CreateWindowW(L"STATIC", L"Saved Presets", WS_CHILD | WS_VISIBLE,
			450, 40, 100, 20, hwnd, NULL, NULL, NULL);
		hFileList = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_STANDARD,
			450, 60, 180, 460, hwnd, (HMENU)ID_FILE_LIST, NULL, NULL);

		resetWorkingDirectory();
		LoadSavedPositions(hFileList);

	}
				  break;

	case WM_COMMAND:

		// TODO: Transfer this to activate button
		/*
		* if (LOWORD(wParam) == ID_START_BUTTON) {
			wchar_t buffer[12];
			Message mesg;

			// Read values
			GetWindowTextW(hFrontDTE, buffer, 12);
			int frontDTE = _wtoi(buffer);
			mesg.frontDTE = frontDTE; //ADD RELATIVE DTE AMT TO MESSAGE

			GetWindowTextW(hBackDTE, buffer, 12);
			int backDTE = _wtoi(buffer);
			mesg.backDTE = backDTE; //ADD RELATIVE DTE AMT TO MESSAGE

			GetWindowTextW(hFrontStrike, buffer, 12);////////
			int frontStrikeRelative = _wtoi(buffer);
			mesg.frontStrikeChangeAmt = frontStrikeRelative; //ADD RELATIVE AMT TO MESSAGE

			GetWindowTextW(hBackStrike, buffer, 12);/////////////
			int backStrikeRelative = _wtoi(buffer);
			mesg.backStrikeChangeAmt = backStrikeRelative; //ADD RELATIVE AMT TO MESSAGE

			///////////// TIME LOGIC WE WILL NEED
			GetWindowTextW(hEntryTime, buffer, 12);
			std::wstring entryTime = buffer;

			std::wstring fullTimeString = L" " + entryTime;
			mesg.activationTime.assign(fullTimeString);
			///////////////////////

			GetWindowTextW(hTP, buffer, 10);
			double takeProfit = _wtof(buffer);
			mesg.takeProfit = takeProfit;

			GetWindowTextW(hSL, buffer, 10);
			double stopLoss = _wtof(buffer);
			mesg.stopLoss = stopLoss;


			// Get selected option from Front Option Combo Box
			int frontOptionIndex = SendMessageW(GetDlgItem(hwnd, ID_FRONT_OPTION), CB_GETCURSEL, 0, 0);
			SendMessageW(GetDlgItem(hwnd, ID_FRONT_OPTION), CB_GETLBTEXT, frontOptionIndex, (LPARAM)buffer);
			wchar_t frontOption[50];
			wcscpy_s(frontOption, buffer);

			mesg.frontOption.assign(frontOption); //ADD CALL/PUT TO MESSAGE

			// Get selected option from Back Option Combo Box
			int backOptionIndex = SendMessageW(GetDlgItem(hwnd, ID_BACK_OPTION), CB_GETCURSEL, 0, 0);
			SendMessageW(GetDlgItem(hwnd, ID_BACK_OPTION), CB_GETLBTEXT, backOptionIndex, (LPARAM)buffer);
			wchar_t backOption[50];
			wcscpy_s(backOption, buffer);

			mesg.backOption.assign(backOption); //ADD CALL/PUT TO MESSAGE

			// Get selected action from Front Action Combo Box
			int frontActionIndex = SendMessageW(GetDlgItem(hwnd, ID_FRONT_ACTION), CB_GETCURSEL, 0, 0);
			SendMessageW(GetDlgItem(hwnd, ID_FRONT_ACTION), CB_GETLBTEXT, frontActionIndex, (LPARAM)buffer);
			wchar_t frontAction[50];
			wcscpy_s(frontAction, buffer);

			mesg.frontAction.assign(frontAction); //ADD ACTION TO MESSAGE

			// Get selected action from Back Action Combo Box
			int backActionIndex = SendMessageW(GetDlgItem(hwnd, ID_BACK_ACTION), CB_GETCURSEL, 0, 0);
			SendMessageW(GetDlgItem(hwnd, ID_BACK_ACTION), CB_GETLBTEXT, backActionIndex, (LPARAM)buffer);
			wchar_t backAction[50];
			wcscpy_s(backAction, buffer);

			mesg.backAction.assign(backAction);//ADD ACTION TO MESSAGE

			// Get Order Type
			int orderTypeIndex = SendMessageW(GetDlgItem(hwnd, ID_ORDER_TYPE), CB_GETCURSEL, 0, 0);
			SendMessageW(GetDlgItem(hwnd, ID_ORDER_TYPE), CB_GETLBTEXT, orderTypeIndex, (LPARAM)buffer);
			wchar_t orderType[50];
			wcscpy_s(orderType, buffer);

			mesg.orderType.assign(orderType);

			// Example: Show values in a message box
			/*wchar_t msg[512];
			swprintf(msg, 512,
				L"Front Option: %s\nBack Option: %s\nFront Action: %s\nBack Action: %s\nOrder Type: %s\n\n"
				L"Front DTE: %d\nBack DTE: %d\nEntry: %02d:%02d\nExit: %02d:%02d\nTP: %.2f\nSL: %.2f",
				frontOption, backOption, frontAction, backAction, orderType,
				frontDTE, backDTE, entryDate, entryTime, takeProfit, stopLoss);
			MessageBoxW(hwnd, msg, L"Bot Settings", MB_OK);

		std::wstring date = L"20250326 01:01:00"; //TEST ENTRY DATE AND TIME HERE // Format: "YYYYMMDD HH:MM:SS"


		messageQueue.push(mesg);

		}
		*/


		// Save button is pressed
		if (LOWORD(wParam) == ID_SAVE_BUTTON) {
			SavePosition();
		}

		// Load button is pressed
		else if (LOWORD(wParam) == ID_LOAD_BUTTON) {
			LoadSettings();
		}

		// Delete button is clicked
		else if (LOWORD(wParam) == ID_DELETE_BUTTON) {
			int index = SendMessageW(hFileList, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				wchar_t fileName[MAX_PATH];
				SendMessageW(hFileList, LB_GETTEXT, index, (LPARAM)fileName);
				wchar_t filePath[MAX_PATH];
				wcscpy_s(filePath, L"Presets\\");
				PathAppendW(filePath, fileName);
				DeleteSelectedFile(filePath);
				LoadSavedBots(hFileList);
			}
		}

		// File selection changes
		else if (LOWORD(wParam) == ID_FILE_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
			int index = SendMessageW(hFileList, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				wchar_t fileName[MAX_PATH];
				SendMessageW(hFileList, LB_GETTEXT, index, (LPARAM)fileName);
				// Load the selected file
				wchar_t filePath[MAX_PATH];
				wcscpy_s(filePath, L"Presets\\");
				PathAppendW(filePath, fileName);
				// Load settings from the selected file
				LoadPositionSettingsFromFile(filePath);
			}
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK BotWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:

		CreateWindowW(L"STATIC", L"Bot Creator", WS_CHILD | WS_VISIBLE,
			20, 10, 80, 20, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

		// Bot Creation Group
		CreateWindowW(L"BUTTON", L"Entry Time and Position", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			20, 40, 375, 240, hwnd, NULL, NULL, NULL);
		// Entry Time in HH:MM:SS format
		CreateWindowW(L"STATIC", L"Entry Time (HH:MM:SS)", WS_CHILD | WS_VISIBLE,
			30, 60, 165, 20, hwnd, NULL, NULL, NULL);
		hEntryTime = CreateWindowW(L"EDIT", L"HH:MM:SS", WS_CHILD | WS_VISIBLE | WS_BORDER,
			30, 80, 165, 20, hwnd, (HMENU)ID_ENTRY_TIME, NULL, NULL);
		// Number of "Lots"
		CreateWindowW(L"STATIC", L"Number of Lots:", WS_CHILD | WS_VISIBLE,
			30, 120, 110, 20, hwnd, NULL, NULL, NULL);
		hNumberLots = CreateWindowW(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER,
			30, 140, 165, 20, hwnd, (HMENU)ID_NUMBER_LOTS, NULL, NULL);
		// List of Saved Positions
		CreateWindowW(L"STATIC", L"Saved Positions", WS_CHILD | WS_VISIBLE,
			205, 60, 115, 20, hwnd, NULL, NULL, NULL);
		hPositionList = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_STANDARD,
			205, 80, 180, 100, hwnd, (HMENU)ID_POSITION_LIST, NULL, NULL);

		// Save, Clear, Remove, and Add Buttons
		CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE,
			30, 235, 100, 30, hwnd, (HMENU)ID_SAVE_BUTTON_BOT, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE,
			150, 235, 100, 30, hwnd, (HMENU)ID_ADD_BUTTON_BOT, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE,
			270, 235, 100, 30, hwnd, (HMENU)ID_CLEAR_BUTTON_BOT, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Remove Selection", WS_CHILD | WS_VISIBLE,
			255, 195, 130, 30, hwnd, (HMENU)ID_REMOVE_BUTTON_BOT, NULL, NULL);

		// Current Bot Group
		CreateWindowW(L"BUTTON", L"Current Bot (Lots, Time, Position)", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			405, 40, 250, 240, hwnd, NULL, NULL, NULL);
		// List of Current Bot Details
		CreateWindowW(L"STATIC", L"Selected Positions", WS_CHILD | WS_VISIBLE,
			415, 60, 140, 20, hwnd, NULL, NULL, NULL);
		hBotDetails = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_STANDARD,
			415, 80, 230, 200, hwnd, (HMENU)ID_BOT_DETAILS_LIST, NULL, NULL);


		// Activate Bot Group
		CreateWindowW(L"BUTTON", L"Activate Bot", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			20, 300, 250, 240, hwnd, NULL, NULL, NULL);
		// List of Saved Bots
		CreateWindowW(L"STATIC", L"Saved Bots", WS_CHILD | WS_VISIBLE,
			30, 320, 80, 20, hwnd, NULL, NULL, NULL);
		hBotList = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_STANDARD,
			30, 340, 230, 100, hwnd, (HMENU)ID_BOT_LIST, NULL, NULL);
		// Activate Button
		CreateWindowW(L"BUTTON", L"Activate Bot", WS_CHILD | WS_VISIBLE,
			90, 450, 100, 30, hwnd, (HMENU)ID_ACTIVATE_BUTTON_BOT, NULL, NULL);
		// Delete Button
		CreateWindowW(L"BUTTON", L"Delete Bot", WS_CHILD | WS_VISIBLE,
			90, 490, 100, 30, hwnd, (HMENU)ID_DELETE_BUTTON_BOT, NULL, NULL);
		

		resetWorkingDirectory();
		LoadSavedPositions(hPositionList);
		LoadSavedBots(hBotList);

		break;

	case WM_COMMAND:
		// Add Button is clicked
		if (LOWORD(wParam) == ID_ADD_BUTTON_BOT) {
			// Get the selected index from the position list
			int selIndex = SendMessageW(hPositionList, LB_GETCURSEL, 0, 0);
			if (selIndex != LB_ERR) {
				wchar_t positionText[256];
				wchar_t entryTimeText[256];
				wchar_t numberLotsText[256];

				// Retreive text from both sources, append, and add to bot details list
				SendMessageW(hPositionList, LB_GETTEXT, selIndex, (LPARAM)positionText);
				GetWindowTextW(hEntryTime, entryTimeText, 256);
				GetWindowTextW(hNumberLots, numberLotsText, 256);

				wchar_t combinedText[768];
				wsprintfW(combinedText, L"%s,%s,%s",entryTimeText, positionText, numberLotsText);

				SendMessageW(hBotDetails, LB_ADDSTRING, 0, (LPARAM)combinedText);
			}
		}

		// Clear Button is clicked
		else if (LOWORD(wParam) == ID_CLEAR_BUTTON_BOT) {
			// Clear the list box first
			SendMessageW(hBotDetails, LB_RESETCONTENT, 0, 0);
		}

		// Save Button is clicked
		else if (LOWORD(wParam) == ID_SAVE_BUTTON_BOT) {
			SaveBot();
			LoadSavedBots(hBotList);
		}

		// Activate Button is clicked
		else if (LOWORD(wParam) == ID_ACTIVATE_BUTTON_BOT) {

			int selIndex = SendMessageW(hBotList, LB_GETCURSEL, 0, 0);
			if (selIndex == LB_ERR) {
				MessageBoxW(hwnd, L"No bot file selected.", L"Error", MB_ICONERROR);
				break;
			}

			wchar_t botFileName[MAX_PATH] = { 0 };
			SendMessageW(hBotList, LB_GETTEXT, selIndex, (LPARAM)botFileName);

			wchar_t botFilePath[MAX_PATH];
			wcscpy_s(botFilePath, L"Bots\\");
			PathAppendW(botFilePath, botFileName);

			FILE* botFile = nullptr;
			if (_wfopen_s(&botFile, botFilePath, L"r") != 0 || !botFile) {
				MessageBoxW(hwnd, L"Failed to open the bot file.", L"Error", MB_ICONERROR);
				break;
			}


			wchar_t lineBuffer[512];
			while (fgetws(lineBuffer, 512, botFile)) {
				size_t len = wcslen(lineBuffer);
				while (len > 0 && (lineBuffer[len - 1] == L'\n' || lineBuffer[len - 1] == L'\r')) {
					lineBuffer[len - 1] = L'\0';
					len--;
				}

				wchar_t* context = nullptr;
				wchar_t* timeToken = wcstok_s(lineBuffer, L",", &context);
				wchar_t* csvFileToken = wcstok_s(nullptr, L",", &context);
				wchar_t* lotNumber = wcstok_s(nullptr, L",", &context);

				if (!timeToken || !csvFileToken || !lotNumber) {
					MessageBoxW(hwnd, L"Invalid bot file format.", L"Error", MB_ICONERROR);
					continue;
				}

				wchar_t csvFilePath[MAX_PATH];
				wcscpy_s(csvFilePath, L"Presets\\");
				PathAppendW(csvFilePath, csvFileToken);

				FILE* csvFile = nullptr;
				if (_wfopen_s(&csvFile, csvFilePath, L"r") != 0 || !csvFile) {
					wchar_t errMsg[256];
					swprintf(errMsg, 256, L"Failed to open preset file: %s", csvFilePath);
					MessageBoxW(hwnd, errMsg, L"Error", MB_ICONERROR);
					continue;
				}

				wchar_t csvLine[512];
				if (!fgetws(csvLine, 512, csvFile)) {
					MessageBoxW(hwnd, L"Failed to read preset file.", L"Error", MB_ICONERROR);
					fclose(csvFile);
					continue;
				}
				fclose(csvFile);

				wchar_t* csvContext = nullptr;
				wchar_t* token = wcstok_s(csvLine, L",", &csvContext);
				if (!token) continue;

				Message mesg = {};

				if (wcscmp(token, L"NULL") == 0)
					mesg.frontDTE = 0;
				else {
					mesg.frontDTE = _wtoi(token);
				}
				token = wcstok_s(nullptr, L",", &csvContext);


				if (token) {
					mesg.frontStrikeChangeAmt = (wcscmp(token, L"NULL") == 0) ? 0 : _wtoi(token);
					token = wcstok_s(nullptr, L",", &csvContext);
				}


				if (token) {
					mesg.backDTE = (wcscmp(token, L"NULL") == 0) ? 0 : _wtoi(token);
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				if (token) {
					mesg.backStrikeChangeAmt = (wcscmp(token, L"NULL") == 0) ? 0 : _wtoi(token);
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				if (token) {
					mesg.takeProfit = (wcscmp(token, L"NULL") == 0) ? 0.0 : _wtof(token);
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				if (token) {
					mesg.stopLoss = (wcscmp(token, L"NULL") == 0) ? 0.0 : _wtof(token);
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				const wchar_t* optionMapping[] = { L"Call", L"Put" };
				const wchar_t* actionMapping[] = { L"BUY", L"SELL" };
				const wchar_t* orderTypeMapping[] = { L"%", L"#" };

				if (token) {
					int idx = _wtoi(token);
					mesg.frontOption = (idx >= 0 && idx < 2) ? optionMapping[idx] : L"";
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				if (token) {
					int idx = _wtoi(token);
					mesg.frontAction = (idx >= 0 && idx < 2) ? actionMapping[idx] : L"";
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				if (token) {
					int idx = _wtoi(token);
					mesg.backOption = (idx >= 0 && idx < 2) ? optionMapping[idx] : L"";
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				if (token) {
					int idx = _wtoi(token);
					mesg.backAction = (idx >= 0 && idx < 2) ? actionMapping[idx] : L"";
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				if (token) {
					int idx = _wtoi(token);
					mesg.orderType = (idx >= 0 && idx < 2) ? orderTypeMapping[idx] : L"";
					token = wcstok_s(nullptr, L",", &csvContext);
				}

				std::wstring righthalftime = timeToken;

				if (!righthalftime.empty() && (unsigned char)righthalftime[0] == 0xEF &&
					(unsigned char)righthalftime[1] == 0xBB && (unsigned char)righthalftime[2] == 0xBF) {
					righthalftime.erase(0, 3);
				}

				std::time_t t = std::time(nullptr);
				std::tm tm{};
				localtime_s(&tm, &t);  

				std::wstringstream wss;
				wss << std::put_time(&tm, L"%Y%m%d");

				std::wstring finaltime = wss.str() + L" " + righthalftime;

				mesg.activationTime = finaltime;

				double lotValue = wcstold(lotNumber, NULL);
				
				mesg.lots = lotValue;

				messageQueue.push(mesg);
			}

			fclose(botFile);
		}

		// Remove Selection Button is clicked
		else if (LOWORD(wParam) == ID_REMOVE_BUTTON_BOT) {
			// Get the selected index from the bot details list
			int selIndex = SendMessageW(hBotDetails, LB_GETCURSEL, 0, 0);
			if (selIndex != LB_ERR) {
				SendMessageW(hBotDetails, LB_DELETESTRING, selIndex, 0);
			}
		}

		// Delete Bot Button is clicked
		else if (LOWORD(wParam) == ID_DELETE_BUTTON_BOT) {
			int index = SendMessageW(hBotList, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				wchar_t fileName[MAX_PATH];
				SendMessageW(hBotList, LB_GETTEXT, index, (LPARAM)fileName);
				wchar_t filePath[MAX_PATH];
				wcscpy_s(filePath, L"Bots\\");
				PathAppendW(filePath, fileName);
				DeleteSelectedFile(filePath);
				LoadSavedBots(hBotList);
			}
		}

		// Bot selection changes
		else if (LOWORD(wParam) == ID_BOT_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
			int index = SendMessageW(hBotList, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				wchar_t fileName[MAX_PATH];
				SendMessageW(hBotList, LB_GETTEXT, index, (LPARAM)fileName);
				// Load the selected file
				wchar_t filePath[MAX_PATH];
				wcscpy_s(filePath, L"Bots\\");
				PathAppendW(filePath, fileName);
				// Load settings from the selected file
				SendMessageW(hBotDetails, LB_RESETCONTENT, 0, 0);
				LoadBotDataFromFile(filePath, hBotDetails);
			}
		}
		break;

	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {

	WNDCLASSW wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"OptionsBotGUI";
	RegisterClassW(&wc);

	WNDCLASSW wc2 = { 0 };
	wc2.lpfnWndProc = BotWindowProc;
	wc2.hInstance = hInstance;
	wc2.lpszClassName = L"BotWindowClass";
	RegisterClassW(&wc2);

	HWND hwnd = CreateWindowW(L"OptionsBotGUI", L"Position Creator",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 700, 600, NULL, NULL, hInstance, NULL);
	if (!hwnd) return 0;

	HWND hwndSecond = CreateWindowW(L"BotWindowClass", L"Options Trading Bot",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 700, 600, NULL, NULL, hInstance, NULL);
	if (!hwndSecond) return 0;

	ShowWindow(hwnd, nCmdShow);
	ShowWindow(hwndSecond, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}


void StartGui() {
	HINSTANCE hInstance = GetModuleHandle(NULL);
	wWinMain(hInstance, NULL, NULL, SW_SHOWDEFAULT);
}

///////////////////////////

int main(int argc, char** argv)
{
	const char* host = argc > 1 ? argv[1] : "";
	int port = argc > 2 ? atoi(argv[2]) : 0;
	if (port <= 0)
		port = 7497;
	const char* connectOptions = argc > 3 ? argv[3] : "+PACEAPI";
	int clientId = 0;

	unsigned attempt = 0;
	printf("Start of IB C++ Socket Client For Capstone Project %u\n", attempt);

	printf("Starting GUI\n");
	std::thread guiThread(StartGui);


	//TWS Message Loop below
	for (;;) {
		++attempt;
		printf("Attempt %u of %u\n", attempt, MAX_ATTEMPTS);

		CapstoneCppClient client;

		// Run time error will occur (here) if TestCppClient.exe is compiled in debug mode but TwsSocketClient.dll is compiled in Release mode
		// TwsSocketClient.dll (in Release Mode) is copied by API installer into SysWOW64 folder within Windows directory 

		if (connectOptions) {
			client.setConnectOptions(connectOptions);
		}

		client.connect(host, port, clientId);

		while (client.isConnected()) {
			client.processMessages();
		}
		if (attempt >= MAX_ATTEMPTS) {
			break;
		}

		printf("Sleeping %u seconds before next attempt\n", SLEEP_TIME);
		std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
	}
	guiThread.join();
	printf("End of C++ Socket Client Test\n");
}

