#include"HttpClient.h"
#include "document.h"
#include"WSPool.h"
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include"WSClient.h"
#include<iostream>
#include<fstream>
using std::cout;
using std::endl;
using std::ifstream;

std::map<string, std::function<void(Req&, Res&)>> HttpClient::get_progress;
std::map<string, std::function<void(Req&, Res&)>> HttpClient::post_progress;

/**
 * @brief 初始化httpclient
 * 
 * @param fd_ 
 */
HttpClient::HttpClient(const int &fd_):
  Client(fd_),
  parse_state(PARSE_STATE::URL),
  read_close(false),
  http_state(HTTP_STATE::HTTP_OPEN),
  is_json(false)
{
  read_fn = std::bind(&HttpClient::http_read, this);
  write_fn = std::bind(&HttpClient::http_write, this);
  set_event(EPOLLIN | EPOLLET);
  reset();
};

/**
 * @brief 重置状态，用于长连接
 * 
 */
void HttpClient::reset()
{
  send_buf.clear();
  parse_state = PARSE_STATE::URL;
  http_state = HTTP_STATE::HTTP_OPEN;
  http_data.clear();
  get_data.clear();
  send_first= "HTTP/1.1 200 OK";
  header.clear();
  header["Connection"]="Keep-Alive";
  header["Keep-Alive"]="timeout=20000";
  header["Content-Type"]="text/plain";
  header["Server"]="string\'s server";
  is_json = false;
}

/**
 * @brief http读事件
 * 
 */
void HttpClient::http_read()
{
  int n = read_all(read_close);
  // cout<<"i got this raw----------------------------------------"<<endl;
  // cout<<rec_buf<<endl;
  if (n < 0) {
    is_close = true;
    return;
  }
  else if (n == 0 && !read_close)
    return;
  
  // 开始解析
  string line;
  while((n = get_line(line))!= -1)
  {
    if (parse_state == PARSE_STATE::URL)
      http_state = parse_url(line);
    else if (parse_state == PARSE_STATE::HEAD)
      http_state = parse_head(line);
    else
      break;

    if (http_state == HTTP_STATE::HTTP_BAD) { //错误就关闭
      is_close = true;
      break;
    } else if (http_state == HTTP_STATE::HTTP_OK || parse_state == PARSE_STATE::BODY)
      break;
    else if (http_state == HTTP_STATE::WS) {
      //cout<<"Get ws---------------------------"<<endl;
      update_event(EPOLLOUT|EPOLLET);
      return;
    }
  }
  //cout<<"parse get over----------------"<<endl;
  if (parse_state == PARSE_STATE::BODY)
    http_state = parse_body();
  
  if (read_close || http_state == HTTP_STATE::HTTP_OK)
    update_event(EPOLLOUT|EPOLLET);
  
  //cout<<"i got this----------------------------------------"<<endl;
  //for(const auto &t: http_data)
  //  printf("%s:%s\n", t.first.c_str(), t.second.c_str());
  //cout<<"http_state:"<<(http_state == HTTP_STATE::HTTP_OK)<<endl;
}
HttpClient::HTTP_STATE HttpClient::parse_url(const string &line)
{
  //cout<<"parse url----------------"<<endl;
  auto line_res = split_const(line, {' ','\t'});
  
  if (line_res.size() == 3) {
    http_data["method"] = line_res[0];
    http_data["version"] = line_res[2];

    if (line_res[1].find("?") != string::npos)
    {
      auto s_res = split_const(line_res[1], '?');
      http_data["url"] = s_res[0];
      s_res = split_const(s_res[1], '&');
      //cout<<"parse url----------------"<<endl;
      for(const auto &temp: s_res){
        const auto m_res = split_const(temp, '=');
        get_data[m_res[0]] = m_res[1];
      }
    }
    else
      http_data["url"] = line_res[1];

    parse_state = PARSE_STATE::HEAD;
    return HTTP_STATE::HTTP_OPEN;
  }
  else
    return HTTP_STATE::HTTP_BAD;
  
}
HttpClient::HTTP_STATE HttpClient::parse_head(const string & line)
{
  if (line.size() == 0) {
    if (http_data["method"] == "GET") {
      if (http_data.find("Sec-WebSocket-Extensions") != http_data.end())
      {
        return HTTP_STATE::WS;
      }
      else {
        return HTTP_STATE::HTTP_OK;
      }
    }
    else if (http_data["method"] == "POST") {
      get_data.clear();
      parse_state = PARSE_STATE::BODY;
      return HTTP_STATE::HTTP_OPEN;
    } else
      return HTTP_STATE::HTTP_BAD;
  }
  auto line_res = split_const(line, ':');
  const vector<string> care_key= {"Connection", "Content-Length", "Content-Type",
                          "Sec-WebSocket-Extensions", "Sec-WebSocket-Key"
                          };
  if (line_res.size() > 1)
  {
    for(const auto &t : care_key)
    {
      if (line_res[0] == t)
        http_data[t] = line_res[1];
    }
    return HTTP_STATE::HTTP_OPEN;
  }
  else
    return HTTP_STATE::HTTP_BAD;
}
HttpClient::HTTP_STATE HttpClient::parse_body()
{
  if (rec_buf.size() < atoi(http_data["Content-Type"].c_str()))
    return HTTP_STATE::HTTP_OPEN;
  else {
    //cout<<"body is----------------------------\n";
    //cout<<rec_buf;
    if (http_data["Content-Type"] == "application/x-www-form-urlencoded") //这种类型才解析
    {
      const auto &s_res = split_const(rec_buf, '&');
      for(const auto &temp: s_res){
        const auto m_res = split_const(temp, '=');
        get_data[m_res[0]] = m_res[1];
      }
      is_json = true;
    }
    else if (try_parse_json(rec_buf, get_data))
    {
      is_json = true;
    }
    return HTTP_STATE::HTTP_OK;
  }
}

