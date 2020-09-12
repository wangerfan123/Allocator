#ifndef _MY_ALLOCATOR_H
#define _MY_ALLOCATOR_H

#include <cstddef>
#include "memoryPool.h"

#define MY_ALLOC MyAllocator<T>


//�ڴ����������һ��ģ���࣬ģ���������������ʵ�֣������ڹ̶���С���ڴ����� 
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
    
    // Default constructor ����
    MyAllocator() noexcept;

    // A converting copy constructor ģ�忽�����캯��
    template <class U>
    MyAllocator(const MyAllocator<U>&) noexcept;

    // == and != operators �ȽϷ��ź���
    template<class U>
    bool operator==(const MyAllocator<U>&) const noexcept;

    template<class U> 
    bool operator!=(const MyAllocator<U>&) const noexcept;

    // Allocate block of storage ������
    pointer allocate(size_type n, const_pointer hint = 0);

    // Release block of storage �ͷ���
    void deallocate(pointer p, size_type n);
};

// Default constructor
template <class T>
MY_ALLOC::MyAllocator() noexcept {}//����,noexcept���ѱ������������һ�����ᷢ������ʡȥ�˱������������ʱ��ȴ���

// A converting copy constructor
template <class T>
template <class U>
MY_ALLOC::MyAllocator(const MyAllocator<U>&) noexcept: MyAllocator() {}//��������

// == and != operators
template <class T>
template<class U>
bool MY_ALLOC::operator==(const MyAllocator<U>&) const noexcept//�Ƚ������
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
typename MY_ALLOC::pointer MY_ALLOC::allocate(size_type n, const_pointer hint)//��Ҫ�����ڴ�ʱ�������ڴ�صķ��亯������������Ԫ�ظ�����
{
    return reinterpret_cast<pointer>(MemoryPool::getInstance().allocate(sizeof(T) * n));//Ԫ�ش�С��Ԫ�صĸ���
}

// Release block of storage
template <class T>
void MY_ALLOC::deallocate(pointer p, size_type n)//��Ҫ�ͷ�ʱ�������ڴ�ص��ͷź���
{
    MemoryPool::getInstance().deallocate(reinterpret_cast<data_pointer>(p));//������Ҫ�ͷŵ�ָ��
}

#endif
