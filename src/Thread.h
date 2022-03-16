#pragma once
#include<vector>
#include<unordered_map>
#include"Client.h"
#include<memory>
#include"mutex.h"
#include<unistd.h>

const int maxClient = 4096;
const int initial_client = 128;

class Thread:public std::enable_shared_from_this<Thread> {
  using SP_Client = std::shared_ptr<Client>;
  using CallBack = std::function<void()>;
  using EventType = decltype(epoll_event::events);
  private:
    Mutex mutex;
    int epollfd;
    std::vector<epoll_event> events;
    std::unordered_map<int, SP_Client>fd2client;
    int client_num;
    int wakeup_fd;
    std::vector<CallBack> jobs_quene; //主线程给的任务队列


  public:
    Thread();
    int tid;
    void run();

    void add_client(SP_Client);
    void update_epoll(int fd, EventType ev);
    void delete_client(SP_Client);

    void append_job(CallBack&&);
    void handle_jobs();

};