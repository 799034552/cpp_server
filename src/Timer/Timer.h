#include<functional>
#include<vector>
#include<queue>
#include<time.h>
#include<memory>
#include"Client.h"
using std::function;
using CallBack=function<void()>;
using SP_Client = std::shared_ptr<Client>;
class TimeNode {
  public:
    time_t exp_time;
    std::weak_ptr<Client> client;
    bool is_update;
    bool is_delete;
    TimeNode():is_update(false),is_delete(false){}
};
using SP_TimeNode = std::shared_ptr<TimeNode>;

struct cmp_TimeNode{
	bool operator()(const SP_TimeNode &a, const SP_TimeNode &b){
    return a->exp_time > b->exp_time;
	}
};
class Timer {
  private:
    std::priority_queue<SP_TimeNode, std::deque<SP_TimeNode>, cmp_TimeNode> time_quene;
  public:
    SP_TimeNode add_time_node(SP_Client client, time_t exp_time);
    void check();
};