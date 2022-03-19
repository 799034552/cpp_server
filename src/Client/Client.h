#pragma once
#include<functional>
#include<sys/epoll.h>
#include<stdio.h>
#include<vector>
#include<memory>
#include"util.h"
#include<unistd.h>
#include"thread"
//#include"Timer.h"
#define BUF_SIZE 1024
class TimeNode;
class Client {
  using CallBack = std::function<void()>;
  using EventType = decltype(epoll_event::events);
  using SP_Client = std::shared_ptr<Client>;
  protected:
    int fd;
    enum class LINE_STATE {LINE_OPEN = 0, LINE_BAD, LINE_OK};
    std::string rec_buf;
    std::string send_buf;
    EventType event;
    EventType revent;
    CallBack read_fn;
    
    SP_Client change_to_client;
    std::function<void(SP_Client)> change_fn;
    int epollfd;
    int read_all(bool &m_is_close);
    LINE_STATE read_line();
    std::shared_ptr<TimeNode> time_node;
    void update_timenode();
    
  public:
    CallBack write_fn;
    bool is_close;
    bool is_change;
    time_t live_time;
    CallBack close_fn;
    Client(const int &fd_):fd(fd_),is_close(false),is_change(false),live_time(2*60),time_node(nullptr) { set_fd_noblock(fd); };
    void set_event(const EventType &e){ event = e;}
    void set_revent(const EventType &e){ revent = e;}
    EventType get_event() const { return event; }
    int get_fd() const { return fd; }
    void set_read_fn(const CallBack &fn) { read_fn = fn; }
    void set_write_fn(const CallBack &fn) { read_fn = fn; }
    void set_close_fn(const CallBack &fn) { close_fn = fn; }
    void set_change_fn(const std::function<void(SP_Client)> &fn) { change_fn = fn; }
    void set_epollfd(const int &fd) { epollfd = fd ;}
    void set_time_node(const std::shared_ptr<TimeNode> &t);
    void delete_timenode();
    void update_event(EventType);
    void handle_event();
};