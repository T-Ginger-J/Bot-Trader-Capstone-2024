/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#define UNICODE
#include <windows.h>
#include <iostream>
#include <sstream>
using namespace std;

//#include "SharedData.h"

#include "StdAfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <chrono>
#include <thread>

#include "CapstoneCppClient.h"

#include "message_queue.h"
#include <iomanip>


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

typedef vector<tuple<string, string, int>> position_list;

MessageQueue messageQueue;

enum OPTION { CALL, PUT };
enum ACTION { BUY, SELL };
enum EXIT_ORDER_TYPE { PERCENTAGE, OFFSET };

//////////////////

// void SaveSettings() {
// 	_wremove(L"config.txt");

// 	FILE* file = NULL;
// 	errno_t err = _wfopen_s(&file, L"config.txt", L"w");

// 	if (err == 0 && file) {

// 		Preset preset = Preset();

// 		// Save the text from the combo boxes
// 		GetWindowTextW(hFrontDTE, preset.hFrontDTE, 10);
// 		GetWindowTextW(hFrontStrike, preset.hFrontStrike, 10);
// 		GetWindowTextW(hBackDTE, preset.hBackDTE, 10);
// 		GetWindowTextW(hBackStrike, preset.hBackStrike, 10);
// 		GetWindowTextW(hEntryHour, preset.hEntryHour, 10);
// 		GetWindowTextW(hEntryMin, preset.hEntryMin, 10);
// 		GetWindowTextW(hExitHour, preset.hExitHour, 10); 
// 		GetWindowTextW(hExitMin, preset.hExitMin, 10);
// 		GetWindowTextW(hSL, preset.hSL, 10);
// 		GetWindowTextW(hTP, preset.hTP, 10);
// 		GetWindowTextW(hEntryPrice, preset.hEntryPrice, 10);

// 		preset.hFrontOption = SendMessageW(hFrontOption, CB_GETCURSEL, 0, 0);
// 		preset.hFrontAction = SendMessageW(hFrontAction, CB_GETCURSEL, 0, 0);
// 		preset.hBackOption = SendMessageW(hBackOption, CB_GETCURSEL, 0, 0);
// 		preset.hBackAction = SendMessageW(hBackAction, CB_GETCURSEL, 0, 0);
// 		preset.hOrderType = SendMessageW(hOrderType, CB_GETCURSEL, 0, 0);

// 		SaveSettingsIO(preset);

// 		fclose(file);
// 	}
// }

// void LoadSettings() {
// 	FILE* file = NULL;
// 	errno_t err = _wfopen_s(&file, L"config.txt", L"r");
// 	if (err == 0 && file) {
// 		wchar_t buffer[100];

// 		// Load values into edit controls
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hFrontDTE, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hFrontStrike, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hBackDTE, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hBackStrike, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hEntryHour, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hEntryMin, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hExitHour, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hExitMin, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hTP, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hSL, buffer);
// 		if (fgetws(buffer, 10, file)) SetWindowTextW(hEntryPrice, buffer);

// 		int index;
// 		if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hFrontOption, CB_SETCURSEL, index, 0);
// 		if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hFrontAction, CB_SETCURSEL, index, 0);
// 		if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hBackOption, CB_SETCURSEL, index, 0);
// 		if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hBackAction, CB_SETCURSEL, index, 0);
// 		if (fwscanf_s(file, L"%d\n", &index) == 1) SendMessageW(hOrderType, CB_SETCURSEL, index, 0);

// 		fclose(file);
// 	}
// }

