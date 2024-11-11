#include "stdafx.h"
#include "UserInterface.h"
#include "Order.h"
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
	sp1_string = readln();

	println("--- Front Leg ---");
	
	print("DTE ");
	dte1_string = readln();
	print("Call/Put ");
	cp1_string = UserInterface::getValidInputFromUser(cpStrings, "Must use either \"Call\" or \"Put\".");

	println("--- Back Leg ---");

	print("DTE ");
	dte2_string = readln();
	//print("Strike Price "); // not needed for calendar spreads
	//sp2_string = readln();
	print("Call/Put ");
	cp2_string = UserInterface::getValidInputFromUser(cpStrings, "Must use either \"Call\" or \"Put\".");

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