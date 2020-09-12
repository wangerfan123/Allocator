#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

#include <cstddef>

class MemoryPool
{
public:

    // Member types
	//typedef��������Ϊ���������������
    typedef void* pointer;
    typedef size_t size_type;//size_type����size_t
    typedef ptrdiff_t difference_type;//ͨ��������������ָ��Ĳһ�ֻ�������
    typedef unsigned char data_type; //�޷����ַ���
    typedef unsigned char* data_pointer;//�޷����ַ���ָ��

    // Get the single memory pool instance ����ģʽ�õ�����
    static MemoryPool& getInstance();
    
    // Delete these method to avoid copies
    MemoryPool(const MemoryPool&) = delete;//���ÿ������캯��
    void operator=(const MemoryPool&) = delete;//���ø�ֵ
    
    // Allocate block of storage
    pointer allocate(size_type size);//�Զ�����亯��

    // Release block of storage
    void deallocate(data_pointer ptr);//�Զ����ͷź���

private:

    // Constructor
    MemoryPool();//Ĭ�Ϲ��캯��
    
    // Destructor
    ~MemoryPool();//Ĭ����������
    
    // Memory pool chunk chunk �ṹ��
    struct Chunk
    {
        data_type free;//unsigned char ������־���chunk�Ƿ�ʹ�ã��������chunk��free��־��Ϊ1������δʹ��
        Chunk* pre;//����������chunk���ĸ���ռ���ڴ�����ʣ�µģ�������������ҵ�ԭ����һ����ڴ�鲢��ǰ���ͷŵ�ʱ����кϲ�
        size_type size;//chunk�Ĵ�С

        // Chunk on linked list
        Chunk* preNode;//��ѭ�������ϵ�ǰ���chunkָ��
        Chunk* nxtNode;//��ѭ�������ϵĺ����chunkָ��
    };
    
    // Chunk header size
    static const size_type HEADER_SIZE = sizeof(Chunk);//һ���ṹ��Ĵ�С

    // Block size
    static const size_type BLOCK_SIZE = 8192;//block��С��һ��Ϊ�˼ӿ������ٶ�����Ϊ4096����8192

    // Block end symbol
    static const data_type BLOCK_END = 0xFF;//������ĵط�
    
    // Memory pool head 
    Chunk* head; //�ڴ��ͷ����ʼ������Ϊһ������chunk

    // Allocate block of memory from OS ����operator new��������block
    data_pointer allocateBlock(size_type size, size_type& realSize);

    // Insert chunk after pre
    void insertChunk(Chunk* chunk, Chunk* pre);//�����

    // Remove chunk from memory pool
    void removeChunk(Chunk* chunk);//�Ƴ���

    // Merge chunk with its next chunk
    void mergeNextChunk(Chunk* chunk);//�ϲ���һ����

    // Update memory pool
    void updatePool();//�����ڴ��
};

#endif