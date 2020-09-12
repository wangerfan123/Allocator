#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

#include <cstddef>

class MemoryPool
{
public:

    // Member types
	//typedef的作用是为类型重新起个名字
    typedef void* pointer;
    typedef size_t size_type;//size_type代表size_t
    typedef ptrdiff_t difference_type;//通常用来代表两个指针的差，一种机器类型
    typedef unsigned char data_type; //无符号字符型
    typedef unsigned char* data_pointer;//无符号字符的指针

    // Get the single memory pool instance 单例模式得到单例
    static MemoryPool& getInstance();
    
    // Delete these method to avoid copies
    MemoryPool(const MemoryPool&) = delete;//禁用拷贝构造函数
    void operator=(const MemoryPool&) = delete;//禁用赋值
    
    // Allocate block of storage
    pointer allocate(size_type size);//自定义分配函数

    // Release block of storage
    void deallocate(data_pointer ptr);//自定义释放函数

private:

    // Constructor
    MemoryPool();//默认构造函数
    
    // Destructor
    ~MemoryPool();//默认析构函数
    
    // Memory pool chunk chunk 结构体
    struct Chunk
    {
        data_type free;//unsigned char 用来标志这个chunk是否被使用，刚申请的chunk的free标志都为1，代表未使用
        Chunk* pre;//用来标记这个chunk是哪个被占用内存分配后剩下的，根据这个可以找到原来是一起的内存块并在前面释放的时候进行合并
        size_type size;//chunk的大小

        // Chunk on linked list
        Chunk* preNode;//在循环链表上的前面的chunk指针
        Chunk* nxtNode;//在循环链表上的后面的chunk指针
    };
    
    // Chunk header size
    static const size_type HEADER_SIZE = sizeof(Chunk);//一个结构体的大小

    // Block size
    static const size_type BLOCK_SIZE = 8192;//block大小，一般为了加快申请速度设置为4096或者8192

    // Block end symbol
    static const data_type BLOCK_END = 0xFF;//块结束的地方
    
    // Memory pool head 
    Chunk* head; //内存池头部，始终视其为一个最大的chunk

    // Allocate block of memory from OS 调用operator new分配整个block
    data_pointer allocateBlock(size_type size, size_type& realSize);

    // Insert chunk after pre
    void insertChunk(Chunk* chunk, Chunk* pre);//插入块

    // Remove chunk from memory pool
    void removeChunk(Chunk* chunk);//移除块

    // Merge chunk with its next chunk
    void mergeNextChunk(Chunk* chunk);//合并下一个块

    // Update memory pool
    void updatePool();//升级内存池
};

#endif