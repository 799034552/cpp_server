/**
 * @file HttpClient.h
 * @brief http服务
 * 
 */
#pragma once
#include"Client.h"
class HttpClient: public Client {
  private:
    enum class HTTP_CODE {HTTP_OPEN = 0, HTTP_BAD, HTTP_OK, HTTP_CLOSE};
    bool isClose;
    HTTP_CODE http_read();
    HTTP_CODE http_write();
  public:
    HttpClient(const int &fd_);

};
using SP_HttpClient = std::shared_ptr<HttpClient>;