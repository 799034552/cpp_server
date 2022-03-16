#include"HttpClient.h"
HttpClient::HttpClient(const int &fd_):Client(fd_)
{
  read_fn = std::bind(&HttpClient::http_read, this);
  write_fn = std::bind(&HttpClient::http_write, this);
  set_event(EPOLLIN | EPOLLET);
};

HttpClient::HTTP_CODE HttpClient::http_read()
{
  return HTTP_CODE::HTTP_OK;
}

HttpClient::HTTP_CODE HttpClient::http_write()
{
  return HTTP_CODE::HTTP_OK;
}