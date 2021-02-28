//
// Created by xgc on 2020/10/8.
//

#include "timer.h"

Timer::Timer(int timeout, const std::function<void()> &fun)
    : out_time(now() + static_cast<MilliSecond>(timeout)), is_valid(true),
      timeout_callback(fun) {}
