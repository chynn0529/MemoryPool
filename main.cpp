#include <iostream>
//#include <vector>
//#include <memory>

#include "myMemoryPool.h"

// �����ڴ�����Ѿ����壬��������ʾ

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
    // ����һ���ڴ�أ�Ԥ����1024��MyObject����
    MemoryPool<MyObject> pool(1024);

    // ����һ��MyObject����
    MyObject* obj1 = pool.allocate();
    new (obj1) MyObject(42); // ʹ��λ��new�������

    std::cout << "obj1 data: " << obj1->getData() << std::endl;

    // ʹ����Ϻ󣬵��������������ͷ��ڴ�
    obj1->~MyObject();
    pool.deallocate(obj1);

    // ����������
    std::vector<MyObject*> objects;
    for (int i = 0; i < 10; ++i) {
        MyObject* obj = pool.allocate();
        new (obj) MyObject(i);
        objects.push_back(obj);
    }

    // �����������
    for (auto obj : objects) {
        std::cout << "MyObject data: " << obj->getData() << std::endl;
    }

    // �ͷŶ���
    for (auto obj : objects) {
        obj->~MyObject();
        pool.deallocate(obj);
    }

    return 0;
}
