#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#include "StdAfx.h"

#include "CapstoneCppClient.h"

#include "EClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"
#include "OrderState.h"
#include "Execution.h"
#include "CommissionReport.h"
#include "ContractSamples.h"
#include "OrderSamples.h"
#include "ScannerSubscription.h"
#include "ScannerSubscriptionSamples.h"
#include "executioncondition.h"
#include "PriceCondition.h"
#include "MarginCondition.h"
#include "PercentChangeCondition.h"
#include "TimeCondition.h"
#include "VolumeCondition.h"
#include "AvailableAlgoParams.h"
#include "FAMethodSamples.h"
#include "CommonDefs.h"
#include "AccountSummaryTags.h"
#include "Utils.h"
#include "IneligibilityReason.h"

#include <stdio.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <ctime>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <iomanip>  
#include <sstream> 
#include <deque>


// Global (or class‐member) maps to hold partial data
// Key: orderId (from Execution) to trade record. We assume one complete trade per order.
std::map<int, TradeRecord> g_tradeMap;
// Map to relate execId to orderId (for use when commissionReport comes in)
std::map<std::string, int> g_execIdToOrderId;
// Map to accumulate commission amounts for a given order
std::map<int, double> g_orderCommission;


const int PING_DEADLINE = 2; // seconds
const int SLEEP_BETWEEN_PINGS = 15; // seconds

///////////////////////////////
//OUR GLOBAL VARIABLES
bool isFulfilled = false;
bool isComboCheck = false;
bool getIndexValue = true;

int underlyingConId;
int legOneConId = 0;
int legTwoConId = 0;
double spxCurrentPrice;

double currentBidPrice;
double currentAskPrice;
double currentMarketPrice;

double userStrikePriceLeg1;
std::string userDTELeg1;
std::string userCallOrPutLeg1;
std::string actionLeg1;

double userStrikePriceLeg2;
std::string userDTELeg2;
std::string userCallOrPutLeg2;
std::string actionLeg2;

bool usePercentage = false;
double takeProfitValue;
double stopLossValue;

Decimal amountOfLots;

std::unordered_map<OrderId, Contract> contractMap;

std::unordered_map<time_t, Message> pendingOrderMap;

std::deque<TradeRecord> openTrades;

time_t currentOrderFromMap;

int postOrderTickID = 2000;
double comboLimitPrice;
double currentOrderID;

///////////////////////////////////////////////////////////
// member funcs
//! [socket_init]
CapstoneCppClient::CapstoneCppClient() :
      m_osSignal(2000)//2-seconds timeout
    , m_pClient(new EClientSocket(this, &m_osSignal))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
    , m_extraAuth(false)
{
}
//! [socket_init]
CapstoneCppClient::~CapstoneCppClient()
{
	// destroy the reader before the client
	if( m_pReader )
		m_pReader.reset();

	delete m_pClient;
}

