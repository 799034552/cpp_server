#include"WSPool.h"

std::unordered_map<string, std::function<void(std::shared_ptr<WSClient>)>> WSPool::connect_fns;
string WSPool::pre_url;
std::function<void(std::shared_ptr<WSClient>)> WSPool::connect_fn;