#include <mutex>
#include <iostream>
#include <chrono>
#include <cstring>
#include <thread>
#include <sched.h>
#include <pthread.h>
#include<unistd.h>
// std::mutex iomutex;
void f(int num)
{
    cpu_set_t mask;
    int cpus = sysconf(_SC_NPROCESSORS_CONF);
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        printf("set thread affinity failed! \n");
    }
    std::cout<<"start thread"<<num<<std::endl;
    sched_param sch;
    int policy; 
    sched_param sch1;
    sch1.sched_priority = 70;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch1)) {
        std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';
    }
    int count = 0;
    while(1){
    ++count;
    pthread_getschedparam(pthread_self(), &policy, &sch);
    // std::lock_guard<std::mutex> lk(iomutex);
    std::cout << "Thread " << num << " is executing at priority "
              << sch.sched_priority << " count:"<<count<<'\n';
    std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
void f2(int num)
{
    cpu_set_t mask;
    int cpus = sysconf(_SC_NPROCESSORS_CONF);
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        printf("set thread affinity failed! \n");
    }
    std::cout<<"start thread"<<num<<std::endl;
    sched_param sch;
    int policy;
    int count = 0;
    while(1){
    ++count;
    pthread_getschedparam(pthread_self(), &policy, &sch);
    // std::lock_guard<std::mutex> lk(iomutex);
    std::cout << "Thread " << num << " is executing at priority "
              << sch.sched_priority << " count:"<<count<<'\n';
    std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
 
int main()
{
    std::thread t1(f, 1), t2(f2, 2);
    
    sched_param sch;
    int policy; 
    pthread_getschedparam(t2.native_handle(), &policy, &sch);
    sch.sched_priority = 20;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout<<"set"<<std::endl;
    if (pthread_setschedparam(t2.native_handle(), SCHED_FIFO, &sch)) {
        std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';
    }
    std::cout<<"set down"<<std::endl;
    t1.join(); t2.join();
}