//////////////////////////////////////////////// TWS CONNECTION RELATED METHODS
bool CapstoneCppClient::connect(const char *host, int port, int clientId)
{
	// trying to connect
	printf( "Connecting to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);
	
	//! [connect]
	bool bRes = m_pClient->eConnect( host, port, clientId, m_extraAuth);
	//! [connect]
	
	if (bRes) {
		printf( "Connected to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
		//! [ereader]
		m_pReader = std::unique_ptr<EReader>( new EReader(m_pClient, &m_osSignal) );
		m_pReader->start();
		//! [ereader]
	}
	else
		printf( "Cannot connect to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);

	return bRes;
}

void CapstoneCppClient::disconnect() const
{
	m_pClient->eDisconnect();

	printf ( "Disconnected\n");
}

bool CapstoneCppClient::isConnected() const
{
	return m_pClient->isConnected();
}

void CapstoneCppClient::setConnectOptions(const std::string& connectOptions)
{
	m_pClient->setConnectOptions(connectOptions);
}

void CapstoneCppClient::setOptionalCapabilities(const std::string& optionalCapabilities)
{
    m_pClient->setOptionalCapabilities(optionalCapabilities);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CapstoneCppClient::processMessages()
{
	time_t now = time(NULL);
	//printf("\nHit at start of process messages check order where nextvalidID is called\n");
	printf("\nm_state value: %o\n",m_state);
	//printf("\nm_state TEST value: %o\n", ST_OPTIONSOPERATIONS_ACK);
	//printf("m_orderId value: %o\n\n", m_orderId);

	switch (m_state) {
		case ST_HISTORICALDATAREQUESTS:
			historicalDataRequests();
			break;
		case ST_HISTORICALDATAREQUESTS_ACK:
			break;
		case ST_ORDEROPERATIONS:
			orderOperations();
			break;
		case ST_ORDEROPERATIONS_ACK:
			break;
		case ST_OCASAMPLES:
			ocaSamples();
			break;
		case ST_OCASAMPLES_ACK:
			break;
		case ST_CONDITIONSAMPLES:
			conditionSamples();
			break;
		case ST_CONDITIONSAMPLES_ACK:
			break;
		case ST_BRACKETSAMPLES:
			bracketSample();
			break;
		case ST_BRACKETSAMPLES_ACK:
			break;
        case ST_REQHISTORICALTICKS:
            reqHistoricalTicks();
            break;
        case ST_REQHISTORICALTICKS_ACK:
            break;
		case ST_REQTICKBYTICKDATA:
			reqTickByTickData();
			break;
		case ST_REQTICKBYTICKDATA_ACK:
			break;

		case ST_PING:
			reqCurrentTime();
			break;
		case ST_PING_ACK:
			m_state = ST_IDLE;
			//if( m_sleepDeadline < now) {
			//	disconnect();
			//	printf("\n Disconnect was Hit: this means state was set to ST_PING_ACK and m_sleepDealline is less than 'now'\n");/////////////////
			//	return;
			//}
			break;
		case ST_IDLE:
			if( m_sleepDeadline < now) {
				m_state = ST_WAITFORINPUT;
				return;
			}
			break;
	/////////////////////////////////////////////////////////////////////////////////////////////////////

		case ST_OPTIONSOPERATIONS://////////////////
			optionsOperations();
			break;
		case ST_OPTIONSOPERATIONS_ACK:
			break;

		case ST_USERINPUT:
			break;

		case ST_COMBOPRICE:
			getComboPrices();
			break;
		case ST_COMBOPRICE_ACK:
			break;

		case ST_SINGLEORDER:
			break;
		case ST_SINGLEORDER_ACK:
			m_state = ST_PING;
			break;

		case ST_COMBOORDER:
			placeComboOrder();
			break;
		case ST_COMBOORDER_ACK:
			m_state = ST_PING;
			break;
		case ST_COMBOINFO:
			getComboOrder();
			break;

		case ST_WAITFORINPUT:
			waitForGuiDataAndTime();
			break;
	}

	m_osSignal.waitForSignal();
	errno = 0;
	m_pReader->processMsgs();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OUR METHODS HERE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////HELPER METHODS START
void appendTradeRecordToCsv(const TradeRecord& record, double grossProfit, double commission, double netProfit)
{
	const std::string csvFilename = "TradeRecords.csv";
	std::ofstream csvFile(csvFilename, std::ios::out | std::ios::app);
	if (!csvFile.is_open()) {
		printf("Error opening CSV file for writing.\n");
		return;
	}
	// (Optional) Write header if file is empty. One way is to check file size.
	csvFile.seekp(0, std::ios::end);
	if (csvFile.tellp() == 0) {
		csvFile << "Symbol,EntryTime,ExitTime,EntryPrice,ExitPrice,Shares,GrossP/L,Commission,NetP/L\n";
	}
	csvFile << record.symbol << ","
		<< record.entryTime << ","
		<< record.exitTime << ","
		<< record.entryPrice << ","
		<< record.exitPrice << ","
		<< record.shares << ","
		<< grossProfit << ","
		<< commission << ","
		<< netProfit << "\n";
	csvFile.close();
}

void writeCsvHeaderIfNeeded(const std::string& filename) {
	static bool headerWritten = false;
	if (!headerWritten) {
		std::ofstream csvFile(filename, std::ios::out | std::ios::app);
		if (csvFile.tellp() == 0) {
			csvFile << "Profit,Loss,EntryTime,ExitTime,FillPrice,Commission\n";
		}
		headerWritten = true;
	}
}

time_t parseDate(const std::string& dateStr) {
	struct tm tm = {};

	std::istringstream ss(dateStr);
	ss >> std::get_time(&tm, "%Y%m%d");

	if (ss.fail()) {
		std::cerr << "Failed to parse date: " << dateStr << std::endl;
		return -1;
	}

	return mktime(&tm);
}

std::string wstringToString(const std::wstring& wstr) {
	size_t len = wcstombs(nullptr, wstr.c_str(), 0) + 1;
	if (len == static_cast<size_t>(-1)) return "";  // Conversion error
	char* buffer = new char[len];
	wcstombs(buffer, wstr.c_str(), len);
	std::string str(buffer);
	delete[] buffer;
	return str;
}

time_t CapstoneCppClient::parseActivationTime(const Message& msg) {
	std::wstring ws = msg.activationTime;
	std::string timeStr(ws.begin(), ws.end());

	// Parse "YYYYMMDD HH:MM:SS"
	struct tm tm = {};
	std::istringstream ss(timeStr);

	ss >> std::get_time(&tm, "%Y%m%d %H:%M:%S");
	if (ss.fail()) {
		printf("Failed to parse activation time: %s\n", timeStr.c_str());
		return 0;
	}

	tm.tm_isdst = -1;

	return _mkgmtime(&tm) - CapstoneCppClient::getTzOffset(msg.timeZone);
}

int CapstoneCppClient::getTzOffset(const std::wstring& tz) {
	_tzset(); 
	long timezoneSec;
	_get_timezone(&timezoneSec);
	int daylight;
	_get_daylight(&daylight);
	return -timezoneSec + (daylight ? 3600 : 0);
}

////////////HELPER METHODS END

void CapstoneCppClient::waitForGuiDataAndTime() {
	static std::vector<Message> pendingMessages;

	if (!messageQueue.isEmpty()) {
		Message msg = messageQueue.pop();
		msg.scheduledTime = parseActivationTime(msg);
		pendingMessages.push_back(msg);
		//messageQueue.push(msg);
		printf("Received order scheduled for: %s",
			std::ctime(&msg.scheduledTime));
		pendingOrderMap[msg.scheduledTime] = msg;

	}

	time_t now = time(nullptr);
	
	//CLOSING DAY CHECK AND LOGIC WILL BE BELOW
	/*std::tm now_tm = *std::localtime(&now);
	// Set the target time (3:50 PM of the current day)
	std::tm target_tm = now_tm; // Copy the current time
	target_tm.tm_hour = 15;    // 3 PM
	target_tm.tm_min = 50;      // 50 minutes
	target_tm.tm_sec = 0;       // 0 seconds

	// Convert target time to time_t for comparison
	time_t target_time = mktime(&target_tm);

	if (now >= target_time) {
		printf("\n=== MARKET CLOSING TIME (3:50 PM) REACHED ===\n");

		//////////
		//CLOSE POSITIONS HERE
		///////////
		for (auto &tradePair : g_tradeMap) {
			TradeRecord &record = tradePair.second;
			// If the exit time is empty, then the position is still open.
			if (record.exitTime.empty()) {
				printf("Closing open position for orderId %s, symbol: %s\n",
					   Utils::longMaxString(tradePair.first).c_str(), record.symbol.c_str());
				// Place an immediate market order to exit the position.
				exitPosition(record, tradePair.first);
			}
		}
		// Optionally, clear the open positions after sending exit orders.
		g_tradeMap.clear();

		return; 
	}*/

	auto it = pendingMessages.begin();
	while (it != pendingMessages.end()) {
		if (now >= it->scheduledTime && !it->processed) {
			printf("\n=== ACTIVATING ORDER SCHEDULED FOR %s ===\n",
				std::ctime(&it->scheduledTime));

			it->processed = true;

			currentOrderFromMap = it->scheduledTime;

			it = pendingMessages.erase(it);

			optionsOperations();
			return;
		}
		else {
			++it;
		}
	}



	printf("No scheduled orders ready - waiting for input\n");
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

void CapstoneCppClient::exitPosition(const TradeRecord& record, long originalOrderId) {

	Order exitOrder;
	// For a BUY (long) position, you need to sell; for a SELL (short) position, buy.
	exitOrder.action = (record.side == "BUY") ? "SELL" : "BUY";
	exitOrder.orderType = "MKT";             
	exitOrder.totalQuantity = record.shares;  

	Contract exitContract;
	auto it = contractMap.find(originalOrderId);
	if (it != contractMap.end()) {
		exitContract = it->second;
	}
	else {

		printf("Warning: Contract details for orderId %d not found in contractMap. Using default contract parameters.\n", originalOrderId);//THIS MEAN ERROR 
		exitContract.symbol = record.symbol;

		exitContract.secType = "OPT";  
		exitContract.exchange = "CBOE";
		exitContract.currency = "USD";
		exitContract.multiplier = "100";
		exitContract.tradingClass = "SPXW";
		// exitContract.lastTradeDateOrContractMonth = <expirationDate>;
		// exitContract.strike = <strikePrice>;
		// exitContract.right = <"C" or "P">;
		return;//RETURN FOR NOW
	}

	printf("Exit order placed for orderId %d: %s %s (%f shares)\n",
		m_orderId, exitOrder.action.c_str(), exitContract.symbol.c_str(), exitOrder.totalQuantity);
	m_pClient->placeOrder(m_orderId++, exitContract, exitOrder);
}


void CapstoneCppClient::optionsOperations()//REQUEST INITAL SPX INFORMATION TO GET CONID
{
	printf("Running Options OPs \n");
	Contract spxwContract;
	spxwContract.symbol = "SPX";
	spxwContract.secType = "IND";
	spxwContract.exchange = "CBOE";
	spxwContract.currency = "USD";

	m_pClient->reqContractDetails(1, spxwContract);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	m_state = ST_OPTIONSOPERATIONS;
}

void CapstoneCppClient::getComboOrder() { // GET INFORMATION FROM USER FOR COMBO ORDER

	Message msg = pendingOrderMap[currentOrderFromMap];
	pendingOrderMap.erase(currentOrderFromMap);

	auto now = std::chrono::system_clock::now();

	auto expirationDateLeg1 = now + std::chrono::hours(24 * msg.frontDTE); // Add days to current date
	std::time_t expirationTimeLeg1 = std::chrono::system_clock::to_time_t(expirationDateLeg1);
	std::tm expirationTmLeg1 = *std::localtime(&expirationTimeLeg1);

	char bufferLeg1[9]; // YYYYMMDD + null terminator
	std::strftime(bufferLeg1, sizeof(bufferLeg1), "%Y%m%d", &expirationTmLeg1);
	userDTELeg1 = bufferLeg1;

	auto expirationDateLeg2 = now + std::chrono::hours(24 * msg.backDTE); // Add days to current date
	std::time_t expirationTimeLeg2 = std::chrono::system_clock::to_time_t(expirationDateLeg2);
	std::tm expirationTmLeg2 = *std::localtime(&expirationTimeLeg2);

	char bufferLeg2[9]; // YYYYMMDD + null terminator
	std::strftime(bufferLeg2, sizeof(bufferLeg2), "%Y%m%d", &expirationTmLeg2);
	userDTELeg2 = bufferLeg2;

	printf("Leg 1 EXP Date: %s \n", userDTELeg1.c_str());
	printf("Leg 2 EXP Date: %s \n", userDTELeg2.c_str());

	userStrikePriceLeg1 = msg.frontStrikeChangeAmt;
	userStrikePriceLeg1 = spxCurrentPrice + userStrikePriceLeg1;
	userStrikePriceLeg1 = 5 * floor(abs(userStrikePriceLeg1 / 5));

	printf("Leg 1 Using Strike of: %f \n", userStrikePriceLeg1);

	userCallOrPutLeg1 = wstringToString(msg.frontOption).c_str();
	actionLeg1 = wstringToString(msg.frontAction).c_str();

	userStrikePriceLeg2 = userStrikePriceLeg1 + msg.backStrikeChangeAmt;
	/*userStrikePriceLeg2 = msg.backStrikeChangeAmt;
	userStrikePriceLeg2 = spxCurrentPrice + userStrikePriceLeg2;
	userStrikePriceLeg2 = 5 * floor(abs(userStrikePriceLeg2 / 5)); */

	printf("Leg 2 Using Strike of: %f \n", userStrikePriceLeg2);

	userCallOrPutLeg2 = wstringToString(msg.backOption).c_str();
	actionLeg2 = wstringToString(msg.backAction).c_str();

	printf("Call or Put Value leg 1: %s\n", userCallOrPutLeg1.c_str());
	printf("Action Leg1: %ls\n", msg.frontAction.c_str());

	printf("Call or Put Value leg 2: %s\n", userCallOrPutLeg2.c_str());
	printf("Action Leg2: %ls\n\n", msg.backAction.c_str());
	
	if (msg.orderType == L"%") {
		usePercentage = true;
	}
	else if(msg.orderType == L"#") {
		usePercentage = false;
	}

	takeProfitValue = msg.takeProfit;// HARD CODED FOR NOW NEED TO GET THIS FROM GUI
	stopLossValue = msg.stopLoss;

	amountOfLots = DecimalFunctions::doubleToDecimal(msg.lots);

	/////////////////////////////////////////////////////////
	Contract optionContract1;
	optionContract1.symbol = "SPX";
	optionContract1.secType = "OPT";
	optionContract1.exchange = "CBOE";
	optionContract1.currency = "USD";
	optionContract1.lastTradeDateOrContractMonth = userDTELeg1;  // Expiration date (YYYYMMDD)
	optionContract1.strike = userStrikePriceLeg1;  // Example strike price
	optionContract1.right = userCallOrPutLeg1;  // "C" for Call, "P" for Put
	optionContract1.multiplier = "100";
	optionContract1.tradingClass = "SPXW";

	Contract optionContract2;
	optionContract2.symbol = "SPX";
	optionContract2.secType = "OPT";
	optionContract2.exchange = "CBOE";
	optionContract2.currency = "USD";
	optionContract2.lastTradeDateOrContractMonth = userDTELeg2;  // Expiration date (YYYYMMDD)
	optionContract2.strike = userStrikePriceLeg2;  // Example strike price
	optionContract2.right = userCallOrPutLeg2;  // "C" for Call, "P" for Put
	optionContract2.multiplier = "100";
	optionContract2.tradingClass = "SPXW";

	m_pClient->reqContractDetails(1000, optionContract1);
	std::this_thread::sleep_for(std::chrono::seconds(1));//////
	m_pClient->reqContractDetails(1001, optionContract2);
	std::this_thread::sleep_for(std::chrono::seconds(1));//////


	m_state = ST_COMBOPRICE;
}

void CapstoneCppClient::getComboPrices() {
	m_pClient->reqMktData(5, ContractSamples::SPXComboContract(legOneConId,actionLeg1 ,legTwoConId, actionLeg2), "", false, false, TagValueListSPtr());
	std::this_thread::sleep_for(std::chrono::seconds(2));//////
	m_pClient->cancelMktData(5);

	m_state = ST_COMBOORDER;
}


void CapstoneCppClient::placeComboOrder() {

	printf("In Combo Order Method\n\n");
	std::this_thread::sleep_for(std::chrono::seconds(1));//////

	Contract comboContract = ContractSamples::SPXComboContract(legOneConId,actionLeg1,legTwoConId,actionLeg2);

	comboLimitPrice = round(((currentBidPrice + currentAskPrice) / 2) / 0.05) * 0.05;
	printf("Using a Limit Price of: %f\n", comboLimitPrice);
	if (currentBidPrice == 0.00 || currentAskPrice == 0) {
		m_state = ST_COMBOPRICE;
		return;
	}

	if (1 == 1) { // MAKE TRUE FOR NOW

		double takeProfitPrice;
		double stopLossPrice;

		if (usePercentage) {
			takeProfitPrice = comboLimitPrice * (1 + (takeProfitValue/100));
			takeProfitPrice = round(takeProfitPrice / 0.05) * 0.05;
			stopLossPrice = comboLimitPrice * (1 - (stopLossValue/100));
			stopLossPrice = round(stopLossPrice / 0.05) * 0.05;
		}
		else if (!usePercentage) {
			takeProfitPrice = comboLimitPrice + takeProfitValue;
			stopLossPrice = comboLimitPrice - stopLossValue;
		}


		printf("Take Profit Set to %f\n",takeProfitPrice);
		printf("Stop Loss Set to %f\n",stopLossPrice);

		//std::this_thread::sleep_for(std::chrono::seconds(10));

		Order parent;
		Order takeProfit;
		Order stopLoss;
	
		currentOrderID = m_orderId;
		contractMap[m_orderId] = comboContract;
		OrderSamples::BracketOrder(m_orderId, parent, takeProfit, stopLoss, "BUY", amountOfLots, comboLimitPrice, takeProfitPrice, stopLossPrice);
		m_pClient->placeOrder(parent.orderId, comboContract, parent);
		m_pClient->placeOrder(takeProfit.orderId, comboContract, takeProfit);
		m_pClient->placeOrder(stopLoss.orderId, comboContract, stopLoss);
		m_orderId = stopLoss.orderId + 1;
	}
	else {
		Order comboOrder;
		comboOrder.action = "BUY";             
		comboOrder.orderType = "LMT";          
		comboOrder.totalQuantity = amountOfLots; // Quantity for the combo order
		comboOrder.lmtPrice = comboLimitPrice;           
		comboOrder.transmit = true;

		currentOrderID = m_orderId;
		contractMap[m_orderId] = comboContract;
		m_pClient->placeOrder(m_orderId++, comboContract, comboOrder);
	}
	

	isComboCheck = true;
	m_state = ST_COMBOORDER_ACK;
}

void CapstoneCppClient::getAllExecutions() {
	m_pClient->reqExecutions(10001, ExecutionFilter());
	std::cout << "Requested execution data" << std::endl;
	m_state = ST_PING;
}

//! [nextvalidid]
void CapstoneCppClient::nextValidId(OrderId orderId)
{
	printf("Next Valid Id: %ld\n", orderId);
	m_orderId = orderId;
	//! [nextvalidid]
	m_state = ST_WAITFORINPUT;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TWS SAMPLE METHOD BELOW
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// methods
//! [connectack]
void CapstoneCppClient::connectAck() {
	if (!m_extraAuth && m_pClient->asyncEConnect())
        m_pClient->startApi();
}
//! [connectack]

void CapstoneCppClient::reqCurrentTime()
{
	printf( "Requesting Current Time\n");

	// set ping deadline to "now + n seconds"
	m_sleepDeadline = time( NULL) + PING_DEADLINE;

	m_state = ST_PING_ACK;

	m_pClient->reqCurrentTime();
}

void CapstoneCppClient::currentTime( long time)
{
	if ( m_state == ST_PING_ACK) {
		time_t t = ( time_t)time;
        struct tm timeinfo;
        char currentTime[80];

#if defined(IB_WIN32)
        localtime_s(&timeinfo, &t);
        asctime_s(currentTime, sizeof currentTime, &timeinfo);
#else
        localtime_r(&t, &timeinfo);
        asctime_r(&timeinfo, currentTime);
#endif
        printf( "The current date/time is: %s", currentTime);

		time_t now = ::time(NULL);
		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

		m_state = ST_PING_ACK;
	}
}

///////////////////////////////////////////////////////////////////////////
//EVERYTHING BELOW IS ALL THE CALLBACK FUNCTIONS FOR VARIOUS API CALLS
///////////////////////////////////////////////////////////////////////////

//! [contractdetails]
void CapstoneCppClient::contractDetails(int reqId, const ContractDetails& contractDetails) {

	if (reqId == 1000 || reqId == 1001) {
		std::cout << "Received Contract Details for " << contractDetails.contract.symbol << "\n";
		std::cout << "ConId: " << contractDetails.contract.conId << ", Strike: " << contractDetails.contract.strike
			<< ", Expiry: " << contractDetails.contract.lastTradeDateOrContractMonth
			<< ", Type: " << contractDetails.contract.right << "\n";

		if (reqId == 1000) {
			legOneConId = contractDetails.contract.conId;
		}
		else if (reqId == 1001) {
			legTwoConId = contractDetails.contract.conId;
		}

	}
	else {
		//printf("ContractDetails begin. ReqId: %d\n", reqId);
		//printContractMsg(contractDetails.contract);
		//printContractDetailsMsg(contractDetails);
		//printf("ContractDetails end. ReqId: %d\n", reqId);

		underlyingConId = contractDetails.contract.conId;
	}
}
//! [contractdetails]


//! [contractdetailsend]
void CapstoneCppClient::contractDetailsEnd(int reqId) {
	//printf("ContractDetailsEnd. %d\n", reqId);
	std::cout << "Retrieved SPX underlying conId: " << underlyingConId << std::endl;

	if (reqId != 1000 && reqId != 1001) {
		if (underlyingConId > 0) {
			getIndexValue = true;

			Contract contract;
			contract.symbol = "SPX";
			contract.secType = "IND";
			contract.exchange = "CBOE";
			contract.currency = "USD";
			contract.conId = underlyingConId;

			m_pClient->reqMarketDataType(1); //REMOVE WHEN TESTIG WITH PROF
			std::this_thread::sleep_for(std::chrono::seconds(1));
			m_pClient->reqMktData(2, contract, "", false, false, TagValueListSPtr());
			std::this_thread::sleep_for(std::chrono::seconds(2));
			m_pClient->cancelMktData(2);

		}
		else {
			std::cerr << "Invalid conId for SPX. Please check the contract details." << std::endl;
		}
	}
}
//! [contractdetailsend]

//! [tickprice]
void CapstoneCppClient::tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {
	printf("FIELD VALUE:%d \n", (int)field);
	//printf("BOOL VALUE:%d \n", getIndexValue);

	if (getIndexValue) {//DEBUG STATEMENT
		//printf("Tick Price. Ticker Id: %ld, Field: %d, Price: %s, CanAutoExecute: %d, PastLimit: %d, PreOpen: %d\n", tickerId, (int)field, Utils::doubleMaxString(price).c_str(), attribs.canAutoExecute, attribs.pastLimit, attribs.preOpen);
		//printf("FIELD VALUE:%d \n", (int)field);
		//printf("BOOL VALUE:%d \n", getIndexValue);
	}


	if (getIndexValue && (int)field == 9) { //==68 for Juliano account, == 4 for Prof account
		spxCurrentPrice = (int)price;
		getIndexValue = false;
		std::cout << "SPX Index Value (Current): " << spxCurrentPrice << std::endl;
		m_state = ST_COMBOINFO;
		return;
	}
	else if(!getIndexValue) {
		if ((int)field == 1) { //==66 for Juliano account, == 1 for Prof account
			std::cout << "Bid: " << price << std::endl;
			currentBidPrice = price;

		}
		else if ((int)field == 2) { //==67 for Juliano account, == 2 for Prof account
			std::cout << "Ask: " << price << std::endl;
			currentAskPrice = price;
		}
		else if ((int)field == 9) { //==68 for Juliano account, == 9 for Prof account
			std::cout << "Market Price (Last): " << price << std::endl;
			//currentMarketPrice = price;
		}
		else if ((int)field == 14) {
			//std::cout << "Market Price (Current): " << price << std::endl;
			//currentMarketPrice = price;
		}
		else if ((int)field == 4) {
			std::cout << "Market Price (Current): " << price << std::endl;
			currentMarketPrice = price;
		}

		printf("\n\n");

	}

	if (tickerId >= 2000 && (int)field == 2 && isComboCheck) { //==67 for Juliano account, == 2 for Prof account

		printf("Recgonized request ID of > 2000\n");
		
		if (comboLimitPrice <= price) { //THIS IS TEST FOR SINGLE CHANGE TO 0.05 FOR CALENDER

			double newLimit = comboLimitPrice + 0.05;
			printf("Adjusting Limit Price from: %f to: %f\n", comboLimitPrice, newLimit);
			comboLimitPrice = newLimit;

			double newTakeProfitPrice;
			double newStopLossPrice;

			if (usePercentage) {
				newTakeProfitPrice = comboLimitPrice * (1 + (takeProfitValue/100));
				newTakeProfitPrice = round(newTakeProfitPrice / 0.05) * 0.05;
				newStopLossPrice = comboLimitPrice * (1 - (stopLossValue/100));
				newStopLossPrice = round(newStopLossPrice / 0.05) * 0.05;
			}
			else if (!usePercentage) {
				newTakeProfitPrice = comboLimitPrice + takeProfitValue;
				newStopLossPrice = comboLimitPrice - stopLossValue;
			}

			Contract contractToAdjust = contractMap[currentOrderID];

			OrderCancel oc;
			oc.manualOrderCancelTime = "";
			m_pClient->cancelOrder(currentOrderID, oc);    // Cancel parent
			m_pClient->cancelOrder(currentOrderID + 1, oc); // Cancel take profit
			m_pClient->cancelOrder(currentOrderID + 2 ,oc); // Cancel stop loss

			Order adjustedParent;
			Order adjustedTakeProfit;
			Order adjustedStopLoss;

			OrderSamples::BracketOrder(m_orderId,
				adjustedParent,
				adjustedTakeProfit,
				adjustedStopLoss,
				"BUY",
				amountOfLots,
				newLimit,
				newTakeProfitPrice,
				newStopLossPrice);

			m_pClient->placeOrder(adjustedParent.orderId, contractToAdjust, adjustedParent);
			m_pClient->placeOrder(adjustedTakeProfit.orderId, contractToAdjust, adjustedTakeProfit);
			m_pClient->placeOrder(adjustedStopLoss.orderId, contractToAdjust, adjustedStopLoss);

			contractMap[adjustedParent.orderId] = contractToAdjust;
			currentOrderID = adjustedParent.orderId;  
			comboLimitPrice = newLimit;

			printf("New orders placed with IDs: Parent=%d, TP=%d, SL=%d\n",
				adjustedParent.orderId,
				adjustedTakeProfit.orderId,
				adjustedStopLoss.orderId);

			m_orderId = adjustedStopLoss.orderId + 1;
			m_state = ST_ADJUSTORDER;
			isComboCheck = false;
			std::this_thread::sleep_for(std::chrono::seconds(7));//TESTING
			return;
		}

		// CHANGE IF NEEDED
		m_state = ST_WAITFORINPUT;
	} 

}
//! [tickprice]

//! [orderstatus]
void CapstoneCppClient::orderStatus(OrderId orderId, const std::string& status, Decimal filled,
	Decimal remaining, double avgFillPrice, long long permId, int parentId,
	double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {

	printf("\n\nOrderStatus. Id: %ld, Status: %s, Filled: %s, Remaining: %s, AvgFillPrice: %s, PermId: %s, LastFillPrice: %s, ClientId: %s, WhyHeld: %s, MktCapPrice: %s\n\n",
		orderId, status.c_str(), DecimalFunctions::decimalStringToDisplay(filled).c_str(), DecimalFunctions::decimalStringToDisplay(remaining).c_str(), Utils::doubleMaxString(avgFillPrice).c_str(), Utils::llongMaxString(permId).c_str(),
		Utils::doubleMaxString(lastFillPrice).c_str(), Utils::intMaxString(clientId).c_str(), whyHeld.c_str(), Utils::doubleMaxString(mktCapPrice).c_str());
	if (status == "Filled") {
		isComboCheck = false;
		m_state = ST_WAITFORINPUT;
	}

	if (contractMap.find(orderId) != contractMap.end()) {
		Contract contract = contractMap[orderId];

		if (parentId == 0 && !DecimalFunctions::decimalToDouble(remaining) == 0.0 && (status == "Submitted" || status == "PartiallyFilled")) {//|| status == "PreSubmitted")) { //PreSubmitted is for Testing this will be removed
			// Request market data to get the current bid/ask prices
			m_pClient->reqMktData(postOrderTickID, contract, "", false, false, TagValueListSPtr());
			isComboCheck = true;
			//std::this_thread::sleep_for(std::chrono::seconds(1));
			m_pClient->cancelMktData(postOrderTickID++);
		}

	}


}
//! [orderstatus]

///////////////
//! [ticksize]
void CapstoneCppClient::tickSize(TickerId tickerId, TickType field, Decimal size) {
	//printf("Tick Size. Ticker Id: %ld, Field: %d, Size: %s\n", tickerId, (int)field, DecimalFunctions::decimalStringToDisplay(size).c_str());
}
//! [ticksize]

//! [tickoptioncomputation]
void CapstoneCppClient::tickOptionComputation(TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
	double optPrice, double pvDividend,
	double gamma, double vega, double theta, double undPrice) {
	//printf("TickOptionComputation. Ticker Id: %ld, Type: %d, TickAttrib: %s, ImpliedVolatility: %s, Delta: %s, OptionPrice: %s, pvDividend: %s, Gamma: %s, Vega: %s, Theta: %s, Underlying Price: %s\n",
	//	tickerId, (int)tickType, Utils::intMaxString(tickAttrib).c_str(), Utils::doubleMaxString(impliedVol).c_str(), Utils::doubleMaxString(delta).c_str(), Utils::doubleMaxString(optPrice).c_str(),
	//	Utils::doubleMaxString(pvDividend).c_str(), Utils::doubleMaxString(gamma).c_str(), Utils::doubleMaxString(vega).c_str(), Utils::doubleMaxString(theta).c_str(), Utils::doubleMaxString(undPrice).c_str());
}
//! [tickoptioncomputation]

//! [tickstring]
void CapstoneCppClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
	//printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
}
//! [tickstring]

//! [tickgeneric]
void CapstoneCppClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {
	//printf("Tick Generic. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, Utils::doubleMaxString(value).c_str());
}
//! [tickgeneric]

//! [marketdatatype]
void CapstoneCppClient::marketDataType(TickerId reqId, int marketDataType) {
	//printf("MarketDataType. ReqId: %ld, Type: %d\n", reqId, marketDataType);
}
//! [marketdatatype]

//! [tickReqParams]
void CapstoneCppClient::tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) {
	//printf("tickerId: %d, minTick: %s, bboExchange: %s, snapshotPermissions: %u\n", tickerId, Utils::doubleMaxString(minTick).c_str(), bboExchange.c_str(), snapshotPermissions);

	m_bboExchange = bboExchange;
}
//! [tickReqParams]

//! [openorder]
void CapstoneCppClient::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
	/*printf("OpenOrder. PermId: %s, ClientId: %s, OrderId: %s, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType:%s, TotalQty: %s, CashQty: %s, "
		"LmtPrice: %s, AuxPrice: %s, Status: %s, MinTradeQty: %s, MinCompeteSize: %s, CompeteAgainstBestOffset: %s, MidOffsetAtWhole: %s, MidOffsetAtHalf: %s, "
		"FAGroup: %s, FAMethod: %s, CustomerAccount: %s, ProfessionalCustomer: %s, BondAccruedInterest: %s, IncludeOvernight: %s, ExtOperator:%s, ManualOrderIndicator: %s\n",
		Utils::llongMaxString(order.permId).c_str(), Utils::longMaxString(order.clientId).c_str(), Utils::longMaxString(orderId).c_str(), order.account.c_str(), contract.symbol.c_str(),
		contract.secType.c_str(), contract.exchange.c_str(), order.action.c_str(), order.orderType.c_str(), DecimalFunctions::decimalStringToDisplay(order.totalQuantity).c_str(),
		Utils::doubleMaxString(order.cashQty).c_str(), Utils::doubleMaxString(order.lmtPrice).c_str(), Utils::doubleMaxString(order.auxPrice).c_str(), orderState.status.c_str(),
		Utils::intMaxString(order.minTradeQty).c_str(), Utils::intMaxString(order.minCompeteSize).c_str(),
		order.competeAgainstBestOffset == COMPETE_AGAINST_BEST_OFFSET_UP_TO_MID ? "UpToMid" : Utils::doubleMaxString(order.competeAgainstBestOffset).c_str(),
		Utils::doubleMaxString(order.midOffsetAtWhole).c_str(), Utils::doubleMaxString(order.midOffsetAtHalf).c_str(),
		order.faGroup.c_str(), order.faMethod.c_str(), order.customerAccount.c_str(), (order.professionalCustomer ? "true" : "false"), order.bondAccruedInterest.c_str(),
		(order.includeOvernight ? "true" : "false"), order.extOperator.c_str(), Utils::intMaxString(order.manualOrderIndicator).c_str()); */
}
//! [openorder]

//! [openorderend]
void CapstoneCppClient::openOrderEnd() {
	printf("OpenOrderEnd\n");
}
//! [openorderend]

//! [execdetails]
void CapstoneCppClient::execDetails(int reqId, const Contract& contract, const Execution& execution) {

	std::string orderIdStr = Utils::longMaxString(execution.orderId);
	double shares = DecimalFunctions::decimalToDouble(execution.shares);

	// Print execution details
	printf("ExecDetails: reqId=%d, execId=%s, orderId=%s, symbol=%s, side=%s, shares=%.2f, price=%.2f, time=%s\n",
		reqId,
		execution.execId.c_str(),
		orderIdStr.c_str(),
		contract.symbol.c_str(),
		execution.side.c_str(),
		shares, 
		execution.price,
		execution.time.c_str());

	// Save mapping from execId to orderId
	g_execIdToOrderId[execution.execId] = execution.orderId;

	/*// Check if execution is an entry or exit order
	auto it = g_tradeMap.find(execution.orderId);
	if (it == g_tradeMap.end()) {
		// Entry order
		TradeRecord record;
		record.symbol = contract.symbol;
		record.entryTime = execution.time;
		record.entryPrice = execution.price;
		record.shares = shares; 
		record.side = execution.side;
		record.exitTime = ""; 

		g_tradeMap[execution.orderId] = record;
	}
	else {

		TradeRecord& record = it->second;
		record.exitTime = execution.time;
		record.exitPrice = execution.price;

		double grossProfit = 0.0;
		if (record.side == "BUY") {
			grossProfit = (record.exitPrice - record.entryPrice) * record.shares;
		}
		else if (record.side == "SELL") {
			grossProfit = (record.entryPrice - record.exitPrice) * record.shares;
		}

		double commission = g_orderCommission[execution.orderId];
		double netProfit = grossProfit - commission;

		appendTradeRecordToCsv(record, grossProfit, commission, netProfit);

		g_tradeMap.erase(it);
		g_orderCommission.erase(execution.orderId);
	}*/
	std::string currentSide = execution.side;
	auto matchOpenTrade = [&](const TradeRecord& tr) {
		return tr.symbol == contract.symbol &&
			((currentSide == "BUY" && tr.side == "SELL") ||  // Closing short
				(currentSide == "SELL" && tr.side == "BUY"));   // Closing long
	};

	auto it = std::find_if(openTrades.begin(), openTrades.end(), matchOpenTrade);

	if (it != openTrades.end()) {
		// Process as exit trade
		double closeAmount = min(shares, it->shares);
		double remainingShares = it->shares - closeAmount;

		// Calculate gross profit
		double grossProfit = 0.0;
		if (it->side == "BUY") { // Closing long
			grossProfit = (execution.price - it->entryPrice) * closeAmount;
		}
		else { // Closing short
			grossProfit = (it->entryPrice - execution.price) * closeAmount;
		}

		double entryCommission = g_orderCommission[it->entryOrderId];
		double exitCommission = g_orderCommission[execution.orderId];
		double netProfit = grossProfit - entryCommission - exitCommission;

		TradeRecord closedRecord = *it;
		closedRecord.exitTime = execution.time;
		closedRecord.exitPrice = execution.price;
		closedRecord.exitOrderId = execution.orderId;
		closedRecord.shares = closeAmount;

		appendTradeRecordToCsv(closedRecord, grossProfit,
			entryCommission + exitCommission, netProfit);

		if (remainingShares > 0) {
			it->shares = remainingShares;
		}
		else {
			openTrades.erase(it);
		}
	}
	else {
		TradeRecord newTrade;
		newTrade.symbol = contract.symbol;
		newTrade.entryTime = execution.time;
		newTrade.entryPrice = execution.price;
		newTrade.shares = shares;
		newTrade.side = execution.side;
		newTrade.entryOrderId = execution.orderId;
		newTrade.exitOrderId = -1;
		openTrades.push_back(newTrade);
	}
}
//! [execdetails]

//! [execdetailsend]
void CapstoneCppClient::execDetailsEnd(int reqId) {
	printf("We have hit ExecDetailsEnd. %d\n", reqId);
}
//! [execdetailsend]

//! [commissionreport]
void CapstoneCppClient::commissionReport(const CommissionReport& commissionReport) {
	//printf("CommissionReport. %s - %s %s RPNL %s\n", commissionReport.execId.c_str(), Utils::doubleMaxString(commissionReport.commission).c_str(), commissionReport.currency.c_str(), Utils::doubleMaxString(commissionReport.realizedPNL).c_str());
	printf("CommissionReport: execId=%s, commission=%.2f, currency=%s, realizedPNL=%.2f\n",
		commissionReport.execId.c_str(),
		commissionReport.commission,
		commissionReport.currency.c_str(),
		commissionReport.realizedPNL);

	// Look up the corresponding orderId via the execId mapping.
	auto it = g_execIdToOrderId.find(commissionReport.execId);
	if (it != g_execIdToOrderId.end()) {
		int orderId = it->second;
		// Sum commission amounts if there are multiple reports for a given order.
		g_orderCommission[orderId] += commissionReport.commission;
	}
}
//! [commissionreport]
 
 //! [error]
void CapstoneCppClient::error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson)
{
	if (!advancedOrderRejectJson.empty()) {
		printf("Error. Id: %d, Code: %d, Msg: %s, AdvancedOrderRejectJson: %s\n", id, errorCode, errorString.c_str(), advancedOrderRejectJson.c_str());
	}
	else {
		printf("Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());
	}
}
//! [error]

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapstoneCppClient::historicalDataRequests()
{
	/*** Requesting historical data ***/
	//! [reqhistoricaldata]
	std::time_t rawtime;
	std::tm timeinfo;
	char queryTime[80];

	std::time(&rawtime);
#if defined(IB_WIN32)
	gmtime_s(&timeinfo, &rawtime);
#else
	gmtime_r(&rawtime, &timeinfo);
#endif
	std::strftime(queryTime, sizeof queryTime, "%Y%m%d-%H:%M:%S", &timeinfo);

	m_pClient->reqHistoricalData(4001, ContractSamples::EurGbpFx(), queryTime, "1 M", "1 day", "MIDPOINT", 1, 1, false, TagValueListSPtr());
	m_pClient->reqHistoricalData(4002, ContractSamples::EuropeanStock(), queryTime, "10 D", "1 min", "TRADES", 1, 1, false, TagValueListSPtr());
	m_pClient->reqHistoricalData(4003, ContractSamples::USStockAtSmart(), queryTime, "1 M", "1 day", "SCHEDULE", 1, 1, false, TagValueListSPtr());
	//! [reqhistoricaldata]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Canceling historical data requests ***/
	m_pClient->cancelHistoricalData(4001);
	m_pClient->cancelHistoricalData(4002);
	m_pClient->cancelHistoricalData(4003);

	m_state = ST_HISTORICALDATAREQUESTS_ACK;
}

void CapstoneCppClient::orderOperations()
{
	/*** Requesting the next valid id ***/
	//! [reqids]
	//The parameter is always ignored.
	m_pClient->reqIds(-1);
	//! [reqids]
	//! [reqallopenorders]
	m_pClient->reqAllOpenOrders();
	//! [reqallopenorders]
	//! [reqautoopenorders]
	m_pClient->reqAutoOpenOrders(true);
	//! [reqautoopenorders]
	//! [reqopenorders]
	m_pClient->reqOpenOrders();
	//! [reqopenorders]

	/*** Placing/modifying an order - remember to ALWAYS increment the nextValidId after placing an order so it can be used for the next one! ***/
	//! [order_submission]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::LimitOrder("SELL", DecimalFunctions::stringToDecimal("1"), 50));
	//! [order_submission]

	//! [place_midprice]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::Midprice("BUY", DecimalFunctions::stringToDecimal("1"), 150));
	//! [place_midprice]

	//! [place order with cashQty]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::LimitOrderWithCashQty("BUY", 111.11, 5000));
	//! [place order with cashQty]

	std::this_thread::sleep_for(std::chrono::seconds(1));

	/*** Cancel one order ***/
	//! [cancelorder]
	m_pClient->cancelOrder(m_orderId - 1, OrderSamples::OrderCancelEmpty());
	//! [cancelorder]

	/*** Cancel all orders for all accounts ***/
	//! [reqglobalcancel]
	m_pClient->reqGlobalCancel(OrderSamples::OrderCancelEmpty());
	//! [reqglobalcancel]

	/*** Request the day's executions ***/
	//! [reqexecutions]
	m_pClient->reqExecutions(10001, ExecutionFilter());
	//! [reqexecutions]

	//! [reqcompletedorders]
	m_pClient->reqCompletedOrders(false);
	//! [reqcompletedorders]

	//! [order_submission]
	m_pClient->placeOrder(m_orderId++, ContractSamples::CryptoContract(), OrderSamples::LimitOrder("BUY", DecimalFunctions::stringToDecimal("0.12345678"), 3700));
	//! [order_submission]

	//! [manual_order_time]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::LimitOrderWithManualOrderTime("BUY", DecimalFunctions::stringToDecimal("100"), 111.11, "20220314-13:00:00"));
	//! [manual_order_time]

	//! [manual_order_cancel_time]
	m_pClient->cancelOrder(m_orderId - 1, OrderSamples::OrderCancelWithManualTime("20240614-00:00:05"));
	//! [manual_order_cancel_time]

	//! [pegbest_up_to_mid_order_submission]
	m_pClient->placeOrder(m_orderId++, ContractSamples::IBKRATSContract(), OrderSamples::PegBestUpToMidOrder("BUY", DecimalFunctions::stringToDecimal("100"), 111.11, 100, 200, 0.02, 0.025));
	//! [pegbest_up_to_mid_order_submission]

	//! [pegbest_order_submission]
	m_pClient->placeOrder(m_orderId++, ContractSamples::IBKRATSContract(), OrderSamples::PegBestOrder("BUY", DecimalFunctions::stringToDecimal("100"), 111.11, 100, 200, 0.03));
	//! [pegbest_order_submission]

	//! [pegmid_order_submission]
	m_pClient->placeOrder(m_orderId++, ContractSamples::IBKRATSContract(), OrderSamples::PegMidOrder("BUY", DecimalFunctions::stringToDecimal("100"), 111.11, 100, 0.02, 0.025));
	//! [pegmid_order_submission]

	//! [customer_account]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::LimitOrderWithCustomerAccount("BUY", DecimalFunctions::stringToDecimal("100"), 111.11, "CustAcct"));
	//! [customer_account]

	//! [include_overnight]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::LimitOrderWithIncludeOvernight("BUY", DecimalFunctions::stringToDecimal("100"), 111.11));
	//! [include_overnight]

	//! [cme_tagging_fields]
	m_pClient->placeOrder(m_orderId++, ContractSamples::SimpleFuture(), OrderSamples::LimitOrderWithCmeTaggingFields("BUY", DecimalFunctions::stringToDecimal("1"), 5333, "ABCD", 1));
	std::this_thread::sleep_for(std::chrono::seconds(5));
	m_pClient->cancelOrder(m_orderId - 1, OrderSamples::OrderCancelWithCmeTaggingFields("BCDE", 0));
	std::this_thread::sleep_for(std::chrono::seconds(2));
	m_pClient->placeOrder(m_orderId++, ContractSamples::SimpleFuture(), OrderSamples::LimitOrderWithCmeTaggingFields("BUY", DecimalFunctions::stringToDecimal("1"), 5444, "CDEF", 0));
	std::this_thread::sleep_for(std::chrono::seconds(5));
	m_pClient->reqGlobalCancel(OrderSamples::OrderCancelWithCmeTaggingFields("DEFG", 1));
	//! [cme_tagging_fields]

	m_state = ST_ORDEROPERATIONS_ACK;
}

void CapstoneCppClient::ocaSamples() // useful sample
{
	//OCA ORDER
	//! [ocasubmit]
	std::vector<Order> ocaOrders;
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", DecimalFunctions::stringToDecimal("1"), 10));
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", DecimalFunctions::stringToDecimal("1"), 11));
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", DecimalFunctions::stringToDecimal("1"), 12));
	for (unsigned int i = 0; i < ocaOrders.size(); i++) {
		OrderSamples::OneCancelsAll("TestOca", ocaOrders[i], 2);
		m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), ocaOrders[i]);
	}
	//! [ocasubmit]

	m_state = ST_OCASAMPLES_ACK;
}

//USEFUL EXMAPLE ABOVE AND BELOW

void CapstoneCppClient::conditionSamples()
{
	//! [order_conditioning_activate]
	Order lmt = OrderSamples::LimitOrder("BUY", DecimalFunctions::stringToDecimal("100"), 10);
	//Order will become active if conditioning criteria is met
	PriceCondition* priceCondition = dynamic_cast<PriceCondition*>(OrderSamples::Price_Condition(208813720, "SMART", 600, false, false));
	ExecutionCondition* execCondition = dynamic_cast<ExecutionCondition*>(OrderSamples::Execution_Condition("EUR.USD", "CASH", "IDEALPRO", true));
	MarginCondition* marginCondition = dynamic_cast<MarginCondition*>(OrderSamples::Margin_Condition(30, true, false));
	PercentChangeCondition* pctChangeCondition = dynamic_cast<PercentChangeCondition*>(OrderSamples::Percent_Change_Condition(15.0, 208813720, "SMART", true, true));
	TimeCondition* timeCondition = dynamic_cast<TimeCondition*>(OrderSamples::Time_Condition("20220808 10:00:00 US/Eastern", true, false));
	VolumeCondition* volumeCondition = dynamic_cast<VolumeCondition*>(OrderSamples::Volume_Condition(208813720, "SMART", false, 100, true));

	lmt.conditions.push_back(std::shared_ptr<PriceCondition>(priceCondition));
	lmt.conditions.push_back(std::shared_ptr<ExecutionCondition>(execCondition));
	lmt.conditions.push_back(std::shared_ptr<MarginCondition>(marginCondition));
	lmt.conditions.push_back(std::shared_ptr<PercentChangeCondition>(pctChangeCondition));
	lmt.conditions.push_back(std::shared_ptr<TimeCondition>(timeCondition));
	lmt.conditions.push_back(std::shared_ptr<VolumeCondition>(volumeCondition));
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), lmt);
	//! [order_conditioning_activate]

	//Conditions can make the order active or cancel it. Only LMT orders can be conditionally canceled.
	//! [order_conditioning_cancel]
	Order lmt2 = OrderSamples::LimitOrder("BUY", DecimalFunctions::stringToDecimal("100"), 20);
	//The active order will be cancelled if conditioning criteria is met
	lmt2.conditionsCancelOrder = true;
	PriceCondition* priceCondition2 = dynamic_cast<PriceCondition*>(OrderSamples::Price_Condition(208813720, "SMART", 600, false, false));
	lmt2.conditions.push_back(std::shared_ptr<PriceCondition>(priceCondition2));
	m_pClient->placeOrder(m_orderId++, ContractSamples::EuropeanStock(), lmt2);
	//! [order_conditioning_cancel]

	m_state = ST_CONDITIONSAMPLES_ACK;
}

void CapstoneCppClient::bracketSample() { // USEFUL SAMPLE
	Order parent;
	Order takeProfit;
	Order stopLoss;
	//! [bracketsubmit]
	OrderSamples::BracketOrder(m_orderId++, parent, takeProfit, stopLoss, "BUY", DecimalFunctions::stringToDecimal("100"), 30, 40, 20);
	m_pClient->placeOrder(parent.orderId, ContractSamples::EuropeanStock(), parent);
	m_pClient->placeOrder(takeProfit.orderId, ContractSamples::EuropeanStock(), takeProfit);
	m_pClient->placeOrder(stopLoss.orderId, ContractSamples::EuropeanStock(), stopLoss);
	//! [bracketsubmit]

	m_state = ST_BRACKETSAMPLES_ACK;
}

void CapstoneCppClient::reqHistoricalTicks()
{
	//! [reqhistoricalticks]
	m_pClient->reqHistoricalTicks(19001, ContractSamples::IBMUSStockAtSmart(), "20170621 09:38:33 US/Eastern", "", 10, "BID_ASK", 1, true, TagValueListSPtr());
	m_pClient->reqHistoricalTicks(19002, ContractSamples::IBMUSStockAtSmart(), "20170621 09:38:33 US/Eastern", "", 10, "MIDPOINT", 1, true, TagValueListSPtr());
	m_pClient->reqHistoricalTicks(19003, ContractSamples::IBMUSStockAtSmart(), "20170621 09:38:33 US/Eastern", "", 10, "TRADES", 1, true, TagValueListSPtr());
	//! [reqhistoricalticks]
	m_state = ST_REQHISTORICALTICKS_ACK;
}

//USE BELOW
void CapstoneCppClient::reqTickByTickData()
{
	/*** Requesting tick-by-tick data (only refresh) ***/

	m_pClient->reqTickByTickData(20001, ContractSamples::EuropeanStock(), "Last", 0, false);
	m_pClient->reqTickByTickData(20002, ContractSamples::EuropeanStock(), "AllLast", 0, false);
	m_pClient->reqTickByTickData(20003, ContractSamples::EuropeanStock(), "BidAsk", 0, true);
	m_pClient->reqTickByTickData(20004, ContractSamples::EurGbpFx(), "MidPoint", 0, false);

	std::this_thread::sleep_for(std::chrono::seconds(10));

	//! [canceltickbytick]
	m_pClient->cancelTickByTickData(20001);
	m_pClient->cancelTickByTickData(20002);
	m_pClient->cancelTickByTickData(20003);
	m_pClient->cancelTickByTickData(20004);
	//! [canceltickbytick]

	/*** Requesting tick-by-tick data (historical + refresh) ***/
	//! [reqtickbytick]
	m_pClient->reqTickByTickData(20005, ContractSamples::EuropeanStock(), "Last", 10, false);
	m_pClient->reqTickByTickData(20006, ContractSamples::EuropeanStock(), "AllLast", 10, false);
	m_pClient->reqTickByTickData(20007, ContractSamples::EuropeanStock(), "BidAsk", 10, false);
	m_pClient->reqTickByTickData(20008, ContractSamples::EurGbpFx(), "MidPoint", 10, true);
	//! [reqtickbytick]

	std::this_thread::sleep_for(std::chrono::seconds(10));

	m_pClient->cancelTickByTickData(20005);
	m_pClient->cancelTickByTickData(20006);
	m_pClient->cancelTickByTickData(20007);
	m_pClient->cancelTickByTickData(20008);

	m_state = ST_REQTICKBYTICKDATA_ACK;
}







///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER TWS EXAMPLE CALLBACK FUCNTIONS WE MAY OR MAY NOT NEED
// ////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapstoneCppClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
                            double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
    printf( "TickEFP. %ld, Type: %d, BasisPoints: %s, FormattedBasisPoints: %s, Total Dividends: %s, HoldDays: %s, Future Last Trade Date: %s, Dividend Impact: %s, Dividends To Last Trade Date: %s\n", 
        tickerId, (int)tickType, Utils::doubleMaxString(basisPoints).c_str(), formattedBasisPoints.c_str(), Utils::doubleMaxString(totalDividends).c_str(), Utils::intMaxString(holdDays).c_str(), 
        futureLastTradeDate.c_str(), Utils::doubleMaxString(dividendImpact).c_str(), Utils::doubleMaxString(dividendsToLastTradeDate).c_str());
}

