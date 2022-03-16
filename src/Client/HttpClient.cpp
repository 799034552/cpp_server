#include"HttpClient.h"
HttpClient::HttpClient(const int &fd_):
  Client(fd_),
  parse_state(PARSE_STATE::URL),
  read_close(false),
  http_state(HTTP_STATE::HTTP_OPEN)
{
  read_fn = std::bind(&HttpClient::http_read, this);
  write_fn = std::bind(&HttpClient::http_write, this);
  set_event(EPOLLIN | EPOLLET);
};

void HttpClient::http_read()
{
  int n = read_all(read_close);
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
    
      default:
        break;
    }
    if (http_state == HTTP_STATE::HTTP_BAD) {
      is_close = true;
      break;
    }
  }
  if (read_close)
    update_event(EPOLLOUT|EPOLLET);

}
HttpClient::HTTP_STATE HttpClient::parse_url(const string &line)
{
  auto line_res = split_const(line, {' ','\t'});
  if (line_res.size() == 3) {
    http_data["method"] = line_res[0];
    http_data["url"] = line_res[1];
    http_data["version"] = line_res[2];
    parse_state = PARSE_STATE::HEAD;
    HTTP_STATE::HTTP_OPEN;
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

}