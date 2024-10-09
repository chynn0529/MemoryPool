#include <iostream>
//#include <vector>
//#include <memory>

#include "myMemoryPool.h"

// 假设内存池类已经定义，如上文所示

class MyObject {
public:
    MyObject(int data = 0) : data_(data) {
        std::cout << "MyObject constructed with data = " << data_ << std::endl;
    }

    ~MyObject() {
        std::cout << "MyObject destructed with data = " << data_ << std::endl;
    }

    void setData(int data) { data_ = data; }
    int getData() const { return data_; }

private:
    int data_;
};

int main() {
    // 创建一个内存池，预分配1024个MyObject对象
    MemoryPool<MyObject> pool(1024);

    // 分配一个MyObject对象
    MyObject* obj1 = pool.allocate();
    new (obj1) MyObject(42); // 使用位置new构造对象

    std::cout << "obj1 data: " << obj1->getData() << std::endl;

    // 使用完毕后，调用析构函数并释放内存
    obj1->~MyObject();
    pool.deallocate(obj1);

    // 分配多个对象
    std::vector<MyObject*> objects;
    for (int i = 0; i < 10; ++i) {
        MyObject* obj = pool.allocate();
        new (obj) MyObject(i);
        objects.push_back(obj);
    }

    // 输出对象数据
    for (auto obj : objects) {
        std::cout << "MyObject data: " << obj->getData() << std::endl;
    }

    // 释放对象
    for (auto obj : objects) {
        obj->~MyObject();
        pool.deallocate(obj);
    }

    return 0;
}