void CapstoneCppClient::winError( const std::string& str, int lastError) {}

void CapstoneCppClient::connectionClosed() {
	printf( "Connection Closed\n");
}

//! [updateaccountvalue]
void CapstoneCppClient::updateAccountValue(const std::string& key, const std::string& val,
                                       const std::string& currency, const std::string& accountName) {
	printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account Name: %s\n", key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
}
//! [updateaccountvalue]

//! [updateportfolio]
void CapstoneCppClient::updatePortfolio(const Contract& contract, Decimal position,
                                    double marketPrice, double marketValue, double averageCost,
                                    double unrealizedPNL, double realizedPNL, const std::string& accountName){
    printf("UpdatePortfolio. %s, %s @ %s: Position: %s, MarketPrice: %s, MarketValue: %s, AverageCost: %s, UnrealizedPNL: %s, RealizedPNL: %s, AccountName: %s\n", 
        (contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), DecimalFunctions::decimalStringToDisplay(position).c_str(),
        Utils::doubleMaxString(marketPrice).c_str(), Utils::doubleMaxString(marketValue).c_str(), Utils::doubleMaxString(averageCost).c_str(), 
        Utils::doubleMaxString(unrealizedPNL).c_str(), Utils::doubleMaxString(realizedPNL).c_str(), accountName.c_str());
}
//! [updateportfolio]

