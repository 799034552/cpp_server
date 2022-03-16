#pragma once
#include"ThreadPool.h"
#include"Thread.h"
#include"util.h"
#include"Client.h"
#include<sys/socket.h>
#include<memory>
#define DEBUG
class Server {
  private:
    int port; //端口
    int listenfd; //监听描述符
    std::unique_ptr<ThreadPool> thread_pool; //线程池
    std::shared_ptr<Thread> main_thread; //主线程
    std::shared_ptr<Client> appcet_client;
    int Socket();
    void Bind(int fd, int port);
    void get_accept();
  public:
    //Server(int port_, int thread_num);
    Server();
    void listen(int port_, std::function<void()> fn = NULL,int thread_num = 4);
    void run();


};