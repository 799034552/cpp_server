#pragma once
#include<functional>
#include<sys/epoll.h>
#include<stdio.h>
#include<vector>
#include<memory>
#include"util.h"
#include<unistd.h>
class Client {
  using CallBack = std::function<void()>;
  using EventType = decltype(epoll_event::events);
  protected:
    int fd;
    enum class LINE_STATE {LINE_OPEN = 0, LINE_BAD, LINE_OK};
    std::string client_buf;
    EventType event;
    EventType revent;
    CallBack read_fn;
    CallBack write_fn;
    int read_all();
    LINE_STATE read_line();



  public:
    Client(const int &fd_):fd(fd_) { set_fd_noblock(fd); };
    void set_event(const EventType &e){ event = e;}
    void set_revent(const EventType &e){ revent = e;}
    EventType get_event() const { return event; }
    int get_fd() const { return fd; }
    void set_read_fn(const CallBack &fn) { read_fn = fn; }
    void set_write_fn(const CallBack &fn) { read_fn = fn; }
    void close();
    void handle_event();
};