//! [updateaccounttime]
void CapstoneCppClient::updateAccountTime(const std::string& timeStamp) {
	printf( "UpdateAccountTime. Time: %s\n", timeStamp.c_str());
}
//! [updateaccounttime]

//! [accountdownloadend]
void CapstoneCppClient::accountDownloadEnd(const std::string& accountName) {
	printf( "Account download finished: %s\n", accountName.c_str());
}
//! [accountdownloadend]


//! [bondcontractdetails]
void CapstoneCppClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {
	printf( "BondContractDetails begin. ReqId: %d\n", reqId);
	printBondContractDetailsMsg(contractDetails);
	printf( "BondContractDetails end. ReqId: %d\n", reqId);
}
//! [bondcontractdetails]

void CapstoneCppClient::printContractMsg(const Contract& contract) {
	printf("\tConId: %ld\n", contract.conId);
	printf("\tSymbol: %s\n", contract.symbol.c_str());
	printf("\tSecType: %s\n", contract.secType.c_str());
	printf("\tLastTradeDateOrContractMonth: %s\n", contract.lastTradeDateOrContractMonth.c_str());
	printf("\tLastTradeDate: %s\n", contract.lastTradeDate.c_str());
	printf("\tStrike: %s\n", Utils::doubleMaxString(contract.strike).c_str());
	printf("\tRight: %s\n", contract.right.c_str());
	printf("\tMultiplier: %s\n", contract.multiplier.c_str());
	printf("\tExchange: %s\n", contract.exchange.c_str());
	printf("\tPrimaryExchange: %s\n", contract.primaryExchange.c_str());
	printf("\tCurrency: %s\n", contract.currency.c_str());
	printf("\tLocalSymbol: %s\n", contract.localSymbol.c_str());
	printf("\tTradingClass: %s\n", contract.tradingClass.c_str());
}

