//
// Created by xgc on 2020/10/8.
//

#pragma once

#include "channel.h"
#include <chrono>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <queue>

class HttpData;

class TimerNode
{
public:
  TimerNode(std::shared_ptr<HttpData> request_data, int timeout);
  ~TimerNode();
  TimerNode(TimerNode &tn);

  void Update(int timeout);
  bool IsValid();
  void ClearRequest();
  void SetInvalid() {valid_ = false; }
  bool IsInvalid() const { return !valid_; };
  size_t GetExpiredTime() const { return expired_time_; };

private:
  bool valid_;
  size_t expired_time_;
  std::shared_ptr<HttpData> http_data_ptr;
};



// class Timer {
// public:
//   using TimeStamp = std::chrono::high_resolution_clock::time_point;
//   using Clock = std::chrono::high_resolution_clock;
//   using MilliSecond = std::chrono::milliseconds;

//   static TimeStamp now() { return Clock::now(); }
//   Timer(int timeout, const std::function<void()> &fun);

//   bool operator<(const Timer &t) const { return out_time < t.out_time; }

//   bool isVilid() { return is_valid; }
//   void setInvalid() { is_valid = false; }
//   void callback() { timeout_callback(); }
//   TimeStamp getTime() { return out_time; }

// private:
//   TimeStamp out_time;
//   bool is_valid;
//   std::function<void()> timeout_callback;
// };

struct TimerCmp
{
  bool operator()(std::shared_ptr<TimerNode> &a,
                  std::shared_ptr<TimerNode> &b) const
  {
    return a->GetExpiredTime() > b->GetExpiredTime();
  }  
};

class TimerManager
{
public:
  TimerManager();
  ~TimerManager();

  void AddTimer(std::shared_ptr<HttpData> http_data_ptr, int timeout);
  void HandleExpiredEvent();

private:
  typedef std::shared_ptr<TimerNode> TimerNodePtr;
  std::priority_queue<TimerNodePtr, std::deque<TimerNodePtr>, TimerCmp> timer_queue;
};
