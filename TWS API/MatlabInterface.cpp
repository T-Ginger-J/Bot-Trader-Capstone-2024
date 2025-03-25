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

#include "samples/Cpp/CapstoneCPP/CapstoneCppClient.h"
#include "samples/Cpp/CapstoneCPP/IOUtils.h"

#include "samples/Cpp/CapstoneCPP/message_queue.h"


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

		Preset preset = Preset();

		// Save the text from the combo boxes
		GetWindowTextW(hFrontDTE, preset.hFrontDTE, 10);
		GetWindowTextW(hFrontStrike, preset.hFrontStrike, 10);
		GetWindowTextW(hBackDTE, preset.hBackDTE, 10);
		GetWindowTextW(hBackStrike, preset.hBackStrike, 10);
		GetWindowTextW(hEntryHour, preset.hEntryHour, 10);
		GetWindowTextW(hEntryMin, preset.hEntryMin, 10);
		GetWindowTextW(hExitHour, preset.hExitHour, 10); 
		GetWindowTextW(hExitMin, preset.hExitMin, 10);
		GetWindowTextW(hSL, preset.hSL, 10);
		GetWindowTextW(hTP, preset.hTP, 10);
		GetWindowTextW(hEntryPrice, preset.hEntryPrice, 10);

		preset.hFrontOption = SendMessageW(hFrontOption, CB_GETCURSEL, 0, 0);
		preset.hFrontAction = SendMessageW(hFrontAction, CB_GETCURSEL, 0, 0);
		preset.hBackOption = SendMessageW(hBackOption, CB_GETCURSEL, 0, 0);
		preset.hBackAction = SendMessageW(hBackAction, CB_GETCURSEL, 0, 0);
		preset.hOrderType = SendMessageW(hOrderType, CB_GETCURSEL, 0, 0);
		
		SaveSettingsIO(preset);

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

///////////////////

int comboPosition(char* host, int port, char* connectionOptions, int argc, char** argv, MessageQueue messageQueue) 
{
	twsMessageLoop(host, port, connectionOptions, messageQueue);
}
int singlePosition(char* host, int port, char* connectionOptions, int argc, char** argv, MessageQueue messageQueue) 
{
	twsMessageLoop(host, port, connectionOptions, messageQueue);
}
int cancelOrder(char* host, int port, char* connectionOptions, int argc, char** argv, MessageQueue messageQueue) 
{
	twsMessageLoop(host, port, connectionOptions, messageQueue);
}

int twsMessageLoop(const char* host, int port, const char* connectOptions, MessageQueue messageQueue) 
{
	int clientId = 0;
	unsigned attempt = 0;

	//TWS Message Loop below
	for (;;) {
		++attempt;
		printf( "Attempt %u of %u\n", attempt, MAX_ATTEMPTS);

		CapstoneCppClient client;

		// Run time error will occur (here) if TestCppClient.exe is compiled in debug mode but TwsSocketClient.dll is compiled in Release mode
		// TwsSocketClient.dll (in Release Mode) is copied by API installer into SysWOW64 folder within Windows directory 
		
		if( connectOptions) {
			client.setConnectOptions( connectOptions );
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

	const int ml_option = argc > 4 ? atoi(argv[4]) : 0;
	
	printf( "Start of IB C++ Socket Client For Capstone Project %u\n", attempt);

	MessageQueue messageQueue;

	//reformat argv for functions
	int new_argc = argc - 5;
	char** new_argv = new char*[new_argc];
	for (int i = 0; i < new_argc; i++)
	{
		new_argv[i] = argv[i + 5];
	}

	switch (ml_option) {
	case 1:
		return comboPosition(host, port, connectOptions, new_argc, new_argv, messageQueue);
	case 2:
		return singlePosition(host, port, connectOptions, new_argc, new_argv, messageQueue);
	case 3:
		return cancelOrder(host, port, connectOptions, new_argc, new_argv, messageQueue);
	}

	printf("No MATLAB option provided\n");
	return 1
}