void CapstoneCppClient::printContractDetailsMsg(const ContractDetails& contractDetails) {
	printf("\tMarketName: %s\n", contractDetails.marketName.c_str());
	printf("\tMinTick: %s\n", Utils::doubleMaxString(contractDetails.minTick).c_str());
	printf("\tPriceMagnifier: %s\n", Utils::longMaxString(contractDetails.priceMagnifier).c_str());
	printf("\tOrderTypes: %s\n", contractDetails.orderTypes.c_str());
	printf("\tValidExchanges: %s\n", contractDetails.validExchanges.c_str());
	printf("\tUnderConId: %s\n", Utils::intMaxString(contractDetails.underConId).c_str());
	printf("\tLongName: %s\n", contractDetails.longName.c_str());
	printf("\tContractMonth: %s\n", contractDetails.contractMonth.c_str());
	printf("\tIndystry: %s\n", contractDetails.industry.c_str());
	printf("\tCategory: %s\n", contractDetails.category.c_str());
	printf("\tSubCategory: %s\n", contractDetails.subcategory.c_str());
	printf("\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
	printf("\tTradingHours: %s\n", contractDetails.tradingHours.c_str());
	printf("\tLiquidHours: %s\n", contractDetails.liquidHours.c_str());
	printf("\tEvRule: %s\n", contractDetails.evRule.c_str());
	printf("\tEvMultiplier: %s\n", Utils::doubleMaxString(contractDetails.evMultiplier).c_str());
	printf("\tAggGroup: %s\n", Utils::intMaxString(contractDetails.aggGroup).c_str());
	printf("\tUnderSymbol: %s\n", contractDetails.underSymbol.c_str());
	printf("\tUnderSecType: %s\n", contractDetails.underSecType.c_str());
	printf("\tMarketRuleIds: %s\n", contractDetails.marketRuleIds.c_str());
	printf("\tRealExpirationDate: %s\n", contractDetails.realExpirationDate.c_str());
	printf("\tLastTradeTime: %s\n", contractDetails.lastTradeTime.c_str());
	printf("\tStockType: %s\n", contractDetails.stockType.c_str());
	printf("\tMinSize: %s\n", DecimalFunctions::decimalStringToDisplay(contractDetails.minSize).c_str());
	printf("\tSizeIncrement: %s\n", DecimalFunctions::decimalStringToDisplay(contractDetails.sizeIncrement).c_str());
	printf("\tSuggestedSizeIncrement: %s\n", DecimalFunctions::decimalStringToDisplay(contractDetails.suggestedSizeIncrement).c_str());
	if (contractDetails.contract.secType == "FUND") {
		printf("\tFund Data: \n");
		printf("\t\tFundName: %s\n", contractDetails.fundName.c_str());
		printf("\t\tFundFamily: %s\n", contractDetails.fundFamily.c_str());
		printf("\t\tFundType: %s\n", contractDetails.fundType.c_str());
		printf("\t\tFundFrontLoad: %s\n", contractDetails.fundFrontLoad.c_str());
		printf("\t\tFundBackLoad: %s\n", contractDetails.fundBackLoad.c_str());
		printf("\t\tFundBackLoadTimeInterval: %s\n", contractDetails.fundBackLoadTimeInterval.c_str());
		printf("\t\tFundManagementFee: %s\n", contractDetails.fundManagementFee.c_str());
		printf("\t\tFundClosed: %s\n", contractDetails.fundClosed ? "yes" : "no");
		printf("\t\tFundClosedForNewInvestors: %s\n", contractDetails.fundClosedForNewInvestors ? "yes" : "no");
		printf("\t\tFundClosedForNewMoney: %s\n", contractDetails.fundClosedForNewMoney ? "yes" : "no");
		printf("\t\tFundNotifyAmount: %s\n", contractDetails.fundNotifyAmount.c_str());
		printf("\t\tFundMinimumInitialPurchase: %s\n", contractDetails.fundMinimumInitialPurchase.c_str());
		printf("\t\tFundSubsequentMinimumPurchase: %s\n", contractDetails.fundSubsequentMinimumPurchase.c_str());
		printf("\t\tFundBlueSkyStates: %s\n", contractDetails.fundBlueSkyStates.c_str());
		printf("\t\tFundBlueSkyTerritories: %s\n", contractDetails.fundBlueSkyTerritories.c_str());
		printf("\t\tFundDistributionPolicyIndicator: %s\n", Utils::getFundDistributionPolicyIndicatorName(contractDetails.fundDistributionPolicyIndicator).c_str());
		printf("\t\tFundAssetType: %s\n", Utils::getFundAssetTypeName(contractDetails.fundAssetType).c_str());
	}
	printContractDetailsSecIdList(contractDetails.secIdList);
	printContractDetailsIneligibilityReasonList(contractDetails.ineligibilityReasonList);
}

void CapstoneCppClient::printContractDetailsSecIdList(const TagValueListSPtr &secIdList) {
	const int secIdListCount = secIdList.get() ? secIdList->size() : 0;
	if (secIdListCount > 0) {
		printf("\tSecIdList: {");
		for (int i = 0; i < secIdListCount; ++i) {
			const TagValue* tagValue = ((*secIdList)[i]).get();
			printf("%s=%s;",tagValue->tag.c_str(), tagValue->value.c_str());
		}
		printf("}\n");
	}
}

void CapstoneCppClient::printContractDetailsIneligibilityReasonList(const IneligibilityReasonListSPtr &ineligibilityReasonList) {
	const int ineligibilityReasonListCount = ineligibilityReasonList.get() ? ineligibilityReasonList->size() : 0;
	if (ineligibilityReasonListCount > 0) {
		printf("\tIneligibilityReasonList: {");
		for (int i = 0; i < ineligibilityReasonListCount; ++i) {
			const IneligibilityReason* ineligibilityReason = ((*ineligibilityReasonList)[i]).get();
			printf("[id: %s, description: %s];", ineligibilityReason->id.c_str(), ineligibilityReason->description.c_str());
		}
		printf("}\n");
	}
}

void CapstoneCppClient::printBondContractDetailsMsg(const ContractDetails& contractDetails) {
	printf("\tSymbol: %s\n", contractDetails.contract.symbol.c_str());
	printf("\tSecType: %s\n", contractDetails.contract.secType.c_str());
	printf("\tCusip: %s\n", contractDetails.cusip.c_str());
	printf("\tCoupon: %s\n", Utils::doubleMaxString(contractDetails.coupon).c_str());
	printf("\tMaturity: %s\n", contractDetails.maturity.c_str());
	printf("\tIssueDate: %s\n", contractDetails.issueDate.c_str());
	printf("\tRatings: %s\n", contractDetails.ratings.c_str());
	printf("\tBondType: %s\n", contractDetails.bondType.c_str());
	printf("\tCouponType: %s\n", contractDetails.couponType.c_str());
	printf("\tConvertible: %s\n", contractDetails.convertible ? "yes" : "no");
	printf("\tCallable: %s\n", contractDetails.callable ? "yes" : "no");
	printf("\tPutable: %s\n", contractDetails.putable ? "yes" : "no");
	printf("\tDescAppend: %s\n", contractDetails.descAppend.c_str());
	printf("\tExchange: %s\n", contractDetails.contract.exchange.c_str());
	printf("\tCurrency: %s\n", contractDetails.contract.currency.c_str());
	printf("\tMarketName: %s\n", contractDetails.marketName.c_str());
	printf("\tTradingClass: %s\n", contractDetails.contract.tradingClass.c_str());
	printf("\tConId: %s\n", Utils::longMaxString(contractDetails.contract.conId).c_str());
	printf("\tMinTick: %s\n", Utils::doubleMaxString(contractDetails.minTick).c_str());
	printf("\tOrderTypes: %s\n", contractDetails.orderTypes.c_str());
	printf("\tValidExchanges: %s\n", contractDetails.validExchanges.c_str());
	printf("\tNextOptionDate: %s\n", contractDetails.nextOptionDate.c_str());
	printf("\tNextOptionType: %s\n", contractDetails.nextOptionType.c_str());
	printf("\tNextOptionPartial: %s\n", contractDetails.nextOptionPartial ? "yes" : "no");
	printf("\tNotes: %s\n", contractDetails.notes.c_str());
	printf("\tLong Name: %s\n", contractDetails.longName.c_str());
	printf("\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
	printf("\tTradingHours: %s\n", contractDetails.tradingHours.c_str());
	printf("\tLiquidHours: %s\n", contractDetails.liquidHours.c_str());
	printf("\tEvRule: %s\n", contractDetails.evRule.c_str());
	printf("\tEvMultiplier: %s\n", Utils::doubleMaxString(contractDetails.evMultiplier).c_str());
	printf("\tAggGroup: %s\n", Utils::intMaxString(contractDetails.aggGroup).c_str());
	printf("\tMarketRuleIds: %s\n", contractDetails.marketRuleIds.c_str());
	printf("\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
	printf("\tLastTradeTime: %s\n", contractDetails.lastTradeTime.c_str());
	printf("\tMinSize: %s\n", DecimalFunctions::decimalStringToDisplay(contractDetails.minSize).c_str());
	printf("\tSizeIncrement: %s\n", DecimalFunctions::decimalStringToDisplay(contractDetails.sizeIncrement).c_str());
	printf("\tSuggestedSizeIncrement: %s\n", DecimalFunctions::decimalStringToDisplay(contractDetails.suggestedSizeIncrement).c_str());
	printContractDetailsSecIdList(contractDetails.secIdList);
}


//! [updatemktdepth]
void CapstoneCppClient::updateMktDepth(TickerId id, int position, int operation, int side,
                                   double price, Decimal size) {
    printf( "UpdateMarketDepth. %ld - Position: %s, Operation: %d, Side: %d, Price: %s, Size: %s\n", id, Utils::intMaxString(position).c_str(), operation, side, 
        Utils::doubleMaxString(price).c_str(), DecimalFunctions::decimalStringToDisplay(size).c_str());
}
//! [updatemktdepth]

//! [updatemktdepthl2]
void CapstoneCppClient::updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
                                     int side, double price, Decimal size, bool isSmartDepth) {
    printf( "UpdateMarketDepthL2. %ld - Position: %s, Operation: %d, Side: %d, Price: %s, Size: %s, isSmartDepth: %d\n", id, Utils::intMaxString(position).c_str(), operation, side, 
        Utils::doubleMaxString(price).c_str(), DecimalFunctions::decimalStringToDisplay(size).c_str(), isSmartDepth);
}
//! [updatemktdepthl2]

//! [updatenewsbulletin]
void CapstoneCppClient::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
	printf( "News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: %s\n", msgId, msgType, newsMessage.c_str(), originExch.c_str());
}
//! [updatenewsbulletin]

//! [managedaccounts]
void CapstoneCppClient::managedAccounts( const std::string& accountsList) {
	printf( "Account List: %s\n", accountsList.c_str());
}
//! [managedaccounts]

//! [receivefa]
void CapstoneCppClient::receiveFA(faDataType pFaDataType, const std::string& cxml) {
	std::cout << "Receiving FA: " << (int)pFaDataType << std::endl << cxml << std::endl;
}
//! [receivefa]

//! [historicaldata]
void CapstoneCppClient::historicalData(TickerId reqId, const Bar& bar) {
    printf( "HistoricalData. ReqId: %ld - Date: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, bar.time.c_str(), 
        Utils::doubleMaxString(bar.open).c_str(), Utils::doubleMaxString(bar.high).c_str(), Utils::doubleMaxString(bar.low).c_str(), Utils::doubleMaxString(bar.close).c_str(), 
		DecimalFunctions::decimalStringToDisplay(bar.volume).c_str(), Utils::intMaxString(bar.count).c_str(), DecimalFunctions::decimalStringToDisplay(bar.wap).c_str());
}
//! [historicaldata]

//! [historicaldataend]
void CapstoneCppClient::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
	std::cout << "HistoricalDataEnd. ReqId: " << reqId << " - Start Date: " << startDateStr << ", End Date: " << endDateStr << std::endl;	
}
//! [historicaldataend]

