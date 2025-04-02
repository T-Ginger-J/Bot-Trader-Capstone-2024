/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#pragma once
#ifndef TWS_API_SAMPLES_CAPSTONECPPCLIENT_CAPSTONECPPCLIENT_H
#define TWS_API_SAMPLES_CAPSTONECPPCLIENT_CAPSTONECPPCLIENT_H

#include "EWrapper.h"
#include "EReaderOSSignal.h"
#include "EReader.h"

#include <memory>
#include <vector>

#include "message_queue.h"

class EClientSocket;

enum State {
	ST_CONNECT,
	ST_HISTORICALDATAREQUESTS,
	ST_HISTORICALDATAREQUESTS_ACK,
	ST_OPTIONSOPERATIONS,
	ST_OPTIONSOPERATIONS_ACK,
	ST_OCASAMPLES,
	ST_OCASAMPLES_ACK,
	ST_CONDITIONSAMPLES,
	ST_CONDITIONSAMPLES_ACK,
	ST_BRACKETSAMPLES,
	ST_BRACKETSAMPLES_ACK,
	ST_CANCELORDER,
	ST_CANCELORDER_ACK,
	ST_PING,
	ST_PING_ACK,
    ST_REQHISTORICALTICKS,
    ST_REQHISTORICALTICKS_ACK,
    ST_REQTICKBYTICKDATA,
    ST_REQTICKBYTICKDATA_ACK,
	ST_IDLE,

	ST_ORDEROPERATIONS,
	ST_ORDEROPERATIONS_ACK,
	/////////////////////////////////
	//OUR STATES BELOW
	ST_USERINPUT,
	ST_USERINPUT_ACK,
	ST_GETSPXPRICE,
	ST_GETSPXPRICE_ACK,
	ST_SINGLEORDER,
	ST_SINGLEORDER_ACK,
	ST_COMBOORDER,
	ST_COMBOORDER_ACK,
	ST_COMBOPRICE,
	ST_COMBOPRICE_ACK,
	ST_ADJUSTORDER,
	ST_ADJUSTORDER_ACK,
	ST_COMBOINFO,

	ST_WAITFORINPUT,

};

struct TradeRecord {
	std::string symbol;
	std::string entryTime;
	double entryPrice;
	std::string exitTime;
	double exitPrice;
	double shares;
	std::string side;   // The side of the entry ("BUY" for a long entry, "SELL" for a short entry)
};

//! [ewrapperimpl]
class CapstoneCppClient : public EWrapper
{
//! [ewrapperimpl]
public:

	CapstoneCppClient();
	~CapstoneCppClient();

	void setConnectOptions(const std::string&);
	void setOptionalCapabilities(const std::string&);
	void processMessages();



public:

	bool connect(const char * host, int port, int clientId = 0);
	void disconnect() const;
	bool isConnected() const;

private:

	void historicalDataRequests();
	void optionsOperations();
	void orderOperations();
	void ocaSamples();
	void conditionSamples();
	void bracketSample();
    void reqHistoricalTicks();
    void reqTickByTickData();

	void reqCurrentTime();
	
	/////////////////////////////////////////
	//OUR METHODS STUBS BELOW
	void getComboOrder();
	void getComboPrices();

	void placeComboOrder();



	void getAllExecutions();

	time_t parseActivationTime(const Message& msg);
	int getTzOffset(const std::wstring& tz);

	void waitForGuiDataAndTime();
	void exitPosition(const TradeRecord& record, long originalOrderId);
	//////////////////////////////////////////////////
public:
	// events
	#include "EWrapper_prototypes.h"


private:
	void printContractMsg(const Contract& contract);
	void printContractDetailsMsg(const ContractDetails& contractDetails);
	void printContractDetailsSecIdList(const TagValueListSPtr &secIdList);
	void printBondContractDetailsMsg(const ContractDetails& contractDetails);
	void printContractDetailsIneligibilityReasonList(const IneligibilityReasonListSPtr &ineligibilityReasonList);

private:
	//! [socket_declare]
	EReaderOSSignal m_osSignal;
	EClientSocket * const m_pClient;
	//! [socket_declare]
	State m_state;
	time_t m_sleepDeadline;

	OrderId m_orderId;
	std::unique_ptr<EReader> m_pReader;
    bool m_extraAuth;
	std::string m_bboExchange;
	
};

#endif

time_t parseDate(const std::string& dateStr);
