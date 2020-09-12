#include <vector>
#include "memoryPool.h"

// Get the single memory pool instance
// �ڴ�ص���ģʽ�����ؾ�̬�ĵ�������
MemoryPool& MemoryPool::getInstance()
{
    static MemoryPool instance;
    return instance;
}

// Constructor ���캯��
MemoryPool::MemoryPool()
{
    head = NULL;//ͷ��Ϊ��
}

// Destructor �������������ڴ�������еĿ��õ�chunkȫ�����������ͷ�
MemoryPool::~MemoryPool()
{
    if (head == NULL)//Ϊ����������
        return;
    
    std::vector<void*> toDelete;
    Chunk* chunk = head;
    
	//���ν�����chunk��ָ��ŵ�vector������
    do
    {
        if (chunk->pre == NULL)
            toDelete.push_back(reinterpret_cast<void*>(chunk));
        chunk = chunk->nxtNode;
    }
    while(chunk != head);
    
	//����operator delete ͳһ��vector�е�ָ�����ɾ��
    for (void* ptr : toDelete)
        operator delete(ptr);
}

// Allocate block of storage �����ڴ�
MemoryPool::pointer MemoryPool::allocate(size_type size)//size����Ҫ������ڴ�ռ�Ĵ�С
{
    if (head == NULL || head->size < size)//�������ͷ���Ĵ�С�������������ߵ�Ҫ���������ϵͳ����һ���µ��ڴ�
    {
        // Allocate new block if needed
        size_type realSize;//��ʵ�Ĵ�С
        data_pointer ptr = allocateBlock(size, realSize); //��OS�������µ�һ���ڴ棬Ϊ�˼ӿ������ٶȣ���block�Ĵ�С������Ϊһ������ BLOCK_SIZE ��������
        Chunk* chunk = reinterpret_cast<Chunk*>(ptr);//�����ָ��ת����Ϊchunk����

        chunk->free = 1;//free��־λ1������δ��ʹ��
        chunk->pre = NULL;
        chunk->size = realSize - HEADER_SIZE - sizeof(BLOCK_END);//��ʵ�Ĵ�СΪ��ȥͷ����BLOCK_END

        insertChunk(chunk, head);//������µĿ��õ�chunk��ӵ�chunk�д���
        head = chunk;//ͷ��ָ�����Ϊchunk����Ϊ���������Ŀ϶�������
    }
	//��head�Ĵ�С�㹻�������
    pointer ret = reinterpret_cast<pointer>(
        reinterpret_cast<data_pointer>(head) + HEADER_SIZE
    );

    // Split head chunk into two chunks  ��ͷ����Ϊ�����֣���Ϊͷ���Ĵ�С���ܴ���size��ǰ��size���������䣬������ķ��뵽������
    //���chunk�������head��ȥ��size֮��ʣ�µ�chunk
	Chunk* chunk = reinterpret_cast<Chunk*>(
        reinterpret_cast<data_pointer>(head) + HEADER_SIZE + size
    );
    
    // Original next chunk 
	//���chunk�������ԭ�����ŵ��ڶ���λ�õ�ͷ������Ϊ���chunk����Ҫ�ȳ�ȥsize֮���chunk���ִ���
    Chunk* nxtChunk = reinterpret_cast<Chunk*>(
        reinterpret_cast<data_pointer>(head) + HEADER_SIZE + head->size
    );

    head->free = 0;//��ͷ��free�ı�־Ϊ����Ϊ0������Ҫռ��
    if (head->size > size + HEADER_SIZE)
    {
        chunk->free = 1;//�����ͷ����ʣ�µ�chunk
        chunk->pre = head;//���chunk��ǰ��ı������ȥ�ĵ�ַΪhead
        chunk->size = head->size - size - HEADER_SIZE;//chunk�Ĵ�СΪ�ܴ�С��ȥ�����size��С

		//���µ���head�Ĵ�С
        head->size = size;
        if (*reinterpret_cast<data_pointer>(nxtChunk) != BLOCK_END)//���֮ǰ��nxtchunk���ڵĻ�
            nxtChunk->pre = chunk;

        Chunk* oldHead = head;
        insertChunk(chunk, oldHead);//�����ʣ�µ�chunk�ŵ�oldhead�ĺ��棨֮ǰ�����ŵģ����ڷֳ���������
        removeChunk(oldHead);//�������������ɾ������Ϊ����ʹ�����
    }
    else
        // No enough space for a new chunk
        removeChunk(head);

    updatePool();
    return ret;//���ظղű�ɾ���ļ�����Ϊ����Ĳ���
}