void resetWorkingDirectory() {
	wchar_t exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	PathRemoveFileSpec(exePath);
	SetCurrentDirectory(exePath);
}
OPTION getOptionFromString(string str) {
	if (str.compare("CALL"))
	{
		return CALL;
	}
	else if (str.compare("PUT"))
	{
		return PUT;
	}
	else
	{
		throw new exception("Option must be CALL or PUT.");
	}
}
string getStringFromOption(OPTION option) {
	if (option == CALL) 
	{
		return "CALL";
	}
	else if (option == PUT) 
	{
		return "PUT";
	}
	else {
		throw new exception("Error in string conversion.");
	}
}
ACTION getActionFromString(string str) {
	if (str.compare("BUY"))
	{
		return BUY;
	}
	else if (str.compare("SELL"))
	{
		return SELL;
	}
	else
	{
		throw new exception("Action must be BUY or SELL.");
	}
}
string getStringFromAction(ACTION action) {
	if (action == BUY)
	{
		return "BUY";
	}
	else if (action == SELL)
	{
		return "SELL";
	}
	else {
		throw new exception("Error in string conversion.");
	}
}
EXIT_ORDER_TYPE getTypeFromString(string str) {
	if (str.compare("PERCENTAGE"))
	{
		return PERCENTAGE;
	}
	else if (str.compare("OFFSET"))
	{
		return OFFSET;
	}
	else
	{
		throw new exception("Exit Order Type must be PERCENTAGE or OFFSET.");
	}
}
string getStringFromType(EXIT_ORDER_TYPE type) {
	if (type == PERCENTAGE)
	{
		return "PERCENTAGE";
	}
	else if (type == OFFSET)
	{
		return "OFFSET";
	}
	else {
		throw new exception("Error in string conversion.");
	}
}
void saveText(string str, FILE* file) {
	fwprintf(file, L"%s,", str);
}

///////////////////

