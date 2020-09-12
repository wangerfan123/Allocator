#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "myAllocator.h"

#define N 100000

using namespace std;

vector<int, MyAllocator<int>>* vecs[N];//不使用默认的分配器而是自定义分配器

int bigr()//返回一个随机值
{
    return rand()%10*100000000 + rand()%10000*10000 + rand()%10000;
}

int main()
{
    int clk;
    
    clk = clock();//返回时间
    for (int i = 0; i < N; i++)
        vecs[i] = new vector<int, MyAllocator<int>>(bigr() % 1000 + 1);//分配N段随机长度的vector

    printf("%.3f\n", 1.0 * (clock() - clk) / CLOCKS_PER_SEC);//分配内存的时间
    
    clk = clock();
    for (int i = 0; i < N; i++)
        vecs[bigr()%N]->resize(bigr() % 1000 + 1);//N段重新调整大小
    printf("%.3f\n", 1.0 * (clock() - clk) / CLOCKS_PER_SEC);//重新调整大小的时间

    clk = clock();
    for (int i = 0; i < N; i++)
        delete vecs[i];//释放内存
    printf("%.3f\n", 1.0 * (clock() - clk) / CLOCKS_PER_SEC);//释放内存的时间

    return 0;
}
