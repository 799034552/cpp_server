#pragma once
#include<pthread.h>
class Mutex {
  private:
    pthread_mutex_t mutex;
  public:
   Mutex() {
     pthread_mutex_init(&mutex, NULL);
   }
   ~Mutex() {
     pthread_mutex_destroy(&mutex);
   }
   void lock() { pthread_mutex_lock(&mutex); }
   void unlock() { pthread_mutex_unlock(&mutex); }
};

class MutexGuard {
  private:
    Mutex &m;
  public:
    MutexGuard(Mutex &m_):m(m_) { m.lock(); }
    ~MutexGuard() { m.unlock(); }
};