//
// Created by xgc on 2020/10/8.
//

#ifndef MYWEBSERVER_TIMER_H
#define MYWEBSERVER_TIMER_H

#include <chrono>
#include <functional>

class Timer {
public:
  using TimeStamp = std::chrono::high_resolution_clock::time_point;
  using Clock = std::chrono::high_resolution_clock;
  using MilliSecond = std::chrono::milliseconds;

  static TimeStamp now() { return Clock::now(); }
  Timer(int timeout, const std::function<void()> &fun);

  bool operator<(const Timer &t) const { return out_time < t.out_time; }

  bool isVilid() { return is_valid; }
  void setInvalid() { is_valid = false; }
  void callback() { timeout_callback(); }
  TimeStamp getTime() { return out_time; }

private:
  TimeStamp out_time;
  bool is_valid;
  std::function<void()> timeout_callback;
};

#endif // MYWEBSERVER_TIMER_H