/**
 * @brief websocke握手
 * 
 */
void HttpClient::handle_ws_connect()
{
  
  char sha1_data[30] = {0};
  char sec_accept[32] = {0};
  string sha_in = http_data["Sec-WebSocket-Key"] + WSClient::GUID;
  SHA1((unsigned char*)&sha_in[0], sha_in.size(), (unsigned char*)sha1_data);
  //base64_encode(&sha1_data[0],strlen(sha1_data),sec_accept);
  string res = m_base64_encode((unsigned char *)&sha1_data, strlen(sha1_data));//sec_accept;
  send_buf = 
    "HTTP/1.1 101 Switching Protocols\r\n" 
    "Upgrade: websocket\r\n" 
    "Connection: Upgrade\r\n" 
    "Sec-WebSocket-Accept: "+ res +"\r\n\r\n";
}


void HttpClient::http_write()
{
  if (send_buf.size() == 0 && http_state != HTTP_STATE::WS)
  {
    Req req;
    Res res(header);
    req.set(is_json, &get_data, &rec_buf);
    string url = http_data["url"];
    if(http_data["method"] == "GET")
    {
      //cout<<url<<endl;
      auto en = get_progress.end();
      auto be = get_progress.begin();
      for(; be !=  en; ++be)
      {
        if (be->first == url)
        {
          be->second(req, res);
          break;
        }
      }
      
      if (be == en) //没有被捕获的页面
      {
        //cout<<url<<endl;
        char buf[1024];
        if (url == "/") url = "/index.html";
        string file_url = "www" + url;
        ifstream fs(file_url.c_str(), ifstream::in);
        //cout<<"url-----------------------"<<file_url<<endl;
        //cout<<"locate-----------------"<<getcwd(buf,1024)<<endl;
        file_url = "../" + file_url;
        if (fs.is_open() || (fs.open(file_url, ifstream::in),fs.is_open()))
        {
          while(std::getline(fs, file_url))
            res.send(file_url+"\n");
          res.add_header("Content-Type", "text/html");
          // for(auto t: header)
          //   //cout<<t.first<<" "<<t.second<<endl;
          fs.close();
        }
        else
          res.send("404 not found");

      }
        
    }
    else if (http_data["method"] == "POST" && ( //支持的类型
      http_data["Content-Type"] == "application/x-www-form-urlencoded"
      || http_data["Content-Type"] == "text/plain"
      || http_data["Content-Type"] == "application/json"
      || http_data["Content-Type"] == ""
      ))
    {
      auto en = post_progress.end();
      auto be = post_progress.begin();
      for(; be !=  en; ++be)
      {
        if (be->first == url)
        {
          be->second(req, res);
          break;
        }
      }
      if (be == en) //m没有对应的信息
      {
        send_first = "HTTP/1.1 404 Not Found!";
        res.send("404 Not Found");
      }
    }
    else {
      send_first = "HTTP/1.1 503 Service Unavailable";
      res.send("Service Unavailable");
    }
    read_to_send(res);
    rec_buf.clear();
    //cout<<"i send this----------------------------------------"<<endl;
    //printf("send:\n%s\n",send_buf.c_str());
  }
  else if (http_state == HTTP_STATE::WS && send_buf.size() == 0)
  {
    handle_ws_connect();
  }
  //cout<<"输出的是："<<send_buf<<endl;
  int n = write(fd, &send_buf[0], send_buf.size());
  if (n > 0) 
  {
    if(n == send_buf.size()) //发送成功
    {
      if (http_data["Connection"] != "Keep-Alive")
      {
        is_close = true;
        return;
      }
      update_event(EPOLLIN|EPOLLET);
      if (http_state == HTTP_STATE::WS)
      {
        if (WSPool::connect_fns.find(http_data["url"]) == WSPool::connect_fns.end()) // 没有对应的事件注册
        {
          is_close = true;
          return;
        }
        is_change = true;
        auto temp = std::make_shared<WSClient>(fd, http_data);
        WSPool::connect_fns[http_data["url"]](temp);
        change_to_client = temp;
        return;
      }
      else
      {
        reset();
      }
    }
    else  //发送成功一部分
      send_buf = send_buf.substr(n);
  } else if (n < 0)
    is_close = true;
  
  if (read_close)
    is_close = true;

}

