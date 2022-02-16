#include <iostream>
#include "locker.h"
#include "threadPool.h"
using namespace std;
class request{
    public:
        void process(){
            cout<<"work"<<endl;
        }
};

int main(){
    request t1;
    request t2;
    request t3;
    request t4;
    threadpool<request> temp;
    temp.append(&t1);
    temp.append(&t2);
    temp.append(&t3);
    temp.append(&t4);
    while(1);
}