// Release block of storage �ͷ�block
void MemoryPool::deallocate(data_pointer ptr)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(ptr - HEADER_SIZE);
    Chunk* preChunk = chunk->pre;//�ҵ�����ͷŵ�chunk֮ǰ��chunk
    Chunk* nxtChunk = reinterpret_cast<Chunk*>(ptr + chunk->size);//�ҵ����chunk֮���chunk

    chunk->free = 1;//�����chunk��free����Ϊ1�����Խ���ʹ��

    // Merge next neighboring chunk
    if (ptr[chunk->size] != BLOCK_END && nxtChunk->free)//���chunk�����chunkҲΪ1˵�����Ժͺ���Ľ��кϲ�
    {
        mergeNextChunk(chunk);//�ϲ������chunk�����chunk
        removeChunk(nxtChunk);//ɾ�������chunk
    }

    // Merge previous neighboring chunk
    if (preChunk != NULL && preChunk->free)
        mergeNextChunk(preChunk);//�ϲ�chunk��prechunk֮��
    else
        insertChunk(chunk, head);//��chunk�ŵ�head���棬������ܺϲ���ǰ��ģ���ֱ�ӵ���һ���µ�chunk���뵽������

    updatePool();
}

// Allocate block of memory from OS �Ӳ���ϵͳ��ȡ�ڴ��
MemoryPool::data_pointer MemoryPool::allocateBlock(size_type size, size_type& realSize)
{
    realSize = size + HEADER_SIZE + sizeof(BLOCK_END);//��ʵ������ڴ��С�����ݵĴ�С����chunk�ṹ��Ĵ�С��BLOCK_END
    realSize = (realSize + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;//������ڴ���block_size��������
    data_pointer ret = reinterpret_cast<data_pointer>(operator new(realSize));//����operator new ����realsize��С�Ŀռ�
    ret[realSize - sizeof(BLOCK_END)] = BLOCK_END;//������λ����ΪBLOCK_END
    return ret;
}

// Insert chunk after pre ��chunk���뵽pre�ĺ���
void MemoryPool::insertChunk(Chunk* chunk, Chunk* pre)
{
    if (pre == NULL) //���Ϊ�յĻ���chunk�Լ���Ϊһ���Լ����Լ���ѭ������
    {
        // Empty linked list 
        chunk->preNode = chunk->nxtNode = chunk;
        head = chunk;//ͷ���������chunk
    }
	/*
		***pre******pre->nextnode***
		***pre******chunk******pre->nextnode****
	*/
    else//��chunk���뵽ѭ��������
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
	if (head == chunk)//���chunk����ͷ�ڵ��λ��
        // Change head if head is being removed
        head = head->nxtNode;
	
    if (chunk->preNode == chunk && chunk->nxtNode == chunk)//������ֻ��һ��Ԫ����
        // Only one element in linked list
        head = NULL;//ͷ�ڵ�ֱ��Ϊ��
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

// Merge chunk with its next chunk ����һ��chunk���кϲ�
void MemoryPool::mergeNextChunk(Chunk* chunk)
{
    data_pointer ptr = reinterpret_cast<data_pointer>(chunk);//ptr����chunk
    Chunk* nxtChunk = reinterpret_cast<Chunk*>(ptr + HEADER_SIZE + chunk->size);//����chunk�Ĵ�С������һ��chunk��ָ��
    chunk->size += HEADER_SIZE + nxtChunk->size;//�����chunk�Ĵ�С����Ϊ������һ��chunk�Ĵ�С����һ��chunk�ϲ������chunk��

    ptr = reinterpret_cast<data_pointer>(nxtChunk);//ptr����nextchunk
    if (ptr[HEADER_SIZE + nxtChunk->size] != BLOCK_END)
    {
        nxtChunk = reinterpret_cast<Chunk*>(ptr + HEADER_SIZE + nxtChunk->size);//�ҵ�nextchunk����һ��chunk
        nxtChunk->pre = chunk;//����һ��chunk��ǰ������Ϊ����ϲ���chunk�����������chunk��free��־�Ƿ�Ϊ1
    }
}

// Update memory pool �����ڴ��
void MemoryPool::updatePool()
{
    if (head == NULL)
        return;

    if (head->size < head->nxtNode->size)//���ͷ�ڵ�ȵڶ����Ĵ�СС���򽫺���ĵڶ�����Ϊ�µ�ͷ�ڵ�
        head = head->nxtNode;
    else if (head->size > head->nxtNode->size && head->nxtNode->nxtNode != head)//����ڶ����ڵ�Ĵ���С��head�Ļ�
    {
        Chunk* chunk = head->nxtNode;
        removeChunk(chunk);//ɾ������ڶ����ڵ�
        insertChunk(chunk, head->preNode);//������ڵ�ŵ�head��ǰ��
    }
}
