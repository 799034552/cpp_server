#pragma once
#include<functional>
#include<sys/epoll.h>
#include<stdio.h>
#include<vector>
#include"util.h"
#include<unistd.h>
class Client {
  using CallBack = std::function<void()>;
  using EventType = decltype(epoll_event::events);
  protected:
    int fd;
    enum class HTTP_CODE {HTTP_OPEN = 0, HTTP_BAD, HTTP_OK, HTTP_CLOSE};
    
    std::string buf;
    EventType event;
    EventType revent;
    CallBack read_fn;
    CallBack write_fn;

  public:
    Client(const int &fd_):fd(fd_) {};
    void set_event(const EventType &e){ event = e;}
    void set_revent(const EventType &e){ revent = e;}
    EventType get_event() const { return event; }
    int get_fd() const { return fd; }
    void set_read_fn(const CallBack &fn) { read_fn = fn; }
    void set_write_fn(const CallBack &fn) { read_fn = fn; }
    void close();
    void handle_event();
};
class HttpClient: public Client {
  private:

    HTTP_CODE http_read();
    HTTP_CODE http_write();
  public:
    HttpClient(const int &fd_);

};