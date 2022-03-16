#pragma once
#include<memory>
#include"Thread.h"
using SP_Thread = std::shared_ptr<Thread>;
class ThreadPool {
  private:
    std::vector<SP_Thread> thread_poll;
    unsigned cur_thr;
  public:
    ThreadPool(int thread_n = 4);
    SP_Thread get_next_thread();
    void run();
};