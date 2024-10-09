#ifndef MYMEMORYPOOL_H
#define MYMEMORYPOOL_H

#include <cstddef>
#include <vector>
#include <mutex>
#include <memory>
#include <iostream>
#include <cassert>

// MemoryPool �ඨ��
template <typename T>
class MemoryPool {
public:
    // ���캯������ʼ���ڴ�أ�Ĭ��ÿ���ڴ����1024������
    explicit MemoryPool(size_t blockSize = 1024);

    // �����������ͷ����з�����ڴ��
    ~MemoryPool();

    // ��ֹ�������캯���Ϳ�����ֵ�����
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // �����ڴ棬����ָ��T���͵�ָ��
    T* allocate();

    // �ͷ��ڴ棬��ָ�뷵�ص��ڴ��
    void deallocate(T* ptr);

private:
    // �ڴ��ṹ�����������������ڴ��
    struct FreeNode {
        FreeNode* next;
    };

    // ����һ���µ��ڴ����򣬲����仮��Ϊ���FreeNode�����뵽����������
    void allocateBlock();

    size_t m_blockSize;                 // ÿ���ڴ��а����Ķ�������
    std::vector<void*> m_blocks;        // ���з�����ڴ��
    FreeNode* m_freeList;               // �����ڴ�������ͷָ��

    std::mutex m_mutex;                 // �̰߳�ȫ��
};

// MemoryPool ���캯��
template <typename T>
MemoryPool<T>::MemoryPool(size_t blockSize)
    : m_blockSize(blockSize), m_freeList(nullptr) {
    allocateBlock(); // ��ʼ����һ���ڴ��
}

// MemoryPool ��������
template <typename T>
MemoryPool<T>::~MemoryPool() {
    for (auto block : m_blocks) {
        ::operator delete(block); // �ͷ�ÿ���ڴ��
    }
}

// ����һ���µ��ڴ�����
template <typename T>
void MemoryPool<T>::allocateBlock() {
    // ����ÿ���ڴ��Ĵ�С��ȷ������
    size_t size = sizeof(FreeNode) > sizeof(T) ? sizeof(FreeNode) : sizeof(T);

    // ����һ����ڴ�
    char* block = static_cast<char*>(::operator new(size * m_blockSize));

    m_blocks.push_back(block); // ��¼������ڴ��

    // ���·�����ڴ�黮��Ϊ���FreeNode���������������
    for (size_t i = 0; i < m_blockSize; ++i) {
        FreeNode* node = reinterpret_cast<FreeNode*>(block + i * size);
        node->next = m_freeList;
        m_freeList = node;
    }
}

// �����ڴ�
template <typename T>
T* MemoryPool<T>::allocate() {
    std::lock_guard<std::mutex> lock(m_mutex); // �̰߳�ȫ

    if (!m_freeList) {
        allocateBlock(); // �����������Ϊ�գ������µ��ڴ��
    }

    // �ӿ���������ȡ��һ���ڵ�
    FreeNode* node = m_freeList;
    m_freeList = node->next;

    return reinterpret_cast<T*>(node);
}

// �ͷ��ڴ�
template <typename T>
void MemoryPool<T>::deallocate(T* ptr) {
    std::lock_guard<std::mutex> lock(m_mutex); // �̰߳�ȫ

    // ���ͷŵ��ڴ�����¼��뵽��������
    FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
    node->next = m_freeList;
    m_freeList = node;
}

//// ʾ���࣬���ڲ��� MemoryPool
//class MyObject {
//public:
//    MyObject(int data = 0) : data_(data) {
//        std::cout << "MyObject constructed with data = " << data_ << std::endl;
//    }

//    ~MyObject() {
//        std::cout << "MyObject destructed with data = " << data_ << std::endl;
//    }

//    void setData(int data) { data_ = data; }
//    int getData() const { return data_; }

//private:
//    int data_;
//};


//// ʹ�� MemoryPool ��ʾ��
//int main() {
//    // ����һ�� MemoryPool��Ԥ����1024�� MyObject ����
//    MemoryPool<MyObject> pool(1024);

//    // ����һ�� MyObject ����
//    MyObject* obj1 = pool.allocate();
//    new (obj1) MyObject(42); // ʹ��λ��new�������

//    std::cout << "obj1 data: " << obj1->getData() << std::endl;

//    // ʹ����Ϻ󣬵��������������ͷ��ڴ�
//    obj1->~MyObject();
//    pool.deallocate(obj1);

//    // ����������
//    std::vector<MyObject*> objects;
//    for (int i = 0; i < 10; ++i) {
//        MyObject* obj = pool.allocate();
//        new (obj) MyObject(i);
//        objects.push_back(obj);
//    }

//    // �����������
//    for (auto obj : objects) {
//        std::cout << "MyObject data: " << obj->getData() << std::endl;
//    }

//    // �ͷŶ���
//    for (auto obj : objects) {
//        obj->~MyObject();
//        pool.deallocate(obj);
//    }

//    return 0;
//}


#endif // MYMEMORYPOOL_H
