/**
 * @file HttpClient.h
 * @brief http服务
 * 
 */
#pragma once
#include"Client.h"
#include<unordered_map>
using std::string;
class HttpClient: public Client {
  private:
    enum class HTTP_STATE {HTTP_OPEN = 0, HTTP_BAD, HTTP_OK, HTTP_CLOSE};
    enum class PARSE_STATE {URL = 0, HEAD, BODY};
    PARSE_STATE parse_state;
    HTTP_STATE http_state;

    bool read_close; // 写关闭
    
    void http_read();
    void http_write();
    std::unordered_map<string, string> http_data;
    HTTP_STATE parse_url(const string &);
    HTTP_STATE parse_head(const string &);
    HTTP_STATE parse_body(const string &);
  public:
    HttpClient(const int &fd_);

};
using SP_HttpClient = std::shared_ptr<HttpClient>;