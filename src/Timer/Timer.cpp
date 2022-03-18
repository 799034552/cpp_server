#include"Timer.h"
#include<iostream>
SP_TimeNode Timer::add_time_node(SP_Client client, time_t exp_time)
{
  SP_TimeNode temp = SP_TimeNode(new TimeNode());
  temp->client = client;
  temp->exp_time = exp_time;
  time_quene.push(temp);
  return temp;
}

/**
 * @brief 非活跃连接检查
 * 
 */
void Timer::check()
{
  time_t now_time = time(0);
  std::vector<SP_TimeNode> update_list;
  while (!time_quene.empty())
  {
    auto &time_node = time_quene.top();
    auto client = time_node->client.lock();
    if (!client) {
      time_quene.pop();
      continue;
    }
      
    if (time_node->is_delete) //已经删除
    {
      if (client && client->close_fn)
        client->close_fn();
    }
    else if(time_node->is_update) //更新过
    {
      time_node->is_update = false;
      time_node->exp_time = now_time + client->live_time;
      update_list.push_back(time_node);
    }
    else if (time_node->exp_time > now_time)
      break;
    else
    {
      client->close_fn();
      // std::cout<<"主动删除"<<std::endl;
    }
    time_quene.pop();
  }
  for(const auto &t:update_list)
  {
    time_quene.push(t);
  }
}

void Client::set_time_node(const std::shared_ptr<TimeNode> &t) {time_node = t;}
void Client::update_timenode() {
  if (time_node != nullptr)
    time_node->is_update = true;
}