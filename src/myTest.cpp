#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "myAllocator.h"

#define N 100000

using namespace std;

vector<int, MyAllocator<int>>* vecs[N];//��ʹ��Ĭ�ϵķ����������Զ��������

int bigr()//����һ�����ֵ
{
    return rand()%10*100000000 + rand()%10000*10000 + rand()%10000;
}

int main()
{
    int clk;
    
    clk = clock();//����ʱ��
    for (int i = 0; i < N; i++)
        vecs[i] = new vector<int, MyAllocator<int>>(bigr() % 1000 + 1);//����N��������ȵ�vector

    printf("%.3f\n", 1.0 * (clock() - clk) / CLOCKS_PER_SEC);//�����ڴ��ʱ��
    
    clk = clock();
    for (int i = 0; i < N; i++)
        vecs[bigr()%N]->resize(bigr() % 1000 + 1);//N�����µ�����С
    printf("%.3f\n", 1.0 * (clock() - clk) / CLOCKS_PER_SEC);//���µ�����С��ʱ��

    clk = clock();
    for (int i = 0; i < N; i++)
        delete vecs[i];//�ͷ��ڴ�
    printf("%.3f\n", 1.0 * (clock() - clk) / CLOCKS_PER_SEC);//�ͷ��ڴ��ʱ��

    return 0;
}
