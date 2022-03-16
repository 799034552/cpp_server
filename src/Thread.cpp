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
  add_client(wakeup_client);

}
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
      Client &cur_cli = *fd2client[fd];
      cur_cli.set_revent(events[i].events);
      cur_cli.handle_event();
      update_client(cur_cli);
    }

  }
}

void Thread::update_client(const Client& client)
{
  epoll_event t_event;
  t_event.data.fd = client.get_fd();
  t_event.events = client.get_event();
  epoll_ctl(epollfd, EPOLL_CTL_MOD, client.get_fd(), &t_event);
}

void Thread::add_client(SP_Client sp_client)
{
  int fd = sp_client->get_fd();
  epoll_event t_event;
  t_event.data.fd = fd;
  t_event.events = sp_client->get_event();
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &t_event);
  fd2client[fd] = sp_client;
  ++client_num;
}

void Thread::delete_client(SP_Client sp_client)
{
  epoll_ctl(epollfd, EPOLL_CTL_DEL, sp_client->get_fd(), NULL);
  fd2client.erase(sp_client->get_fd());
}


void Thread::append_job(CallBack && fn)
{
  {
    MutexGuard lock(mutex);
    jobs_quene.push_back(fn);
  }
  char t_buf[1] = {};
  write(wakeup_fd, t_buf, sizeof(t_buf));
}

void Thread::handle_jobs()
{
  char t_buf[3];
  while((read(wakeup_fd, t_buf, sizeof(t_buf))) > 0);
  std::vector<CallBack> m_jobs_quene;
  {
    MutexGuard lock(mutex);
    m_jobs_quene.swap(jobs_quene);
  }
  for(auto &fn : m_jobs_quene)
    fn();
  update_client(*fd2client[wakeup_fd]);
}