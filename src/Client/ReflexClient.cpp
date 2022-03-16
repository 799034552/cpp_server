#include"ReflexClient.h"

ReflexClient::ReflexClient(const int &fd_):Client(fd_)
{
  read_fn = std::bind(&ReflexClient::reflex_read, this);
  write_fn = std::bind(&ReflexClient::reflex_write, this);
  set_event(EPOLLIN | EPOLLET);
}

void ReflexClient::reflex_read()
{
  char t_buf[1024] = {};
  int n;
  while((n = read(fd, t_buf, 1023)) > 0)
  {
    t_buf[n]='\0';
    client_buf += t_buf;
  }
  printf("%s",client_buf.c_str());
  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      update_event(EPOLLOUT | EPOLLET);
    }
    else
      return ;
  } else {
    if (client_buf.size() == 0)
    {
      ::close(fd);
      is_close = true;
    }
    else
    {
      update_event(EPOLLOUT | EPOLLET);
    }
    return;
  }
  
}

void ReflexClient::reflex_write()
{
  int n = write(fd, &client_buf[0], client_buf.size());
  if (n > 0) 
  {
    if (n == client_buf.size())  //全部发送完成
    { 
      client_buf.clear();
      update_event(EPOLLIN | EPOLLET);
    }
    else //还没发送完
      client_buf = client_buf.substr(n);
  }
  return;
}

