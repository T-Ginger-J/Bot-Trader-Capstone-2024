// MatlabInterface.cpp : This file contains the 'main' function. Program execution begins and ends there.


//#define UNICODE

#include <message_queue.h>
#include <CapstoneCppClient.h>

#include <string>
#include <thread>
#include <iostream>
#include <WinSock2.h>
#include <windows.h>
/*
using namespace std;*/
#include <iomanip>
#include <sstream>

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

typedef std::vector<std::tuple<std::string, std::string, int>> position_list;

enum OPTION { CALL, PUT };
enum ACTION { BUY, SELL };
enum EXIT_ORDER_TYPE { PERCENTAGE, OFFSET };

////////////////////////////////////////

//std::string customToString(double value) {
//	std::ostringstream oss;
//	oss << value;
//	return oss.str();
//}

void resetWorkingDirectory() {
	wchar_t exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	//PathRemoveFileSpec(exePath);
	SetCurrentDirectory(exePath);
}
OPTION getOptionFromString(std::string str) {
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
		throw new std::exception("Option must be CALL or PUT.");
	}
}
std::string getStringFromOption(OPTION option) {
	if (option == CALL)
	{
		return "CALL";
	}
	else if (option == PUT)
	{
		return "PUT";
	}
	else {
		throw new std::exception("Error in string conversion.");
	}
}
ACTION getActionFromString(std::string str) {
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
		throw new std::exception("Action must be BUY or SELL.");
	}
}
std::string getStringFromAction(ACTION action) {
	if (action == BUY)
	{
		return "BUY";
	}
	else if (action == SELL)
	{
		return "SELL";
	}
	else {
		throw new std::exception("Error in string conversion.");
	}
}
EXIT_ORDER_TYPE getTypeFromString(std::string str) {
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
		throw new std::exception("Exit Order Type must be PERCENTAGE or OFFSET.");
	}
}
std::string getStringFromType(EXIT_ORDER_TYPE type) {
	if (type == PERCENTAGE)
	{
		return "PERCENTAGE";
	}
	else if (type == OFFSET)
	{
		return "OFFSET";
	}
	else {
		throw new std::exception("Error in string conversion.");
	}
}
void saveText(std::string str, FILE* file) {
	fprintf_s(file, "%s,", str);
}

///////////////////

int StartTWSClient(const char* host, int port, const char* connectOptions)
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
			return 1;
		}

		printf("Sleeping %u seconds before next attempt\n", SLEEP_TIME);
		std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
	}

	return 0;
}


//TODO: Integrate with MATLAB
int executeBot(int argc, char** argv)
{
	//Inputs:
	//host
	//port
	//connectOptions
	//bot path

	const char* host = argc > 0 ? argv[0] : "";
	int port = argc > 1 ? atoi(argv[1]) : 0;
	if (port <= 0)
		port = 7497;
	const char* connectOptions = argc > 2 ? argv[2] : "+PACEAPI";

	if (argc < 4 || argv[3] == "") {
		printf("No bot path given.");
		return 1;
	}
	std::string filename = argv[4];

	//GET BOT
	/*int selIndex;
	if (selIndex == LB_ERR) {
		printf("No bot file selected.");
		return 1;
	}*/
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
			std::wstring ws(errMsg);
			std::string errorMessage(ws.begin(), ws.end());
			std::cout << errorMessage;
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

		return StartTWSClient(host, port, connectOptions);
	}
}
int createBot(int argc, char** argv)
{
	//inputs:
	//path
	//array [tuple<path, time, number>]
	std::tuple<char*, char*, int> position;
	position_list positions;

	if (argc < 1 || argv[0] == "") {
		printf("No bot path given.");
		return 1;
	}

	char* botfilename = argv[0];

	if ((argc - 1) % 3 == 0 && argc > 1) {
		printf("Invalid positions provided.");
		return 1;
	}

	for (int i = 1; i < argc; i += 3)
	{
		char* positionPath = argv[i];
		char* entryTime = argv[i + 1];
		int number = atoi(argv[i + 2]);
		position = std::make_tuple(positionPath, entryTime, number);
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
			wsprintfW(buffer, L"%s,%s,%s", std::get<1>(positions[i]), std::get<0>(positions[i]), std::get<2>(positions[i]));
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
int createPosition(int argc, char** argv)
{
	//inputs:
	//path
	//front leg - tuple(option{call,put}, expiry_date, strike_offset, action{buy,sell})
	//back leg - tuple(option{call,put}, expiry_date, strike_offset, action{buy,sell})
	//orders type {percentage, offset}
	//take_profit
	//stop_loss

	char* path;
	std::tuple<OPTION, char*, float, ACTION> frontLeg;
	std::tuple<OPTION, char*, float, ACTION> backLeg;
	EXIT_ORDER_TYPE exitOrderType;
	float takeProfit;
	float stopLoss;

	if (argc < 12) {
		printf("Incomplete Order Provided.");
		return 1;
	}

	char* botfilename = argv[0];

	std::get<0>(frontLeg) = getOptionFromString(argv[1]);
	std::get<1>(frontLeg) = argv[2];
	std::get<2>(frontLeg) = std::stof(argv[3]);
	std::get<3>(frontLeg) = getActionFromString(argv[4]);

	std::get<0>(backLeg) = getOptionFromString(argv[5]);
	std::get<1>(backLeg) = argv[6];
	std::get<2>(backLeg) = std::stof(argv[7]);
	std::get<3>(backLeg) = getActionFromString(argv[8]);

	exitOrderType = getTypeFromString(argv[9]);
	takeProfit = std::stof(argv[10]);
	stopLoss = std::stof(argv[11]);

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

		saveText(getStringFromOption(std::get<0>(frontLeg)), file);
		saveText(std::get<1>(frontLeg), file);
		saveText(std::to_string(std::get<2>(frontLeg)), file);
		saveText(getStringFromAction(std::get<3>(frontLeg)), file);

		saveText(getStringFromOption(std::get<0>(backLeg)), file);
		saveText(std::get<1>(backLeg), file);
		saveText(std::to_string(std::get<2>(backLeg)), file);
		saveText(getStringFromAction(std::get<3>(backLeg)), file);

		saveText(getStringFromType(exitOrderType), file);
		saveText(std::to_string(takeProfit), file);
		saveText(std::to_string(stopLoss), file);

		fclose(file);
		resetWorkingDirectory();
	}
	else {
		// Display save failed Message
		printf("Failed to save settings.");
		resetWorkingDirectory();
	}
}


/////////////////////////////////
int main(int argc, char** argv)
{
	/*
	* argv[0] = MatlabInterface.exe
	* argv[1] = ml_option
	* argv[2...] = args
	*/

	const int ml_option = argc > 1 ? atoi(argv[1]) : 0;

	printf("Start of IB C++ Socket Client For Capstone Project\n");

	//reformat argv for functions
	int new_argc = argc - 1;
	char** new_argv = new char* [new_argc];
	for (int i = 1; i < new_argc; i++)
	{
		new_argv[i] = argv[i + 1];
	}

	switch (ml_option) {
	case 1:
		return executeBot(argc, argv);
	case 2:
		return createBot(argc, argv);
	case 3:
		return createPosition(argc, argv);
	}

	printf("No MATLAB option provided\n");
	return 1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
