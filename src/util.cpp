#include "util.h"
/**
 * @brief Set the fd noblock object
 * 
 * @param fd 
 */
void set_fd_noblock(int &fd)
{
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

/**
 * @brief 分割字符串,返回分割好的，修改输入的
 * 
 * @param s 
 * @param sp 
 * @return vector<string> 
 */
vector<string> split(string &s, const char *sp)
{
  vector<string> res;
  std::size_t be = 0, next = 0;
  auto be_it = s.begin();
  while((next = s.find(sp, be)) != string::npos)
  {
    res.emplace_back(be_it + be, be_it + next);
    be = next+2;
  }
  s.assign(be_it + be, s.end());
  return res;
}

void trim(string &s)
{
  auto a = s.begin();
  s.assign(a + s.find_first_not_of(' '),a + s.find_last_not_of(' ')+1);
}
vector<string> split_const(const string &s, std::initializer_list<const char> sp)
{
  vector<string> res;
  std::size_t be = 0, next = 0;
  auto be_it = s.begin();

  next = string::npos;
  for(const auto &t: sp)
      next = std::min(s.find(t, be), next);
  while(next != string::npos)
  {
    if (be != next)
      res.emplace_back(be_it + be, be_it + next);
    be = next+1;
    next = string::npos;
    for(const auto &t: sp)
        next = std::min(s.find(t, be), next);
  }
  if (be != s.size())
    res.emplace_back(be_it + be, s.end());

  std::for_each(res.begin(), res.end(), trim);
  return res;
}
vector<string> split_const(const string &s, const char sp)
{
  vector<string> res;
  std::size_t be = 0, next = 0;
  auto be_it = s.begin();

  while((next = s.find(sp, be)) != string::npos)
  {
    if (be != next)
      res.emplace_back(be_it + be, be_it + next);
    be = next+1;
  }
  if (be != s.size())
    res.emplace_back(be_it + be, s.end());

  std::for_each(res.begin(), res.end(), trim);
  return res;
}