//! [scannerparameters]
void CapstoneCppClient::scannerParameters(const std::string& xml) {
	printf( "ScannerParameters. %s\n", xml.c_str());
}
//! [scannerparameters]

//! [scannerdata]
void CapstoneCppClient::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
                                const std::string& distance, const std::string& benchmark, const std::string& projection,
                                const std::string& legsStr) {
	printf( "ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n", reqId, rank, contractDetails.contract.symbol.c_str(), contractDetails.contract.secType.c_str(), contractDetails.contract.currency.c_str(), distance.c_str(), benchmark.c_str(), projection.c_str(), legsStr.c_str());
}
//! [scannerdata]

//! [scannerdataend]
void CapstoneCppClient::scannerDataEnd(int reqId) {
	printf( "ScannerDataEnd. %d\n", reqId);
}
//! [scannerdataend]

//! [realtimebar]
void CapstoneCppClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                                Decimal volume, Decimal wap, int count) {
    printf( "RealTimeBars. %ld - Time: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, Utils::longMaxString(time).c_str(), 
        Utils::doubleMaxString(open).c_str(), Utils::doubleMaxString(high).c_str(), Utils::doubleMaxString(low).c_str(), Utils::doubleMaxString(close).c_str(), 
		DecimalFunctions::decimalStringToDisplay(volume).c_str(), Utils::intMaxString(count).c_str(), DecimalFunctions::decimalStringToDisplay(wap).c_str());
}
//! [realtimebar]

