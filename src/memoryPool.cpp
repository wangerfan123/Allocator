#include <vector>
#include "memoryPool.h"

// Get the single memory pool instance
// 内存池单例模式，返回静态的单例对象
MemoryPool& MemoryPool::getInstance()
{
    static MemoryPool instance;
    return instance;
}

// Constructor 构造函数
MemoryPool::MemoryPool()
{
    head = NULL;//头部为空
}

// Destructor 析构函数：将内存池中所有的可用的chunk全部进行析构释放
MemoryPool::~MemoryPool()
{
    if (head == NULL)//为空则不用析构
        return;
    
    std::vector<void*> toDelete;
    Chunk* chunk = head;
    
	//依次将所有chunk的指针放到vector数组中
    do
    {
        if (chunk->pre == NULL)
            toDelete.push_back(reinterpret_cast<void*>(chunk));
        chunk = chunk->nxtNode;
    }
    while(chunk != head);
    
	//调用operator delete 统一对vector中的指针进行删除
    for (void* ptr : toDelete)
        operator delete(ptr);
}

// Allocate block of storage 分配内存
MemoryPool::pointer MemoryPool::allocate(size_type size)//size是需要分配的内存空间的大小
{
    if (head == NULL || head->size < size)//如果链表头部的大小不能满足申请者的要求，再向操作系统申请一块新的内存
    {
        // Allocate new block if needed
        size_type realSize;//真实的大小
        data_pointer ptr = allocateBlock(size, realSize); //从OS申请了新的一块内存，为了加快申请速度，新block的大小被设置为一个常数 BLOCK_SIZE 的整数倍
        Chunk* chunk = reinterpret_cast<Chunk*>(ptr);//将这个指针转换成为chunk类型

        chunk->free = 1;//free标志位1代表尚未被使用
        chunk->pre = NULL;
        chunk->size = realSize - HEADER_SIZE - sizeof(BLOCK_END);//真实的大小为减去头部和BLOCK_END

        insertChunk(chunk, head);//将这个新的可用的chunk添加到chunk中待用
        head = chunk;//头部指针更新为chunk，因为这个新申请的肯定是最大的
    }
	//在head的大小足够的情况下
    pointer ret = reinterpret_cast<pointer>(
        reinterpret_cast<data_pointer>(head) + HEADER_SIZE
    );

    // Split head chunk into two chunks  把头部分为两部分，因为头部的大小可能大于size，前面size的用作分配，而后面的放入到链表中
    //这个chunk代表的是head除去了size之后剩下的chunk
	Chunk* chunk = reinterpret_cast<Chunk*>(
        reinterpret_cast<data_pointer>(head) + HEADER_SIZE + size
    );
    
    // Original next chunk 
	//这个chunk代表的是原来被放到第二个位置的头部，因为这个chunk可能要比除去size之后的chunk的又大了
    Chunk* nxtChunk = reinterpret_cast<Chunk*>(
        reinterpret_cast<data_pointer>(head) + HEADER_SIZE + head->size
    );

    head->free = 0;//将头部free的标志为设置为0，代表将要占用
    if (head->size > size + HEADER_SIZE)
    {
        chunk->free = 1;//即将释放这个剩下的chunk
        chunk->pre = head;//这个chunk的前面的被分配出去的地址为head
        chunk->size = head->size - size - HEADER_SIZE;//chunk的大小为总大小减去分配的size大小

		//重新调整head的大小
        head->size = size;
        if (*reinterpret_cast<data_pointer>(nxtChunk) != BLOCK_END)//如果之前的nxtchunk存在的话
            nxtChunk->pre = chunk;

        Chunk* oldHead = head;
        insertChunk(chunk, oldHead);//将这个剩下的chunk放到oldhead的后面（之前是连着的，现在分成了两个）
        removeChunk(oldHead);//将这个从链表中删除，因为即将使用这个
    }
    else
        // No enough space for a new chunk
        removeChunk(head);

    updatePool();
    return ret;//返回刚才被删除的即将作为分配的部分
}

