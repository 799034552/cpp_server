#include"WSPool.h"

std::function<void(std::shared_ptr<WSClient>)> WSPool::connect_fn = nullptr;