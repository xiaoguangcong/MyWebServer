//
// Created by xgc on 2020/10/8.
//

#include "../include/timer.h"
#include <ctime>
#include <memory>
#include <sys/time.h>

TimerNode::TimerNode(std::shared_ptr<HttpData> request_data, int timeout)
  : valid_(true), http_data_ptr(request_data)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  // 以毫秒计
  expired_time_ = 
    (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode()
{
  if (http_data_ptr)
  {
    http_data_ptr.HandleClose();
  }
}

TimerNode::TimerNode(TimerNode &tn)
  : http_data_ptr(tn.http_data_ptr), expired_time_(0)
{}

void TimerNode::Update(int timeout)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  expired_time_ =
      (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::IsValid()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
  if (temp < expired_time_)
  {
    return true;
  }
  else
  {
    this->SetInvalid();
    return false;
  }
}

void TimerNode::ClearRequest()
{
  http_data_ptr.reset();
  this->SetInvalid();
}

TimerManager::TimerManager() {}

TimerManager::~TimerManager() {}

void TimerManager::AddTimer(std::shared_ptr<HttpData> http_data_ptr, int timeout)
{
  TimerNodePtr new_timer(new TimerNode(http_data_ptr, timeout));
  timer_queue.push(new_timer);
  http_data_ptr.LinkTimer(new_timer);
}

/* 处理逻辑是这样的~
因为
(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为InValid的时间节点，会延迟到它超时 或它前面的节点都被删除时，它才会被删除。
一个点被置为InValid ,它最迟会在TIMER_TIME_OUT时间后被删除。

这样做有两个好处：
(1) 第一个好处是不需要遍历优先队列，省时。
(2)第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
*/


void TimerManager::HandleExpiredEvent()
{
  while(!timer_queue.empty())
  {
    TimerNodePtr timer_ptr = timer_queue.top();
    if (timer_ptr->IsInvalid())
    {
      timer_queue.pop();
    }
    else if (timer_ptr->IsValid() == false)
    {
      timer_queue.pop();
    }
    else
    {
      break;
    }
  }
}