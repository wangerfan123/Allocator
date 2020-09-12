#ifndef _MY_ALLOCATOR_H
#define _MY_ALLOCATOR_H

#include <cstddef>
#include "memoryPool.h"

#define MY_ALLOC MyAllocator<T>


//内存分配器就是一个模板类，模板参数就是链表来实现，适用于固定大小的内存块分配 
template <class T>
class MyAllocator
{
public:
    
    // Member types
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef unsigned char data_type;
    typedef unsigned char* data_pointer;
    
    // Default constructor 构造
    MyAllocator() noexcept;

    // A converting copy constructor 模板拷贝构造函数
    template <class U>
    MyAllocator(const MyAllocator<U>&) noexcept;

    // == and != operators 比较符号函数
    template<class U>
    bool operator==(const MyAllocator<U>&) const noexcept;

    template<class U> 
    bool operator!=(const MyAllocator<U>&) const noexcept;

    // Allocate block of storage 分配器
    pointer allocate(size_type n, const_pointer hint = 0);

    // Release block of storage 释放器
    void deallocate(pointer p, size_type n);
};

// Default constructor
template <class T>
MY_ALLOC::MyAllocator() noexcept {}//构造,noexcept提醒编译器这个函数一定不会发生错误，省去了编译器检查错误的时间等代价

// A converting copy constructor
template <class T>
template <class U>
MY_ALLOC::MyAllocator(const MyAllocator<U>&) noexcept: MyAllocator() {}//拷贝构造

// == and != operators
template <class T>
template<class U>
bool MY_ALLOC::operator==(const MyAllocator<U>&) const noexcept//比较运算符
{
    return true;
}

template <class T>
template<class U>
bool MY_ALLOC::operator!=(const MyAllocator<U>&) const noexcept
{
    return false;
}

// Allocate block of storage
template <class T>
typename MY_ALLOC::pointer MY_ALLOC::allocate(size_type n, const_pointer hint)//需要分配内存时，调用内存池的分配函数。代表分配的元素个数。
{
    return reinterpret_cast<pointer>(MemoryPool::getInstance().allocate(sizeof(T) * n));//元素大小乘元素的个数
}

// Release block of storage
template <class T>
void MY_ALLOC::deallocate(pointer p, size_type n)//需要释放时，调用内存池的释放函数
{
    MemoryPool::getInstance().deallocate(reinterpret_cast<data_pointer>(p));//传入需要释放的指针
}

#endif
