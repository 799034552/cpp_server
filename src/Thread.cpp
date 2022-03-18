#include"Thread.h"
#include<stdio.h>
#include<sys/epoll.h>
#include<sys/eventfd.h>
Thread::Thread()
  :events(initial_client),
  client_num(0),
  tid(0),
  wakeup_fd(eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC)),
  epollfd(epoll_create1(0))
{
  SP_Client wakeup_client = std::make_shared<Client>(wakeup_fd);
  wakeup_client->set_event(EPOLLIN|EPOLLET);
  wakeup_client->set_read_fn(std::bind(&Thread::handle_jobs, this));
  add_client(wakeup_client, false);
}
/**
 * @brief 主要的循环，进程会卡在这里
 * 
 */
void Thread::run()
{
  int event_num = 0;
  #ifdef DEBUG
  printf("thread %d start\n", tid);
  #endif
  while(1)
  {
    if ((event_num = epoll_wait(epollfd, &events[0], events.size(), -1)) < 0)
      perror("epoll error");
    for(int i = 0; i < event_num; ++i)
    {
      int fd = events[i].data.fd;
      SP_Client cur_cli = fd2client[fd];
      cur_cli->set_revent(events[i].events);
      cur_cli->handle_event();
    }
    timer.check();
  }
}

void Thread::update_epoll(int fd, EventType ev)
{
  epoll_event t_event;
  t_event.data.fd = fd;
  t_event.events = ev;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &t_event);
}

void Thread::add_client(SP_Client sp_client, bool is_timeout)
{
  int fd = sp_client->get_fd();
  epoll_event t_event;
  t_event.data.fd = fd;
  t_event.events = sp_client->get_event();
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &t_event);
  fd2client[fd] = sp_client;
  sp_client->set_close_fn(std::bind(&Thread::delete_client, this, sp_client));
  sp_client->set_epollfd(epollfd);
  if (is_timeout)
    sp_client->set_time_node(timer.add_time_node(sp_client, time(0) + sp_client->live_time));
  ++client_num;
}

void Thread::delete_client(SP_Client sp_cli)
{
  epoll_ctl(epollfd, EPOLL_CTL_DEL, sp_cli->get_fd(), NULL);
  fd2client.erase(sp_cli->get_fd());
  --client_num;
}

void Thread::delete_client_by_fd(int fd)
{
  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
  fd2client.erase(fd);
  --client_num;
}


void Thread::append_job(CallBack && fn)
{
  {
    MutexGuard lock(mutex);
    jobs_quene.push_back(fn);
  }
  uint64_t  t_buf = 1;
  write(wakeup_fd, &t_buf, sizeof(t_buf));
}

void Thread::handle_jobs()
{
  uint64_t t_buf;
  (read(wakeup_fd, &t_buf, sizeof(t_buf)));
  std::vector<CallBack> m_jobs_quene;
  {
    MutexGuard lock(mutex);
    m_jobs_quene.swap(jobs_quene);
  }
  for(auto &fn : m_jobs_quene)
    fn();
  //update_client(*fd2client[wakeup_fd]);
}