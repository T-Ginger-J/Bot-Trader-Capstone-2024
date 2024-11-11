#include "stdafx.h"
#include "UserInterface.h"
#include "Order.h"
#include "Utils.h"
#include <iostream>

void UserInterface::print(std::string str) {
	std::cout << str;
}
void UserInterface::println(std::string str) {
	std::cout << str + "\n";
}
void UserInterface::printError(std::string str) {
	println("ERROR: " + str);
}
std::string UserInterface::readln() {
	std::cout << "> ";
	std::string input;
	std::cin >> input;
	return input;
}

void UserInterface::printOrderMade(Order order) {
	printf("ORDER SUCCESS: %ld: %s - %s - %lf\n", order.orderId, order.action.c_str(), order.account.c_str(), order.lmtPrice);
}
void UserInterface::printOrderAttempt(Order order) {
	printf("ORDER ATTEMPT: %ld: %s - %s - %lf\n", order.orderId, order.action.c_str(), order.account.c_str(), order.lmtPrice);
}


void UserInterface::getOrderDetailsFromUser() {
	
	std::string dte1_string;
	std::string dte2_string;
	std::string sp1_string;
	std::string sp2_string;
	std::string cp1_string;
	std::string cp2_string;
	std::vector<std::string> cpStrings = {"Call", "call", "CALL", "c", "C", "Put", "put", "PUT", "p", "P"};

	println("NEW CALENDAR SPREAD ORDER:");

	print("Strike Price ");
	sp1_string = getIntegerInputFromUser(100, 10000);

	println("--- Front Leg ---");
	
	print("DTE ");
	dte1_string = getIntegerInputFromUser(0, 10);
	print("Call/Put ");
	cp1_string = UserInterface::getValidInputFromUser(cpStrings, "Must use either \"Call\" or \"Put\".");

	println("--- Back Leg ---");

	print("DTE ");
	dte2_string = getIntegerInputFromUser(0, 10);
	//print("Strike Price "); // not needed for calendar spreads
	//sp2_string = getIntegerInputFromUser(100, 10000);;
	print("Call/Put ");
	cp2_string = UserInterface::getValidInputFromUser(cpStrings, "Must use either \"Call\" or \"Put\".");

}
void UserInterface::getStopLimitDetailsFromUser() {

	std::string sltp_string;
	std::vector<std::string> sltpStrings = { "sl", "stop loss", "STOP LOSS", "Stop Loss", "SL", "Take Profit", "take profit", "TAKE PROFIT", "tp", "TP", "OCO", "oco", "one cancel other", "One Cancel Other", "ONE CANCEL OTHER" };

	std::string lp_string;
	std::string ocosl_string;
	std::string ocotp_string;

	println("STOP LIMIT ORDER:");

	print("Stop Loss / Take Profit / OCO ");
	sltp_string = UserInterface::getValidInputFromUser(sltpStrings, "Must use either \"Stop Loss\", \"Take Profit\", or \"OCO\".");

	if (sltp_string._Equal("OCO") || sltp_string._Equal("oco") || sltp_string._Equal("one cancel other") || sltp_string._Equal("One Cancel Other") || sltp_string._Equal("ONE CANCEL OTHER")) {
		print("Stop Loss Value ");
		ocosl_string = getIntegerInputFromUser(100, 10000);
		print("Take Profit Value ");
		ocotp_string = getIntegerInputFromUser(100, 10000);
	}
	else {
		print("Limit Price ");
		lp_string = getIntegerInputFromUser(100, 10000);
	}
	
}


std::string UserInterface::getValidInputFromUser(std::vector<std::string> strings, std::string errorMessage) {
	while (true) {
		std::string input = UserInterface::readln();

		for (size_t i = 0; i < strings.size(); i++)
		{
			if (strings[i]._Equal(input)) { return strings[i]; }
		}

		UserInterface::println(errorMessage);
	}
}
int UserInterface::getIntegerInputFromUser() {
	while (true) {
		std::string input = UserInterface::readln();

		if (Utils::isValidInteger(input)) {
			return atoi(input.c_str());
		}

		UserInterface::println("Must use a valid integer.");
	}
}
int UserInterface::getIntegerInputFromUser(int min, int max) {
	while (true) {
		std::string input = UserInterface::readln();

		if (Utils::isValidInteger(input)) {
			
			int input_int = atoi(input.c_str());

			if (min <= input_int && input_int <= max)
			{
				return input_int;
			}
		}

		printf("Must use a valid integer between %d and %d.\n", min, max);
	}
}
double UserInterface::getDoubleInputFromUser() {
	while (true) {
		std::string input = UserInterface::readln();

		if (Utils::isValidDouble(input)) {
			return stod(input);
		}

		UserInterface::println("Must use a valid decimal value.");
	}
}
double UserInterface::getDoubleInputFromUser(double min, double max) {
	while (true) {
		std::string input = UserInterface::readln();

		if (Utils::isValidDouble(input)) {

			double input_double = stod(input);

			if (min <= input_double && input_double <= max)
			{
				return input_double;
			}
		}

		printf("Must use a valid decimal value between %fl and %fl.\n", min, max);
	}
}