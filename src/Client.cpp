#include "Client.h"
/**
 * @brief 根据revent调用回调函数
 * 
 */
void Client::handle_event()
{
  if (revent & EPOLLERR)
  {
    perror("poll error");
    close();
    return;
  }
  else if (revent & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    read_fn();
  else if (revent & EPOLLOUT)
    write_fn();
}

void Client::close()
{

}

HttpClient::HttpClient(const int &fd_):Client(fd_)
{
  read_fn = std::bind(&HttpClient::http_read, this);
  write_fn = std::bind(&HttpClient::http_write, this);
}

HttpClient::HTTP_CODE HttpClient::http_read()
{
  char t_buf[1024] = {};
  int n;
  while((n = read(fd, t_buf, 1024)) > 0)
  {
    buf += t_buf;
  }
  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      set_event(EPOLLOUT | EPOLLET);
      return HTTP_CODE::HTTP_OK;
    }
    else
      return HTTP_CODE::HTTP_BAD;
  } else {
    return HTTP_CODE::HTTP_CLOSE;
  }
  
}

HttpClient::HTTP_CODE HttpClient::http_write()
{
  write(fd, &buf[0], buf.size());
  set_event(EPOLLIN | EPOLLET);
  return HTTP_CODE::HTTP_OK;
}
