#pragma once
#include<fcntl.h>
#include<vector>
#include<string>
#include<algorithm>
using std::vector;
using std::string;
void set_fd_noblock(int &fd);
vector<string> split(string &s, const char *sp);
vector<string> split_const(const string &s, std::initializer_list<const char> sp);
vector<string> split_const(const string &s, const char sp);