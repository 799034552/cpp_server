#pragma once
#include"Client.h"
#include<unordered_map>
#include<string>
#include<functional>
using std::string;
int base64_encode(char *in_str, int in_len, char *out_str);
std::string m_base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
struct frame_head {
    char fin;
    char opcode;
    char mask;
    unsigned long long payload_length;
    char masking_key[4];
};

class WSClient : public Client,public std::enable_shared_from_this<WSClient>
{
  private:
    enum class READ_STATE {READ_OPEN = 0, READ_OK, READ_BAD}; 
    enum class PARSE_STATE {TWO_CHAR = 0, PAY_LOAD_Length, Masking_Key, PAY_LOAD};
    bool read_close; // 写关闭
    bool is_send_over;
    std::unordered_map<string, string> req_head;
    std::function<void(const string &)> get_fn;
    READ_STATE read_state;
    PARSE_STATE parse_state;
    void ws_read();
    void ws_write();
    READ_STATE read_head();
    frame_head f_head;
    void reset();
  public:
    string rubbish;
    static const string GUID;
    WSClient(int fd_,std::unordered_map<string, string> &http_data);
    void on_get(const std::function<void(string, std::shared_ptr<WSClient>)> & fn){
      get_fn = std::bind(fn, std::placeholders::_1, shared_from_this());
      }
    void emit(string);
  
};