//! [fundamentaldata]
void CapstoneCppClient::fundamentalData(TickerId reqId, const std::string& data) {
	printf( "FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
}
//! [fundamentaldata]

void CapstoneCppClient::deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) {
    printf( "DeltaNeutralValidation. %d, ConId: %ld, Delta: %s, Price: %s\n", reqId, deltaNeutralContract.conId, Utils::doubleMaxString(deltaNeutralContract.delta).c_str(), Utils::doubleMaxString(deltaNeutralContract.price).c_str());
}

//! [ticksnapshotend]
void CapstoneCppClient::tickSnapshotEnd(int reqId) {
	printf( "TickSnapshotEnd: %d\n", reqId);
}
//! [ticksnapshotend]

//! [position]
void CapstoneCppClient::position( const std::string& account, const Contract& contract, Decimal position, double avgCost) {
    printf( "Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), DecimalFunctions::decimalStringToDisplay(position).c_str(), Utils::doubleMaxString(avgCost).c_str());
}
//! [position]

//! [positionend]
void CapstoneCppClient::positionEnd() {
	printf( "PositionEnd\n");
}
//! [positionend]

//! [accountsummary]
void CapstoneCppClient::accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
	printf( "Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
}
//! [accountsummary]

//! [accountsummaryend]
void CapstoneCppClient::accountSummaryEnd( int reqId) {
	printf( "AccountSummaryEnd. Req Id: %d\n", reqId);
}
//! [accountsummaryend]

void CapstoneCppClient::verifyMessageAPI( const std::string& apiData) {
	printf("verifyMessageAPI: %s\b", apiData.c_str());
}

void CapstoneCppClient::verifyCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful, errorText.c_str());
}

void CapstoneCppClient::verifyAndAuthMessageAPI( const std::string& apiDatai, const std::string& xyzChallenge) {
	printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(), xyzChallenge.c_str());
}

void CapstoneCppClient::verifyAndAuthCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful, errorText.c_str());
    if (isSuccessful)
        m_pClient->startApi();
}

//! [displaygrouplist]
void CapstoneCppClient::displayGroupList( int reqId, const std::string& groups) {
	printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
}
//! [displaygrouplist]

//! [displaygroupupdated]
void CapstoneCppClient::displayGroupUpdated( int reqId, const std::string& contractInfo) {
	std::cout << "Display Group Updated. ReqId: " << reqId << ", Contract Info: " << contractInfo << std::endl;
}
//! [displaygroupupdated]

//! [positionmulti]
void CapstoneCppClient::positionMulti( int reqId, const std::string& account,const std::string& modelCode, const Contract& contract, Decimal pos, double avgCost) {
    printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), DecimalFunctions::decimalStringToDisplay(pos).c_str(), Utils::doubleMaxString(avgCost).c_str());
}
//! [positionmulti]

//! [positionmultiend]
void CapstoneCppClient::positionMultiEnd( int reqId) {
	printf("Position Multi End. Request: %d\n", reqId);
}
//! [positionmultiend]

//! [accountupdatemulti]
void CapstoneCppClient::accountUpdateMulti( int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
	printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, %s, Value: %s, Currency: %s\n", reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(), currency.c_str());
}
//! [accountupdatemulti]

//! [accountupdatemultiend]
void CapstoneCppClient::accountUpdateMultiEnd( int reqId) {
	printf("Account Update Multi End. Request: %d\n", reqId);
}
//! [accountupdatemultiend]

//! [softDollarTiers]
void CapstoneCppClient::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {
	printf("Soft dollar tiers (%lu):", tiers.size());

	for (unsigned int i = 0; i < tiers.size(); i++) {
		printf("%s\n", tiers[i].displayName().c_str());
	}
}
//! [softDollarTiers]

//! [familyCodes]
void CapstoneCppClient::familyCodes(const std::vector<FamilyCode> &familyCodes) {
	printf("Family codes (%lu):\n", familyCodes.size());

	for (unsigned int i = 0; i < familyCodes.size(); i++) {
		printf("Family code [%d] - accountID: %s familyCodeStr: %s\n", i, familyCodes[i].accountID.c_str(), familyCodes[i].familyCodeStr.c_str());
	}
}
//! [familyCodes]

//! [symbolSamples]
void CapstoneCppClient::symbolSamples(int reqId, const std::vector<ContractDescription> &contractDescriptions) {
	printf("Symbol Samples (total=%lu) reqId: %d\n", contractDescriptions.size(), reqId);

	for (unsigned int i = 0; i < contractDescriptions.size(); i++) {
		Contract contract = contractDescriptions[i].contract;
		std::vector<std::string> derivativeSecTypes = contractDescriptions[i].derivativeSecTypes;
		printf("Contract (%u): conId: %ld, symbol: %s, secType: %s, primaryExchange: %s, currency: %s, ", i, contract.conId, contract.symbol.c_str(), contract.secType.c_str(), contract.primaryExchange.c_str(), contract.currency.c_str());
		printf("Derivative Sec-types (%lu):", derivativeSecTypes.size());
		for (unsigned int j = 0; j < derivativeSecTypes.size(); j++) {
			printf(" %s", derivativeSecTypes[j].c_str());
		}
		printf(", description: %s, issuerId: %s", contract.description.c_str(), contract.issuerId.c_str());
		printf("\n");
	}
}
//! [symbolSamples]

//! [mktDepthExchanges]
void CapstoneCppClient::mktDepthExchanges(const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {
	printf("Mkt Depth Exchanges (%lu):\n", depthMktDataDescriptions.size());

	for (unsigned int i = 0; i < depthMktDataDescriptions.size(); i++) {
        printf("Depth Mkt Data Description [%d] - exchange: %s secType: %s listingExch: %s serviceDataType: %s aggGroup: %s\n", i,
            depthMktDataDescriptions[i].exchange.c_str(),
            depthMktDataDescriptions[i].secType.c_str(),
            depthMktDataDescriptions[i].listingExch.c_str(),
            depthMktDataDescriptions[i].serviceDataType.c_str(),
            Utils::intMaxString(depthMktDataDescriptions[i].aggGroup).c_str());
	}
}
//! [mktDepthExchanges]

//! [tickNews]
void CapstoneCppClient::tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) {
    char timeStampStr[80];
#if defined(IB_WIN32)
    ctime_s(timeStampStr, sizeof(timeStampStr), &(timeStamp /= 1000));
#else
    ctime_r(&(timeStamp /= 1000), timeStampStr);
#endif
    printf("News Tick. TickerId: %d, TimeStamp: %s, ProviderCode: %s, ArticleId: %s, Headline: %s, ExtraData: %s\n", tickerId, timeStampStr, providerCode.c_str(), articleId.c_str(), headline.c_str(), extraData.c_str());
}
//! [tickNews]

//! [smartcomponents]]
void CapstoneCppClient::smartComponents(int reqId, const SmartComponentsMap& theMap) {
	printf("Smart components: (%lu):\n", theMap.size());

	for (SmartComponentsMap::const_iterator i = theMap.begin(); i != theMap.end(); i++) {
		printf(" bit number: %d exchange: %s exchange letter: %c\n", i->first, std::get<0>(i->second).c_str(), std::get<1>(i->second));
	}
}
//! [smartcomponents]

//! [newsProviders]
void CapstoneCppClient::newsProviders(const std::vector<NewsProvider> &newsProviders) {
	printf("News providers (%lu):\n", newsProviders.size());

	for (unsigned int i = 0; i < newsProviders.size(); i++) {
		printf("News provider [%d] - providerCode: %s providerName: %s\n", i, newsProviders[i].providerCode.c_str(), newsProviders[i].providerName.c_str());
	}
}
//! [newsProviders]

//! [newsArticle]
void CapstoneCppClient::newsArticle(int requestId, int articleType, const std::string& articleText) {
	printf("News Article. Request Id: %d, Article Type: %d\n", requestId, articleType);
	if (articleType == 0) {
		printf("News Article Text (text or html): %s\n", articleText.c_str());
	} else if (articleType == 1) {
		std::string path;
		#if defined(IB_WIN32)
			TCHAR s[200];
			GetCurrentDirectory(200, s);
			//path = s;// +std::string("\\MST$06f53098.pdf");
		#elif defined(IB_POSIX)
			char s[1024];
			if (getcwd(s, sizeof(s)) == NULL) {
				printf("getcwd() error\n");
				return;
			}
			path = s + std::string("/MST$06f53098.pdf");
		#endif
		std::vector<std::uint8_t> bytes = Utils::base64_decode(articleText);
		std::ofstream outfile(path, std::ios::out | std::ios::binary); 
		outfile.write((const char*)bytes.data(), bytes.size());
		printf("Binary/pdf article was saved to: %s\n", path.c_str());
	}
}
//! [newsArticle]

//! [historicalNews]
void CapstoneCppClient::historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) {
	printf("Historical News. RequestId: %d, Time: %s, ProviderCode: %s, ArticleId: %s, Headline: %s\n", requestId, time.c_str(), providerCode.c_str(), articleId.c_str(), headline.c_str());
}
//! [historicalNews]

//! [historicalNewsEnd]
void CapstoneCppClient::historicalNewsEnd(int requestId, bool hasMore) {
	printf("Historical News End. RequestId: %d, HasMore: %s\n", requestId, (hasMore ? "true" : " false"));
}
//! [historicalNewsEnd]

//! [headTimestamp]
void CapstoneCppClient::headTimestamp(int reqId, const std::string& headTimestamp) {
	printf( "Head time stamp. ReqId: %d - Head time stamp: %s,\n", reqId, headTimestamp.c_str());

}
//! [headTimestamp]

//! [histogramData]
void CapstoneCppClient::histogramData(int reqId, const HistogramDataVector& data) {
	printf("Histogram. ReqId: %d, data length: %lu\n", reqId, data.size());

    for (const HistogramEntry& entry : data) {
        printf("\t price: %s, size: %s\n", Utils::doubleMaxString(entry.price).c_str(), DecimalFunctions::decimalStringToDisplay(entry.size).c_str());
	}
}
//! [histogramData]

