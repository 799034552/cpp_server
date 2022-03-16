/**
 * @file ReflexClient.h
 * @brief tcp回射服务
 * 
 */
#pragma once
#include"Client.h"

class ReflexClient: public Client {
  private:
    void reflex_read();
    void reflex_write();
  public:
    ReflexClient(const int &fd_);

};
using SP_ReflexClient = std::shared_ptr<ReflexClient>;