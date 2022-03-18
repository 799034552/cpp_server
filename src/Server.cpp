#include"Pack.h"
#include<netinet/in.h>
#include"Server.h"
#include"util.h"
#include<unistd.h>
//#include"ReflexClient.h"
Server::Server():main_thread(new Thread())
{

}
void Server::listen(int port_, std::function<void()> fn,int thread_num)
{
  thread_pool = std::unique_ptr<ThreadPool>(new ThreadPool(thread_num));
  listenfd = Socket();
  // 跳过wait_time直接关闭，仅调试时开启
  #ifdef DEBUG
  int temp = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(temp));
  #endif
  Bind(listenfd, port_);
  ::listen(listenfd, 2048);
  appcet_client = std::make_shared<Client>(listenfd);
  appcet_client->set_event(EPOLLIN);
  appcet_client->set_read_fn(std::bind(&Server::get_accept, this));
  main_thread->add_client(appcet_client, false);
  if (fn) fn();
}

void Server::run()
{
  thread_pool->run();
  main_thread->run();
}

void Server::get_accept()
{
  int connfd;
  if ((connfd = accept(listenfd, NULL, NULL)) < 0)
  {
    perror("accept fail");
    return;
  }
  printf("main thread i get connect\n");
  //SP_ReflexClient client = SP_ReflexClient(new ReflexClient(connfd));
  SP_HttpClient client = SP_HttpClient(new HttpClient(connfd));
  SP_Thread thr = thread_pool->get_next_thread();
  thr->append_job(std::bind(&Thread::add_client, thr, client, true));
}


int Server::Socket()
{
  int fd;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("create listen socket fail"),exit(0);
  return fd;
}

void Server::Bind(int __fd, int port)
{
  sockaddr_in addr;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if ( bind(__fd, (sockaddr*)&addr, sizeof(addr)) < 0)
    perror("listen fail"),exit(0);
}

void Server::get(const string& url, const std::function<void(Req&, Res&)> &fn)
{
  HttpClient::get_progress[url]=fn;
}

void Server::post(const string& url, const std::function<void(Req&, Res&)> &fn)
{
  HttpClient::post_progress[url]=fn;
}