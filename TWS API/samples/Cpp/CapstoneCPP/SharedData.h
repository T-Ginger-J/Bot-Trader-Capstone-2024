#pragma once
// SharedData.h
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <string>
#include <mutex>

struct SharedData {
    int frontDTE;
    int backDTE;
    int entryHour;
    int entryMin;
    int exitHour;
    int exitMin;
    double takeProfit;
    double stopLoss;
    double entryPrice;
    std::string orderType;

    std::mutex mutex;
};

#endif // SHARED_DATA_H