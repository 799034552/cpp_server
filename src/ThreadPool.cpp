#include"ThreadPool.h"
#include<memory>
void * start_thread(void * arg)
{
  SP_Thread thread = *(SP_Thread*)arg;
  thread->run();
}
ThreadPool::ThreadPool(int thread_n):cur_thr(0)
{
  while(thread_n--)
    thread_poll.push_back(SP_Thread(new Thread()));
}

void ThreadPool::run()
{
  pthread_t p;
  int i = 0;
  for(SP_Thread &thread : thread_poll) {
    thread->tid = ++i;
    pthread_create(&p, NULL, start_thread, &thread);  
    pthread_detach(p);
  }
}

SP_Thread ThreadPool::get_next_thread()
{
  return thread_poll[(cur_thr++) % thread_poll.size()];
}