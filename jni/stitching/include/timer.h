//
// Created by irina on 20.05.18.
//

#ifndef TENSORFLOWSTITCHING_TIMER_H
#define TENSORFLOWSTITCHING_TIMER_H

#define TIMING
#include <chrono>
#include <android/log.h>
#include <iostream>
#include <string>
#include <sstream>
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    std::string str = os.str();
    return str;
}
#ifdef TIMING
#define INIT_TIMER  auto start = std::chrono::high_resolution_clock::now();
#define START_TIMER  start = std::chrono::high_resolution_clock::now();
#define STOP_TIMER(name)  __android_log_print(ANDROID_LOG_DEBUG, "Stitching", "TIME of %s : %ld ms", name, \
            std::chrono::duration_cast<std::chrono::milliseconds>( \
            std::chrono::high_resolution_clock::now()-start).count());
#else
#define INIT_TIMER
#define START_TIMER
#define STOP_TIMER(name)
#endif

#endif //TENSORFLOWSTITCHING_TIMER_H