//! [historicalDataUpdate]
void CapstoneCppClient::historicalDataUpdate(TickerId reqId, const Bar& bar) {
    printf( "HistoricalDataUpdate. ReqId: %ld - Date: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, bar.time.c_str(), 
        Utils::doubleMaxString(bar.open).c_str(), Utils::doubleMaxString(bar.high).c_str(), Utils::doubleMaxString(bar.low).c_str(), Utils::doubleMaxString(bar.close).c_str(), 
		DecimalFunctions::decimalStringToDisplay(bar.volume).c_str(), Utils::intMaxString(bar.count).c_str(), DecimalFunctions::decimalStringToDisplay(bar.wap).c_str());
}
//! [historicalDataUpdate]

//! [rerouteMktDataReq]
void CapstoneCppClient::rerouteMktDataReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market data request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}
//! [rerouteMktDataReq]

//! [rerouteMktDepthReq]
void CapstoneCppClient::rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market depth request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}
//! [rerouteMktDepthReq]

//! [marketRule]
void CapstoneCppClient::marketRule(int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) {
    printf("Market Rule Id: %s\n", Utils::intMaxString(marketRuleId).c_str());
    for (unsigned int i = 0; i < priceIncrements.size(); i++) {
        printf("Low Edge: %s, Increment: %s\n", Utils::doubleMaxString(priceIncrements[i].lowEdge).c_str(), Utils::doubleMaxString(priceIncrements[i].increment).c_str());
    }
}
//! [marketRule]

//! [pnl]
void CapstoneCppClient::pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) {
    printf("PnL. ReqId: %d, daily PnL: %s, unrealized PnL: %s, realized PnL: %s\n", reqId, Utils::doubleMaxString(dailyPnL).c_str(), Utils::doubleMaxString(unrealizedPnL).c_str(), 
        Utils::doubleMaxString(realizedPnL).c_str());
}
//! [pnl]

//! [pnlsingle]
void CapstoneCppClient::pnlSingle(int reqId, Decimal pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) {
    printf("PnL Single. ReqId: %d, pos: %s, daily PnL: %s, unrealized PnL: %s, realized PnL: %s, value: %s\n", reqId, DecimalFunctions::decimalStringToDisplay(pos).c_str(), Utils::doubleMaxString(dailyPnL).c_str(),
        Utils::doubleMaxString(unrealizedPnL).c_str(), Utils::doubleMaxString(realizedPnL).c_str(), Utils::doubleMaxString(value).c_str());
}
//! [pnlsingle]

//! [historicalticks]
void CapstoneCppClient::historicalTicks(int reqId, const std::vector<HistoricalTick>& ticks, bool done) {
    for (const HistoricalTick& tick : ticks) {
        std::time_t t = tick.time;
        char timeStr[80];
#if defined(IB_WIN32)
        ctime_s(timeStr, sizeof(timeStr), &t);
#else
        ctime_r(&t, timeStr);
#endif
        std::cout << "Historical tick. ReqId: " << reqId << ", time: " << timeStr << ", price: "<< Utils::doubleMaxString(tick.price).c_str()	<< ", size: " << DecimalFunctions::decimalStringToDisplay(tick.size).c_str() << std::endl;
    }
}
//! [historicalticks]

//! [historicalticksbidask]
void CapstoneCppClient::historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk>& ticks, bool done) {
    for (const HistoricalTickBidAsk& tick : ticks) {
        std::time_t t = tick.time;
        char timeStr[80];
#if defined(IB_WIN32)
        ctime_s(timeStr, sizeof(timeStr), &t);
#else
        ctime_r(&t, timeStr);
#endif
        std::cout << "Historical tick bid/ask. ReqId: " << reqId << ", time: " << timeStr << ", price bid: "<< Utils::doubleMaxString(tick.priceBid).c_str()	<<
            ", price ask: "<< Utils::doubleMaxString(tick.priceAsk).c_str() << ", size bid: " << DecimalFunctions::decimalStringToDisplay(tick.sizeBid).c_str() << ", size ask: " << DecimalFunctions::decimalStringToDisplay(tick.sizeAsk).c_str() <<
            ", bidPastLow: " << tick.tickAttribBidAsk.bidPastLow << ", askPastHigh: " << tick.tickAttribBidAsk.askPastHigh << std::endl;
    }
}
//! [historicalticksbidask]

//! [historicaltickslast]
void CapstoneCppClient::historicalTicksLast(int reqId, const std::vector<HistoricalTickLast>& ticks, bool done) {
    for (HistoricalTickLast tick : ticks) {
        std::time_t t = tick.time;
        char timeStr[80];
#if defined(IB_WIN32)
        ctime_s(timeStr, sizeof(timeStr), &t);
#else
        ctime_r(&t, timeStr);
#endif
        std::cout << "Historical tick last. ReqId: " << reqId << ", time: " << timeStr << ", price: "<< Utils::doubleMaxString(tick.price).c_str() <<
            ", size: " << DecimalFunctions::decimalStringToDisplay(tick.size).c_str() << ", exchange: " << tick.exchange << ", special conditions: " << tick.specialConditions <<
            ", unreported: " << tick.tickAttribLast.unreported << ", pastLimit: " << tick.tickAttribLast.pastLimit << std::endl;
    }
}
//! [historicaltickslast]

//! [tickbytickalllast]
void CapstoneCppClient::tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) {
    char timeStr[80];
#if defined(IB_WIN32)
    ctime_s(timeStr, sizeof(timeStr), &time);
#else
    ctime_r(&time, timeStr);
#endif
    printf("Tick-By-Tick. ReqId: %d, TickType: %s, Time: %s, Price: %s, Size: %s, PastLimit: %d, Unreported: %d, Exchange: %s, SpecialConditions:%s\n", 
        reqId, (tickType == 1 ? "Last" : "AllLast"), timeStr, Utils::doubleMaxString(price).c_str(), DecimalFunctions::decimalStringToDisplay(size).c_str(), tickAttribLast.pastLimit, tickAttribLast.unreported, exchange.c_str(), specialConditions.c_str());
}
//! [tickbytickalllast]

//! [tickbytickbidask]
void CapstoneCppClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize, Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk) {
    char timeStr[80];
#if defined(IB_WIN32)
    ctime_s(timeStr, sizeof(timeStr), &time);
#else
    ctime_r(&time, timeStr);
#endif
    printf("Tick-By-Tick. ReqId: %d, TickType: BidAsk, Time: %s, BidPrice: %s, AskPrice: %s, BidSize: %s, AskSize: %s, BidPastLow: %d, AskPastHigh: %d\n", 
        reqId, timeStr, Utils::doubleMaxString(bidPrice).c_str(), Utils::doubleMaxString(askPrice).c_str(), DecimalFunctions::decimalStringToDisplay(bidSize).c_str(), DecimalFunctions::decimalStringToDisplay(askSize).c_str(), tickAttribBidAsk.bidPastLow, tickAttribBidAsk.askPastHigh);
}
//! [tickbytickbidask]

//! [tickbytickmidpoint]
void CapstoneCppClient::tickByTickMidPoint(int reqId, time_t time, double midPoint) {
    char timeStr[80];
#if defined(IB_WIN32)
    ctime_s(timeStr, sizeof(timeStr), &time);
#else
    ctime_r(&time, timeStr);
#endif
    printf("Tick-By-Tick. ReqId: %d, TickType: MidPoint, Time: %s, MidPoint: %s\n", reqId, timeStr, Utils::doubleMaxString(midPoint).c_str());
}
//! [tickbytickmidpoint]

//! [orderbound]
void CapstoneCppClient::orderBound(long long permId, int clientId, int orderId) {
    printf("Order bound. PermId: %s, clientId: %s, orderId: %s\n", Utils::llongMaxString(permId).c_str(), Utils::intMaxString(clientId).c_str(), Utils::intMaxString(orderId).c_str());
}
//! [orderbound]

//! [completedorder]
void CapstoneCppClient::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {
    printf( "CompletedOrder. PermId: %s, ParentPermId: %s, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType: %s, TotalQty: %s, CashQty: %s, FilledQty: %s, "
        "LmtPrice: %s, AuxPrice: %s, Status: %s, CompletedTime: %s, CompletedStatus: %s, MinTradeQty: %s, MinCompeteSize: %s, CompeteAgainstBestOffset: %s, MidOffsetAtWhole: %s, MidOffsetAtHalf: %s, "
        "CustomerAccount: %s, ProfessionalCustomer: %s\n",
        Utils::llongMaxString(order.permId).c_str(), Utils::llongMaxString(order.parentPermId).c_str(), order.account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(),
        order.action.c_str(), order.orderType.c_str(), DecimalFunctions::decimalStringToDisplay(order.totalQuantity).c_str(), Utils::doubleMaxString(order.cashQty).c_str(), DecimalFunctions::decimalStringToDisplay(order.filledQuantity).c_str(),
        Utils::doubleMaxString(order.lmtPrice).c_str(), Utils::doubleMaxString(order.auxPrice).c_str(), orderState.status.c_str(), orderState.completedTime.c_str(), orderState.completedStatus.c_str(),
        Utils::intMaxString(order.minTradeQty).c_str(), Utils::intMaxString(order.minCompeteSize).c_str(),
        order.competeAgainstBestOffset == COMPETE_AGAINST_BEST_OFFSET_UP_TO_MID ? "UpToMid" : Utils::doubleMaxString(order.competeAgainstBestOffset).c_str(),
        Utils::doubleMaxString(order.midOffsetAtWhole).c_str(), Utils::doubleMaxString(order.midOffsetAtHalf).c_str(),
        order.customerAccount.c_str(), (order.professionalCustomer ? "true" : "false"));
}
//! [completedorder]

//! [completedordersend]
void CapstoneCppClient::completedOrdersEnd() {
	printf( "CompletedOrdersEnd\n");
}
//! [completedordersend]

//! [replacefaend]
void CapstoneCppClient::replaceFAEnd(int reqId, const std::string& text) {
	printf("Replace FA End. Request: %d, Text:%s\n", reqId, text.c_str());
}
//! [replacefaend]

//! [wshMetaData]
void CapstoneCppClient::wshMetaData(int reqId, const std::string& dataJson) {
	printf("WSH Meta Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}
//! [wshMetaData]

//! [wshEventData]
void CapstoneCppClient::wshEventData(int reqId, const std::string& dataJson) {
	printf("WSH Event Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}
//! [wshEventData]

//! [historicalSchedule]
void CapstoneCppClient::historicalSchedule(int reqId, const std::string& startDateTime, const std::string& endDateTime, const std::string& timeZone, const std::vector<HistoricalSession>& sessions) {
	printf("Historical Schedule. ReqId: %d, Start: %s, End: %s, TimeZone: %s\n", reqId, startDateTime.c_str(), endDateTime.c_str(), timeZone.c_str());
	for (unsigned int i = 0; i < sessions.size(); i++) {
		printf("\tSession. Start: %s, End: %s, RefDate: %s\n", sessions[i].startDateTime.c_str(), sessions[i].endDateTime.c_str(), sessions[i].refDate.c_str());
	}
}
//! [historicalSchedule]

//! [userInfo]
void CapstoneCppClient::userInfo(int reqId, const std::string& whiteBrandingId) {
    printf("User Info. ReqId: %d, WhiteBrandingId: %s\n", reqId, whiteBrandingId.c_str());
}
//! [userInfo]

//! [securityDefinitionOptionParameter]//////////////////////////////////////////WILL NOT NEED THIS
void CapstoneCppClient::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
	const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) {

}
//! [securityDefinitionOptionParameter]

//! [securityDefinitionOptionParameterEnd]
void CapstoneCppClient::securityDefinitionOptionalParameterEnd(int reqId) {
	//printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
}
//! [securityDefinitionOptionParameterEnd]
////////////////////////////////////////////////