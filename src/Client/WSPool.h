#pragma once
#include<functional>
#include"WSClient.h"
#include<memory>
#include<unordered_map>
#include<iostream>
using namespace std;
class WSPool {
  public:
    static string pre_url;
    static std::unordered_map<string, std::function<void(std::shared_ptr<WSClient>)>> connect_fns;
    static std::function<void(std::shared_ptr<WSClient>)> connect_fn;
    void on_connect(const std::function<void(std::shared_ptr<WSClient>)> & fn){
      if (pre_url.size() != 0)
        //cout<<"addd fn----------------"<<endl;
        connect_fns[pre_url] = fn;
        connect_fn = fn;
      };

};