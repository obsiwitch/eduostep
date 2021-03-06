#include <iostream>
#include <mutex>
#include "shared.hpp"

struct Node {
    int key;
    Node *next;
    Node(int key, Node *next): key(key), next(next) {}
};

class List {
public:
    List(): head_(NULL), mutex_(), size_(0) {}

    ~List() {
        mutex_.lock();
        Node *current = head_;
        while (current) {
            Node *tmp = current->next;
            delete current;
            current = tmp;
        }
        mutex_.unlock();
    }

    void insert(int key) {
        auto *node = new Node(key, NULL);
        mutex_.lock();
        node->next = head_;
        head_ = node;
        size_++;
        mutex_.unlock();
    }

    Node* search(int key) {
        mutex_.lock();
        auto *result = head_;
        while (result != NULL && key != result->key) {
            result = result->next;
        }
        mutex_.unlock();
        return result;
    }

    int size() {
        mutex_.lock();
        int result = size_;
        mutex_.unlock();
        return result;
    }
private:
    Node *head_;
    std::mutex mutex_;
    int size_;
};

std::ostream& operator<<(std::ostream &stream, List &list) {
    return stream << "size=" << list.size();
}

int main(int argc, char *argv[]) {
    return main_collection<List>(argc, argv);
}