void HttpClient::read_to_send(const Res& res)
{
  send_buf.clear();
  send_buf += send_first + "\r\n";
  header["Content-Length"] = std::to_string(res.get_buf().size());
  for(const auto &temp: header)
    send_buf += (temp.first+ ": " + temp.second + "\r\n");
  send_buf += "\r\n";
  send_buf += res.get_buf();
}

int HttpClient::get_line(string &line)
{
  int n;
  if((n = rec_buf.find("\r\n")) != string::npos)
  {
    line = rec_buf.substr(0, n);
    rec_buf.erase(0, n+2);
    return 0;
  }
  else
    return -1;
}

/**
 * @brief 解析json
 * 
 * @param s 输入字符串
 * @param res 输出的map
 * @return true 成功解析
 */
bool HttpClient::try_parse_json(const string &s, std::unordered_map<string,string> &res)
{
  try
  {
    rapidjson::Document d;
    d.Parse(s.c_str());
    if (!d.IsObject())
      return false;
    auto en = d.MemberEnd();
    for(auto be = d.MemberBegin(); be != en; ++be)
    {
      auto key = be->value.GetType();
      if (key != rapidjson::kNumberType
        && key != rapidjson::kStringType
        && key != rapidjson::kTrueType
        && key != rapidjson::kFalseType
      )
      {
        return false;
      }
    }

    for(auto be = d.MemberBegin(); be != en; ++be)
    {
      auto key = be->value.GetType();
      if (key == rapidjson::kNumberType)
      {
        string temp;
        if (be->value.IsInt())
          temp = std::to_string(be->value.GetInt());
        else if (be->value.IsFloat())
          temp = std::to_string(be->value.GetFloat());
        else
          return false;
        res[be->name.GetString()] = temp;
      }
      else if(key == rapidjson::kStringType)
        res[be->name.GetString()] = be->value.GetString();
      else if (key == rapidjson::kFalseType || key == rapidjson::kTrueType)
        res[be->name.GetString()] = be->value.GetBool()?"true":"false";
    }
    return true;
  }
  catch(const std::exception& e)
  {
    return false;
  }
}