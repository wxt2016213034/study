#include <iostream>
#include <thread>
#include <pthread>

void get_thread_info(const int thread_index)
{
    int policy;
    struct sched_param param;

    printf("\n====> thread_index = %d \n", thread_index);

    pthread_getschedparam(pthread_self(), &policy, &param);
    if (SCHED_OTHER == policy)
        printf("thread_index %d: SCHED_OTHER \n", thread_index);
    else if (SCHED_FIFO == policy)
        printf("thread_index %d: SCHED_FIFO \n", thread_index);
    else if (SCHED_RR == policy)
        printf("thread_index %d: SCHED_RR \n", thread_index);

    printf("thread_index %d: priority = %d \n", thread_index, param.sched_priority);
}


void hello(){
    std::cout<<"hello"<<std::endl;
    get_thread_info(0);
}

int main(){
    std::thread thread1(hello);
    thread1.join();
    return 0;
}