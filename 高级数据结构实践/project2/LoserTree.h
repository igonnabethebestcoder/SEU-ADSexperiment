#ifndef LOSER_TREE_H
#define LOSER_TREE_H
#include<iostream>
#include<vector>
using namespace std;


template<typename T>
class LoserTree {
    vector<int> tree;  // Loser Tree 数组,[0]用于存储胜者
    vector<T> leaves;  // 叶子结点，存储要排序的数据
    int k;  // 叶子结点的数量

public:
    // 构造函数
    LoserTree(int k, vector<T>& input) : k(k), leaves(input) {
        tree.resize(k, -1);  // 初始化 loser tree 的内部节点为 -1
        build();
    }

    // 构建 loser tree
    void build() {
        // 逐个叶子节点更新 loser tree
        for (int i = 0; i < k; ++i) {
            update(i);
        }
    }

    // 更新 loser tree，指定哪个叶子结点发生了变化
    void update(int leaf) {
        int parent = (leaf + k) / 2;  // 计算父节点位置
        int winner = leaf;  // 当前赢家为新添加的叶子节点

        while (parent > 0) {
            // 比较当前赢家和父节点的叶子
            if (leaves[winner] > leaves[tree[parent]]) {
                swap(winner, tree[parent]);  // 赢家留在外面，败者存入父节点
            }
            parent /= 2;  // 移动到下一个父节点
        }
        tree[0] = winner;  // 最终的赢家保存在根节点
    }

    // 获取赢家元素（即最小的元素）
    T getWinner() {
        return leaves[tree[0]];
    }

    // 用一个新值替换赢家
    void replaceWinner(T newValue) {
        leaves[tree[0]] = newValue;  // 将叶子结点值替换为新值
        update(tree[0]);  // 更新 loser tree
    }
};
#endif // !LOSER_TREE_H
