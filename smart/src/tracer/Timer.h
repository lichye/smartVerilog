#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#ifndef TIMER_H
#define TIMER_H
enum timerType {
    SBY_Timer,
    CVC5_Timer,
    FM_Timer
};

class Timer {
public:
    Timer();
    void start(timerType);
    void stop(timerType);
    double elapsed() const;
    // void printTime();
    std::string printTime();

private:
    int sbyCounter;
    int cvc5Counter;
    int fmCounter;
    double sbyTotalTime;
    double cvc5TotalTime;
    double fmTotalTime;
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> end_time;
};
#endif