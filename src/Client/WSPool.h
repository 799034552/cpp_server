#pragma once
#include<functional>
#include"WSClient.h"
#include<memory>
class WSPool {
  public:
    static std::function<void(std::shared_ptr<WSClient>)> connect_fn;
    void on_connect(const std::function<void(std::shared_ptr<WSClient>)> & fn){ connect_fn = fn;};

};