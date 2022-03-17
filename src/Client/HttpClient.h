/**
 * @file HttpClient.h
 * @brief http服务
 * 
 */
#pragma once
#include"Client.h"
#include<unordered_map>
#include<map>
using std::string;
struct Req {
  std::unordered_map<string,string> &data;
};

class Res {
  private:
    string send_buf;
  public:
    void send(const string &s) { send_buf += s; }
    string get_buf() const { return send_buf; }
};

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
    std::unordered_map<string, string> get_data;
    string send_first;
    std::unordered_map<string, string> header;
    HTTP_STATE parse_url(const string &);
    HTTP_STATE parse_head(const string &);
    HTTP_STATE parse_body(const string &);
    void read_to_send(const Res&);
  public:
    static std::map<string, std::function<void(Req&, Res&)>> get_progress;
    static std::map<string, std::function<void(Req&, Res&)>> post_progress;
    HttpClient(const int &fd_);
};
using SP_HttpClient = std::shared_ptr<HttpClient>;