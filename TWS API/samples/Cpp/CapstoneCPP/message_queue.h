#pragma once
#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "stdafx.h"
#include <queue>
#include <mutex>
#include <condition_variable>

struct Message {
    int frontDTE;
    int frontStrikeChangeAmt;
    int backDTE;
    int backStrikeChangeAmt;
    int entryHour;
    int entryMin;
    int exitHour;
    int exitMin;
    double takeProfit;
    double stopLoss;
    std::wstring frontOption;
    std::wstring backOption;
    std::wstring frontAction;
    std::wstring backAction;
    std::wstring orderType;

    std::wstring activationTime;  // Format: "YYYYMMDD HH:MM:SS"
    std::wstring timeZone = L"America/New_York";  // Default timezone
    bool processed = false;
    time_t scheduledTime = 0;
};

class MessageQueue {
public:
    void push(const Message& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(msg);
        cv.notify_one();
    }

    Message pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        Message msg = queue.front();
        queue.pop();
        return msg;
    }

    bool isEmpty() {
        return queue.empty();
    }

private:
    std::queue<Message> queue;
    std::mutex mtx;
    std::condition_variable cv;
};

extern MessageQueue messageQueue;

#endif // MESSAGE_QUEUE_H