// Release block of storage 释放block
void MemoryPool::deallocate(data_pointer ptr)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(ptr - HEADER_SIZE);
    Chunk* preChunk = chunk->pre;//找到这个释放的chunk之前的chunk
    Chunk* nxtChunk = reinterpret_cast<Chunk*>(ptr + chunk->size);//找到这个chunk之后的chunk

    chunk->free = 1;//把这个chunk的free设置为1，可以进行使用

    // Merge next neighboring chunk
    if (ptr[chunk->size] != BLOCK_END && nxtChunk->free)//如果chunk后面的chunk也为1说明可以和后面的进行合并
    {
        mergeNextChunk(chunk);//合并后面的chunk到这个chunk
        removeChunk(nxtChunk);//删除后面的chunk
    }

    // Merge previous neighboring chunk
    if (preChunk != NULL && preChunk->free)
        mergeNextChunk(preChunk);//合并chunk到prechunk之中
    else
        insertChunk(chunk, head);//将chunk放到head后面，如果不能合并到前面的，就直接当作一个新的chunk加入到链表中

    updatePool();
}

// Allocate block of memory from OS 从操作系统获取内存块
MemoryPool::data_pointer MemoryPool::allocateBlock(size_type size, size_type& realSize)
{
    realSize = size + HEADER_SIZE + sizeof(BLOCK_END);//真实申请的内存大小是数据的大小加上chunk结构体的大小和BLOCK_END
    realSize = (realSize + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;//申请的内存是block_size的整数倍
    data_pointer ret = reinterpret_cast<data_pointer>(operator new(realSize));//调用operator new 分配realsize大小的空间
    ret[realSize - sizeof(BLOCK_END)] = BLOCK_END;//将最后的位置设为BLOCK_END
    return ret;
}

// Insert chunk after pre 将chunk插入到pre的后面
void MemoryPool::insertChunk(Chunk* chunk, Chunk* pre)
{
    if (pre == NULL) //如果为空的话，chunk自己成为一个自己连自己的循环链表
    {
        // Empty linked list 
        chunk->preNode = chunk->nxtNode = chunk;
        head = chunk;//头部就是这个chunk
    }
	/*
		***pre******pre->nextnode***
		***pre******chunk******pre->nextnode****
	*/
    else//将chunk插入到循环链表中
    {
        chunk->preNode = pre;
        chunk->nxtNode = pre->nxtNode;
        pre->nxtNode->preNode = chunk;
        pre->nxtNode = chunk;
    }
}

// Remove chunk from memory pool
void MemoryPool::removeChunk(Chunk* chunk)
{
	/*
	***head(chunk)******head->nextnode***
	***head->nextnode***
    */
	if (head == chunk)//如果chunk处在头节点的位置
        // Change head if head is being removed
        head = head->nxtNode;
	
    if (chunk->preNode == chunk && chunk->nxtNode == chunk)//链表中只有一个元素了
        // Only one element in linked list
        head = NULL;//头节点直接为空
	/*
	***chunk->prenode******chunk******chunk->nextnode***
	***chunk->prenode*****chunk->nextnode
	*/
    else
    {
        chunk->preNode->nxtNode = chunk->nxtNode;
        chunk->nxtNode->preNode = chunk->preNode;
    }
}

// Merge chunk with its next chunk 和下一个chunk进行合并
void MemoryPool::mergeNextChunk(Chunk* chunk)
{
    data_pointer ptr = reinterpret_cast<data_pointer>(chunk);//ptr代表chunk
    Chunk* nxtChunk = reinterpret_cast<Chunk*>(ptr + HEADER_SIZE + chunk->size);//加上chunk的大小就是下一个chunk的指针
    chunk->size += HEADER_SIZE + nxtChunk->size;//把这个chunk的大小设置为加上下一个chunk的大小，下一个chunk合并到这个chunk来

    ptr = reinterpret_cast<data_pointer>(nxtChunk);//ptr代表nextchunk
    if (ptr[HEADER_SIZE + nxtChunk->size] != BLOCK_END)
    {
        nxtChunk = reinterpret_cast<Chunk*>(ptr + HEADER_SIZE + nxtChunk->size);//找到nextchunk的下一个chunk
        nxtChunk->pre = chunk;//将下一个chunk的前端设置为这个合并的chunk，不管下面的chunk的free标志是否为1
    }
}

// Update memory pool 更新内存池
void MemoryPool::updatePool()
{
    if (head == NULL)
        return;

    if (head->size < head->nxtNode->size)//如果头节点比第二个的大小小，则将后面的第二个作为新的头节点
        head = head->nxtNode;
    else if (head->size > head->nxtNode->size && head->nxtNode->nxtNode != head)//如果第二个节点的代销小于head的话
    {
        Chunk* chunk = head->nxtNode;
        removeChunk(chunk);//删除这个第二个节点
        insertChunk(chunk, head->preNode);//将这个节点放到head的前面
    }
}
