// filename: test.c
#define _GNU_SOURCE
#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>

// 用来打印当前的线程信息：调度策略是什么？优先级是多少？
void get_thread_info(const int thread_index)
{
    int policy;
    struct sched_param param;

    printf("\n====> thread_index = %d \n", thread_index);

    pthread_getschedparam(pthread_self(), &policy, ¶m);
    if (SCHED_OTHER == policy)
        printf("thread_index %d: SCHED_OTHER \n", thread_index);
    else if (SCHED_FIFO == policy)
        printf("thread_index %d: SCHED_FIFO \n", thread_index);
    else if (SCHED_RR == policy)
        printf("thread_index %d: SCHED_RR \n", thread_index);

    printf("thread_index %d: priority = %d \n", thread_index, param.sched_priority);
}

// 线程函数，
void *thread_routine(void *args)
{
    // 参数是：线程索引号。四个线程，索引号从 1 到 4，打印信息中使用。
    int thread_index = *(int *)args;
    
    // 为了确保所有的线程都创建完毕，让线程睡眠1秒。
    sleep(1);

    // 打印一下线程相关信息：调度策略、优先级。
    get_thread_info(thread_index);

    long num = 0;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 5000000; j++)
        {
            // 没什么意义，纯粹是模拟 CPU 密集计算。
            float f1 = ((i+1) * 345.45) * 12.3 * 45.6 / 78.9 / ((j+1) * 4567.89);
            float f2 = (i+1) * 12.3 * 45.6 / 78.9 * (j+1);
            float f3 = f1 / f2;
        }
        
        // 打印计数信息，为了能看到某个线程正在执行
        printf("thread_index %d: num = %ld \n", thread_index, num++);
    }
    
    // 线程执行结束
    printf("thread_index %d: exit \n", thread_index);
    return 0;
}

void main(void)
{
    // 一共创建四个线程：0和1-实时线程，2和3-普通线程(非实时)
    int thread_num = 4;
    
    // 分配的线程索引号，会传递给线程参数
    int index[4] = {1, 2, 3, 4};

    // 用来保存 4 个线程的 id 号
    pthread_t ppid[4];
    
    // 用来设置 2 个实时线程的属性：调度策略和优先级
    pthread_attr_t attr[2];
    struct sched_param param[2];

    // 实时线程，必须由 root 用户才能创建
    if (0 != getuid())
    {
        printf("Please run as root \n");
        exit(0);
    }

    // 创建 4 个线程
    for (int i = 0; i < thread_num; i++)
    {
        if (i <= 1)    // 前2个创建实时线程
        {
            // 初始化线程属性
            pthread_attr_init(&attr[i]);
            
            // 设置调度策略为：SCHED_FIFO
            pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
            
            // 设置优先级为 51，52。
            param[i].__sched_priority = 51 + i;
            pthread_attr_setschedparam(&attr[i], ¶m[i]);
            
            // 设置线程属性：不要继承 main 线程的调度策略和优先级。
            pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
            
            // 创建线程
            pthread_create(&ppid[i], &attr[i],(void *)thread_routine, (void *)&index[i]);
        }
        else        // 后两个创建普通线程
        {
            pthread_create(&ppid[i], 0, (void *)thread_routine, (void *)&index[i]);
        }
        
    }

    // 等待 4 个线程执行结束
    for (int i = 0; i < 4; i++)
        pthread_join(ppid[i], 0);

    for (int i = 0; i < 2; i++)
        pthread_attr_destroy(&attr[i]);
}
