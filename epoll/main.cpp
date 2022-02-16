#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "threadPool.h"
#include "locker.h"
#include <signal.h>
#include "http_conn.h"


const int MAX_FD = 65535; //最大文件描述符个数
const int MAX_EVENT_NUMBER = 10000; // 监听的最大的事件数量



// 添加文件描述符
extern void addfd( int epollfd, int fd, bool one_shot );
extern void removefd( int epollfd, int fd );
extern void modfd(int epollfd, int fd, int ev);

//信号捕捉
void addsig(int sig, void (handler)(int)){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig,&sa, NULL);
}

int main(int argc, char * argv[]){
    if(argc <= 1){
        std::cout<<"Please enter port_number"<<std::endl;
        exit(-1);
    }
    int port = atoi(argv[1]);


    // 对 SIGPIE作处理
    addsig(SIGPIPE,SIG_IGN);

    //初始化线程池
    threadpool<http_conn> *pool = nullptr;
    try
    {
        pool = new threadpool<http_conn>();
    }
    catch(const std::exception& e)
    {
        exit(-1);
    }

    //数组用于保存所有客户端信息
    http_conn *users = new http_conn[MAX_FD];

    // 网络代码
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);

    //端口复用
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //绑定
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&address, sizeof(address));


    //监听，第二个参数是未完成队列的大小， 及时拒绝掉一部分处理不过来的请求。防止盲等待，
    //如果开启了syncookies  忽略listen的第二个参数
    //有说是完成队列和未完成队列之和的，反正记住是是有两个队列，分别对应SYN_RECV和ESTABLISHED
    listen(listenfd,5);

    //创建epoll对象,第二个参数没啥用，但要大于0
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);

    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    while(true){
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1);
        if((number < 0) && (errno != EINTR)){// EINTR 中断
            std::cout<<"epoll failure"<<std::endl;
            break;
        }
        for(int i = 0 ; i < number; ++i){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *)&client_address,&client_addrlength);

                if(connfd < 0){
                    std::cout<<"errno is"<<errno<<std::endl;
                    continue;
                }
                if( http_conn::m_user_count >= MAX_FD ) {
                    close(connfd);
                    continue;
                }
                //文件描述符做索引，不关闭就递增，关闭就替换；
                users[connfd].init( connfd, client_address);
            } else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                // 异常断开事件
                users[sockfd].close_conn();

            } else if(events[i].events & EPOLLIN) {

                if(users[sockfd].read()) {
                    pool->append(users + sockfd);
                } else {
                    users[sockfd].close_conn();
                }

            }  else if( events[i].events & EPOLLOUT ) {

                if( !users[sockfd].write() ) {
                    users[sockfd].close_conn();
                }

            }
        }
    }

    close( epollfd );
    close( listenfd );
    delete [] users;
    delete pool;
    return 0;
}