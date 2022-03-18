#include "Client.h"
#include<cstring>
/**
 * @brief 根据revent调用回调函数
 * 
 */
void Client::handle_event()
{
  if (revent & EPOLLERR)
  {
    perror("poll error");
    is_close = true;
    return;
  }
  else if (revent & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    read_fn();
  else if (revent & EPOLLOUT)
    write_fn();
  if (is_close && close_fn)
    close_fn();
  update_timenode();
}

void Client::update_event(EventType ev)
{
  epoll_event t;
  t.data.fd = fd;
  t.events = ev;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &t);
}


/**
 * @brief 读取所有接收到的数据
 * 
 * @param m_is_close 判断是否关闭连接了，m_is_close=true为关闭
 * @return int 受到的数据长度
 */
int Client::read_all(bool &m_is_close)
{
  char buf[BUF_SIZE];
  int n = 0, sum = 0;
  while((n = read(fd, buf, BUF_SIZE - 1)) > 0)
  {
    buf[n] = '\0';
    client_buf += buf;
    sum += n;
  }
  if (n == 0)
    m_is_close = true;
  else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return sum;
  else
    return -1;
}