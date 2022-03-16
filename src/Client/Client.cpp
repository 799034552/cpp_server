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
/**
 * @brief 读取所有接收到的数据
 * 
 * @return int 接受到的数据长度，出错为-1
 */
int Client::read_all()
{

}