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
using U_M = std::unordered_map<string,string>;
class Req {
  public:
    bool is_json;
    std::unordered_map<string,string> *data;
    string *text;
    Req() = default;
    void set(bool is_json_, std::unordered_map<string,string> *data_, string *text_)
      {is_json = is_json_; data = data_; text =text_;}
};

class Res {
  private:
    string send_buf;
    U_M &header;
  public:
    void send(const string &s) { send_buf += s; }
    void add_header(const string &a, const string &b) {header[a] = b;}
    string get_buf() const { return send_buf; }
    Res(U_M &header_):header(header_){}
};

class HttpClient: public Client {
  private:
    enum class HTTP_STATE {HTTP_OPEN = 0, HTTP_BAD, HTTP_OK, HTTP_CLOSE, WS};
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
    HTTP_STATE parse_body();
    void read_to_send(const Res&);
    void reset();
    int get_line(string &line);
    bool is_json;
    bool try_parse_json(const string &s, std::unordered_map<string,string> &res);
    void handle_ws_connect();
  public:
    static std::map<string, std::function<void(Req&, Res&)>> get_progress;
    static std::map<string, std::function<void(Req&, Res&)>> post_progress;
    HttpClient(const int &fd_);
};
using SP_HttpClient = std::shared_ptr<HttpClient>;