//TODO: Refactor so that the functions are: execute, make bot, make position
//TODO: Integrate with MATLAB
int executeBot(const char* host, int port, const char* connectionOptions, int argc, char** argv)
{
	//Inputs:
	//bot path

	if (argc < 1) {
		printf("No bot path given.");
		return 1;
	}
	string filename = argv[0];

	//GET BOT
	int selIndex;
	if (selIndex == LB_ERR) {
		printf("No bot file selected.");
		return 1;
	}
	wchar_t botFileName[MAX_PATH];
	wchar_t botFilePath[MAX_PATH];
	wcscpy_s(botFileName, std::wstring(filename.begin(), filename.end()).c_str());

	wcscpy_s(botFilePath, L"Bots\\");
	PathAppendW(botFilePath, botFileName);

	FILE* botFile = nullptr;
	if (_wfopen_s(&botFile, botFilePath, L"r") != 0 || !botFile) {
		printf("Failed to open the bot file.");
		return 1;
	}

	//walk through bot
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
			printf("Invalid bot file format.");
			continue;
		}

		wchar_t csvFilePath[MAX_PATH];
		wcscpy_s(csvFilePath, L"Presets\\");
		PathAppendW(csvFilePath, csvFileToken);

		FILE* csvFile = nullptr;
		if (_wfopen_s(&csvFile, csvFilePath, L"r") != 0 || !csvFile) {
			wchar_t errMsg[256];
			swprintf(errMsg, 256, L"Failed to open preset file: %s", csvFilePath);
			cout << errMsg;
			continue;
		}

		wchar_t csvLine[512];
		if (!fgetws(csvLine, 512, csvFile)) {
			printf("Failed to read preset file.");
			fclose(csvFile);
			continue;
		}
		fclose(csvFile);

		wchar_t* csvContext = nullptr;
		wchar_t* token = wcstok_s(csvLine, L",", &csvContext);
		if (!token) continue;

		Message mesg = {};

		//IMPORT Front DTE
		if (wcscmp(token, L"NULL") == 0)
			mesg.frontDTE = 0;
		else {
			mesg.frontDTE = _wtoi(token);
		}
		token = wcstok_s(nullptr, L",", &csvContext);

		//IMPORT INPUTS
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

		return 0;
	}
}
int createBot(const char* host, int port, const char* connectionOptions, int argc, char** argv)
{
	//inputs:
	//path
	//array [tuple<path, time, number>]
	tuple<char*, char*, int> position;
	position_list positions;

	if (argc < 1) {
		printf("No bot path given.");
		return 1;
	}

	char* botfilename = argv[0];

	if ((argc - 1) % 3 == 0 && argc >= 4) {
		printf("Invalid positions provided.");
		return 1;
	}

	for (size_t i = 1; i < argc; i += 3)
	{
		char* positionPath = argv[i];
		char* entryTime = argv[i + 1];
		int number = atoi(argv[2]);
		position = make_tuple(positionPath, entryTime, number);
		positions.push_back(position);
	}

	resetWorkingDirectory();

	// Ensure "Bots" directory exists
	if (_wmkdir(L"Bots") != 0 && errno != EEXIST) {
		printf("Failed to create Presets directory.");
		return 1;
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
	/*if (GetSaveFileNameW(&ofn) == 0) {
		return;
	}*/

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
		int count = positions.size();

		// Iterate through the list box and write each line to the file
		for (int i = 0; i < count; i++) {
			wchar_t buffer[256];
			wsprintfW(buffer, L"%s,%s,%s", get<1>(positions[i]), get<0>(positions[i]), get<2>(positions[i]));
			fwprintf(file, L"%s\n", buffer);
		}

		fclose(file);
		printf("Bot saved successfully!");

		resetWorkingDirectory();
		return 0;
	}
	else {
		printf("Failed to save bot file!");
		return 1;
	}
}
int createPosition(const char* host, int port, const char* connectionOptions, int argc, char** argv)
{
	//inputs:
	//path
	//front leg - tuple(option{call,put}, expiry_date, strike_offset, action{buy,sell})
	//back leg - tuple(option{call,put}, expiry_date, strike_offset, action{buy,sell})
	//orders type {percentage, ____}
	//take_profit
	//stop_loss
	
	char* path;
	tuple<OPTION, char*, float, ACTION> frontLeg;
	tuple<OPTION, char*, float, ACTION> backLeg;
	EXIT_ORDER_TYPE exitOrderType;
	float takeProfit;
	float stopLoss;

	if (argc < 12) {
		printf("Incomplete Order Provided.");
		return 1;
	}

	char* botfilename = argv[0];

	get<0>(frontLeg) = getOptionFromString(argv[1]);
	get<1>(frontLeg) = argv[2];
	get<2>(frontLeg) = stof(argv[3]);
	get<3>(frontLeg) = getActionFromString(argv[4]);

	get<0>(backLeg) = getOptionFromString(argv[5]);
	get<1>(backLeg) = argv[6];
	get<2>(backLeg) = stof(argv[7]);
	get<3>(backLeg) = getActionFromString(argv[8]);

	exitOrderType = getTypeFromString(argv[9]);
	takeProfit = stof(argv[10]);
	stopLoss = stof(argv[11]);

	resetWorkingDirectory();

	// Create "Presets" directory if it doesn't exist
	if (_wmkdir(L"Presets") != 0 && errno != EEXIST) {
		// Handle directory creation error
		printf("Failed to create Presets directory.");
		return 1;
	}

	// Create the full file path in the "Presets" directory
	wchar_t filePath[MAX_PATH];
	wchar_t botPath[MAX_PATH];
	wcscpy_s(filePath, L"Presets\\");
	wprintf_s(botPath, L"%s", botfilename);
	PathAppendW(filePath, botPath);

	FILE* file = NULL;
	errno_t err = _wfopen_s(&file, filePath, L"w");
	if (err == 0 && file) {

		saveText(getStringFromOption(get<0>(frontLeg)), file);
		saveText(get<1>(frontLeg), file);
		saveText(to_string(get<2>(frontLeg)), file);
		saveText(getStringFromAction(get<3>(frontLeg)), file);

		saveText(getStringFromOption(get<0>(backLeg)), file);
		saveText(get<1>(backLeg), file);
		saveText(to_string(get<2>(backLeg)), file);
		saveText(getStringFromAction(get<3>(backLeg)), file);

		saveText(getStringFromType(exitOrderType), file);
		saveText(to_string(takeProfit), file);
		saveText(to_string(stopLoss), file);

		fclose(file);
		resetWorkingDirectory();
	}
	else {
		// Display save failed Message
		printf("Failed to save settings.");
		resetWorkingDirectory();
	}
}

int StartTWSClient(const char* host, int port, const char* connectOptions, MessageQueue messageQueue)
{
	int clientId = 0;
	unsigned attempt = 0;

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

	printf("Start of IB C++ Socket Client For Capstone Project\n");

	//reformat argv for functions
	int new_argc = argc - 5;
	char** new_argv = new char* [new_argc];
	for (int i = 0; i < new_argc; i++)
	{
		new_argv[i] = argv[i + 5];
	}

	// MessageQueue messageQueue = MessageQueue();

	switch (ml_option) {
	case 1:
		return executeBot(host, port, connectOptions, new_argc, new_argv);
	case 2:
		return createBot(host, port, connectOptions, new_argc, new_argv);
	case 3:
		return createPosition(host, port, connectOptions, new_argc, new_argv);
	}

	printf("No MATLAB option provided\n");
	return 1;
}

