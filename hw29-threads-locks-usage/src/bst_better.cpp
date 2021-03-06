#include <iostream>
#include <shared_mutex>
#include <vector>
#include "shared.hpp"

struct Node {
    int key;
    int height;
    Node *parent;
    Node *left;
    Node *right;
    Node(int key, Node *parent, Node *left, Node *right)
    : key(key), parent(parent), left(left), right(right) {
        height = parent == NULL ? 0 : parent->height + 1;
    }
};

class BinarySearchTree {
public:
    BinarySearchTree(): root_(NULL), rw_mutex_(), size_(0), height_(0) {}

    ~BinarySearchTree() {
        rw_mutex_.lock();
        if (root_ == NULL) { return; }
        std::vector<Node*> to_process = { root_ };
        while (!to_process.empty()) {
            auto *e = to_process.back();
            to_process.pop_back();
            if (e->left != NULL) { to_process.push_back(e->left); }
            if (e->right != NULL) { to_process.push_back(e->right); }
            delete e;
        }
        rw_mutex_.unlock();
    }

    Node* search(int key) {
        rw_mutex_.lock_shared();
        auto *result = root_;
        while (result != NULL && key != result->key) {
            if (key < result->key) { result = result->left; }
            else { result = result->right; }
        }
        rw_mutex_.unlock_shared();
        return result;
    }

    void insert(int key) {
        rw_mutex_.lock();
        Node *parent = NULL;
        auto *current = root_;
        while (current != NULL) {
            parent = current;
            if (key < current->key) { current = current->left; }
            else { current = current->right; }
        }

        auto *node = new Node(key, parent, NULL, NULL);
        if (parent == NULL) {
            root_ = node;
        } else if (node->key < parent->key) {
            parent->left = node;
        } else {
            parent->right = node;
        }
        size_++;
        height_ = std::max(height_, node->height);
        rw_mutex_.unlock();
    }

    int size() {
        rw_mutex_.lock_shared();
        int result = size_;
        rw_mutex_.unlock_shared();
        return result;
    }

    int height() {
        rw_mutex_.lock_shared();
        int result = height_;
        rw_mutex_.unlock_shared();
        return result;
    }
private:
    Node *root_;
    std::shared_mutex rw_mutex_;
    int size_;
    int height_;
};

std::ostream& operator<<(std::ostream &stream, BinarySearchTree &tree) {
    return stream << "size=" << tree.size() << " height=" << tree.height();
}

int main(int argc, char *argv[]) {
    return main_collection<BinarySearchTree>(argc, argv);
}
