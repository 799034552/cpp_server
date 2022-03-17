#include"HttpClient.h"
#include<iostream>
using std::cout;
using std::endl;

std::map<string, std::function<void(Req&, Res&)>> HttpClient::get_progress;
std::map<string, std::function<void(Req&, Res&)>> HttpClient::post_progress;

HttpClient::HttpClient(const int &fd_):
  Client(fd_),
  parse_state(PARSE_STATE::URL),
  read_close(false),
  http_state(HTTP_STATE::HTTP_OPEN)
{
  read_fn = std::bind(&HttpClient::http_read, this);
  write_fn = std::bind(&HttpClient::http_write, this);
  set_event(EPOLLIN | EPOLLET);

  send_first= "HTTP/1.1 200 OK";
  header["Connection"]="Keep-Alive";
  header["Keep-Alive"]="timeout=20000";
  header["Content-Type"]="text/plain";
  header["Server"]="string\'s server";

};

void HttpClient::http_read()
{
  int n = read_all(read_close);
  cout<<"i got this raw----------------------------------------"<<endl;
  cout<<client_buf<<endl;
  if (n < 0) {
    is_close = true;
    return;
  }
  else if (n == 0 && !read_close)
    return;


  auto split_res = split(client_buf, "\r\n");

  //开始解析
  for(const auto &line : split_res)
  {
    switch (parse_state)
    {
      case PARSE_STATE::URL:
        http_state = parse_url(line);
        break;
      
      case PARSE_STATE::HEAD:
        http_state = parse_head(line);
        break;

      case PARSE_STATE::BODY:
        http_state = parse_body(line);
        break;
    
      default:
        break;
    }
    if (http_state == HTTP_STATE::HTTP_BAD) { //错误就关闭
      is_close = true;
      break;
    } else if (http_state == HTTP_STATE::HTTP_OK) {
      break;
    }
  }
  if (read_close || http_state == HTTP_STATE::HTTP_OK)
    update_event(EPOLLOUT|EPOLLET);
  cout<<"i got this----------------------------------------"<<endl;
  for(const auto &t: http_data)
    printf("%s:%s\n", t.first.c_str(), t.second.c_str());
  cout<<"http_state:"<<(http_state == HTTP_STATE::HTTP_OK)<<endl;

}
HttpClient::HTTP_STATE HttpClient::parse_url(const string &line)
{
  auto line_res = split_const(line, {' ','\t'});
  if (line_res.size() == 3) {
    http_data["method"] = line_res[0];
    http_data["version"] = line_res[2];

    auto s_res = split_const(line_res[1], '?');
    http_data["url"] = s_res[0];
    s_res = split_const(s_res[1], '&');
    for(const auto &temp: s_res){
      const auto m_res = split_const(temp, '=');
      get_data[m_res[0]] = m_res[1];
    }

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
      return HTTP_STATE::HTTP_OK;
    }
    parse_state = PARSE_STATE::BODY;
    return HTTP_STATE::HTTP_OPEN;
  }
  auto line_res = split_const(line, ':');
  if (line_res.size() > 1)
  {
    if (line_res[0] == "Connection")
      http_data["Connection"] = line_res[1];
    else if (line_res[0] == "Content-Length")
      http_data["Content-Length"] = line_res[1];
    else if (line_res[0] == "Content-Type")
      http_data["Content-Type"] = line_res[1];
    return HTTP_STATE::HTTP_OPEN;
  }
  else
    return HTTP_STATE::HTTP_BAD;
}
HttpClient::HTTP_STATE HttpClient::parse_body(const string &)
{

}


void HttpClient::http_write()
{
  if (send_buf.size() == 0)
  {
    client_buf.clear();
    Req req = {get_data};
    Res res;
    string url = http_data["url"];
    if(http_data["method"] == "GET")
    {
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
      if (be == en)
        res.send("404 not found");
    }
    read_to_send(res);
    cout<<"i send this----------------------------------------"<<endl;
    printf("send:\n%s\n",send_buf.c_str());
  }

  int n = write(fd, &send_buf[0], send_buf.size());
  if (n > 0) //发送成功一部分
  {
    if(n == send_buf.size())
    {
      send_buf.clear();
      update_event(EPOLLIN|EPOLLET);
      parse_state = PARSE_STATE::URL;
      http_state = HTTP_STATE::HTTP_OPEN;
      http_data.clear();
    }
    else 
      send_buf = send_buf.substr(n);
  } else if (n < 0)
    is_close = true;

}

void HttpClient::read_to_send(const Res& res)
{
  send_buf.clear();
  send_buf += send_first + "\r\n";
  header["Content-Length"] = std::to_string(res.get_buf().size());
  for(const auto &temp: header)
    send_buf += (temp.first+ ":" + temp.second + "\r\n");
  send_buf += "\r\n";
  send_buf += res.get_buf();
}