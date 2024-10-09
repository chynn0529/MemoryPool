#ifndef MYMEMORYPOOL_H
#define MYMEMORYPOOL_H

#include <cstddef>
#include <vector>
#include <mutex>
#include <memory>
#include <iostream>
#include <cassert>

// MemoryPool 类定义
template <typename T>
class MemoryPool {
public:
    // 构造函数，初始化内存池，默认每块内存包含1024个对象
    explicit MemoryPool(size_t blockSize = 1024);

    // 析构函数，释放所有分配的内存块
    ~MemoryPool();

    // 禁止拷贝构造函数和拷贝赋值运算符
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // 分配内存，返回指向T类型的指针
    T* allocate();

    // 释放内存，将指针返回到内存池
    void deallocate(T* ptr);

private:
    // 内存块结构，用于链表管理空闲内存块
    struct FreeNode {
        FreeNode* next;
    };

    // 分配一块新的内存区域，并将其划分为多个FreeNode，加入到空闲链表中
    void allocateBlock();

    size_t m_blockSize;                 // 每块内存中包含的对象数量
    std::vector<void*> m_blocks;        // 所有分配的内存块
    FreeNode* m_freeList;               // 空闲内存块链表的头指针

    std::mutex m_mutex;                 // 线程安全锁
};

// MemoryPool 构造函数
template <typename T>
MemoryPool<T>::MemoryPool(size_t blockSize)
    : m_blockSize(blockSize), m_freeList(nullptr) {
    allocateBlock(); // 初始分配一个内存块
}

// MemoryPool 析构函数
template <typename T>
MemoryPool<T>::~MemoryPool() {
    for (auto block : m_blocks) {
        ::operator delete(block); // 释放每个内存块
    }
}

// 分配一块新的内存区域
template <typename T>
void MemoryPool<T>::allocateBlock() {
    // 计算每个内存块的大小，确保对齐
    size_t size = sizeof(FreeNode) > sizeof(T) ? sizeof(FreeNode) : sizeof(T);

    // 分配一大块内存
    char* block = static_cast<char*>(::operator new(size * m_blockSize));

    m_blocks.push_back(block); // 记录分配的内存块

    // 将新分配的内存块划分为多个FreeNode，并加入空闲链表
    for (size_t i = 0; i < m_blockSize; ++i) {
        FreeNode* node = reinterpret_cast<FreeNode*>(block + i * size);
        node->next = m_freeList;
        m_freeList = node;
    }
}

// 分配内存
template <typename T>
T* MemoryPool<T>::allocate() {
    std::lock_guard<std::mutex> lock(m_mutex); // 线程安全

    if (!m_freeList) {
        allocateBlock(); // 如果空闲链表为空，分配新的内存块
    }

    // 从空闲链表中取出一个节点
    FreeNode* node = m_freeList;
    m_freeList = node->next;

    return reinterpret_cast<T*>(node);
}

// 释放内存
template <typename T>
void MemoryPool<T>::deallocate(T* ptr) {
    std::lock_guard<std::mutex> lock(m_mutex); // 线程安全

    // 将释放的内存块重新加入到空闲链表
    FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
    node->next = m_freeList;
    m_freeList = node;
}

//// 示例类，用于测试 MemoryPool
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


//// 使用 MemoryPool 的示例
//int main() {
//    // 创建一个 MemoryPool，预分配1024个 MyObject 对象
//    MemoryPool<MyObject> pool(1024);

//    // 分配一个 MyObject 对象
//    MyObject* obj1 = pool.allocate();
//    new (obj1) MyObject(42); // 使用位置new构造对象

//    std::cout << "obj1 data: " << obj1->getData() << std::endl;

//    // 使用完毕后，调用析构函数并释放内存
//    obj1->~MyObject();
//    pool.deallocate(obj1);

//    // 分配多个对象
//    std::vector<MyObject*> objects;
//    for (int i = 0; i < 10; ++i) {
//        MyObject* obj = pool.allocate();
//        new (obj) MyObject(i);
//        objects.push_back(obj);
//    }

//    // 输出对象数据
//    for (auto obj : objects) {
//        std::cout << "MyObject data: " << obj->getData() << std::endl;
//    }

//    // 释放对象
//    for (auto obj : objects) {
//        obj->~MyObject();
//        pool.deallocate(obj);
//    }

//    return 0;
//}


#endif // MYMEMORYPOOL_H
