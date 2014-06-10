// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef BASE_CONCURRENT_QUEUE_H_
#define BASE_CONCURRENT_QUEUE_H_

#include <queue>

#include "base/basictypes.h"
#include "base/mutex.h"

namespace base {

template<typename Data>
class ConcurrentQueue {
 public:
  ConcurrentQueue() {}
  virtual ~ConcurrentQueue() {}
  void Push(Data const& data) {
    lock::MutexLock mutex(&mutex_);
    queue_.push(data);
    //  lock.unlock();
    cond_var_.Signal();
  }

  bool Empty() const {
    lock::MutexLock mutex(&mutex_);
    return queue_.empty();
  }

  bool TryPop(Data& popped_value) {
    lock::MutexLock mutex(&mutex_);
    if (queue_.empty()) {
      return false;
    }
    popped_value = queue_.front();
    queue_.pop();
    return true;
  }

  void Pop(Data& popped_value) {
    lock::MutexLock mutex(&mutex_);
    while (queue_.empty()) {
      cond_var_.Wait(&mutex_);
    }
    popped_value = queue_.front();
    queue_.pop();
  }

  size_t Size() const {
    lock::MutexLock mutex(&mutex_);
    return queue_.size();
  }
private:
  std::queue<Data> queue_;
  mutable lock::Mutex mutex_;
  lock::CondVar cond_var_;
  DISALLOW_COPY_AND_ASSIGN(ConcurrentQueue);
};

template<typename Data>
class FixedSizeConQueue : public ConcurrentQueue<Data> {
 public:
  FixedSizeConQueue(int max_size = 256)
    : max_size_(max_size) {
  }
  virtual ~FixedSizeConQueue() {}
  void Push(Data const& data) {
    lock::MutexLock mutex(&mutex_);
    while (queue_.size() == max_size_) {
      cond_var_.Wait(&mutex_);
    }
    queue_.push(data);
    cond_var_.Signal();
  }

  void Pop(Data& popped_value) {
    lock::MutexLock mutex(&mutex_);
    while (queue_.empty()) {
      cond_var_.Wait(&mutex_);
    }
    popped_value = queue_.front();
    queue_.pop();
    cond_var_.Signal();
  }

private:
  std::queue<Data> queue_;
  int max_size_;
  mutable lock::Mutex mutex_;
  lock::CondVar cond_var_;
  DISALLOW_COPY_AND_ASSIGN(FixedSizeConQueue);
};
}  //  namespace base
#endif  //  BASE_CONCURRENT_QUEUE_H_
