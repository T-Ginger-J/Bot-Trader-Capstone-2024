#pragma once
#include <string>
#include "AvailableAlgoParams.h"

class UserInterface {
	public:
		static void print(std::string str);
		static void println(std::string str);
		static void printError(std::string str);
		static std::string readln();
		
		static void printOrderMade(struct Order order);
		static void printOrderAttempt(struct Order order);
		static void getOrderDetailsFromUser();
		static std::string getValidInputFromUser(std::vector<std::string> strings, std::string errorMessage);

};