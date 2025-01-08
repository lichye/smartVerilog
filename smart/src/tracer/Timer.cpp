#include <iostream>
#include <chrono>
#include <thread>
#include "Timer.h"
#include "utils.h"

Timer::Timer(){
    sbyCounter = 0;
    cvc5Counter = 0;
    sbyTotalTime = 0;
    cvc5TotalTime = 0;
}

void Timer::start(timerType type) {
    start_time = std::chrono::steady_clock::now(); 
}

void Timer::stop(timerType type) {
    end_time = std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration<double>(end_time - start_time).count();
    if(type == SBY_Timer){
        sbyCounter++;
        sbyTotalTime += elapsed_seconds;
    }
    else if(type == CVC5_Timer){
        cvc5Counter++;
        cvc5TotalTime += elapsed_seconds;
    }
    else{
        std::cerr << "Error: Timer type not supported." << std::endl;
    }
}

double Timer::elapsed() const {
    std::chrono::time_point<std::chrono::steady_clock> end_point;
    return std::chrono::duration<double>(end_point - start_time).count();
}

// void Timer::printTime(){
//     print("FM checker are called "+std::to_string(sbyCounter)+" times\n");
//     print("FM checker Timer: "+std::to_string(sbyTotalTime)+" seconds\n");
//     print("CVC5 are called "+std::to_string(cvc5Counter)+" times\n");
//     print("CVC5 Timer: "+std::to_string(cvc5TotalTime)+" seconds\n");
// }

std::string Timer::printTime(){
    std::string result = "";
    result += "FM checker are called "+std::to_string(sbyCounter)+" times\n";
    result += "FM checker Timer: "+std::to_string(sbyTotalTime)+" seconds\n";
    result += "CVC5 are called "+std::to_string(cvc5Counter)+" times\n";
    result += "CVC5 Timer: "+std::to_string(cvc5TotalTime)+" seconds\n";
    return result;
}