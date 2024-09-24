#ifndef LOSER_TREE_H
#define LOSER_TREE_H
#include<iostream>
#include<vector>
using namespace std;


template<typename T>
class LoserTree {
    vector<int> tree;  // Loser Tree ����,[0]���ڴ洢ʤ��
    vector<T> leaves;  // Ҷ�ӽ�㣬�洢Ҫ���������
    int k;  // Ҷ�ӽ�������

public:
    // ���캯��
    LoserTree(int k, vector<T>& input) : k(k), leaves(input) {
        tree.resize(k, -1);  // ��ʼ�� loser tree ���ڲ��ڵ�Ϊ -1
        build();
    }

    // ���� loser tree
    void build() {
        // ���Ҷ�ӽڵ���� loser tree
        for (int i = 0; i < k; ++i) {
            update(i);
        }
    }

    // ���� loser tree��ָ���ĸ�Ҷ�ӽ�㷢���˱仯
    void update(int leaf) {
        int parent = (leaf + k) / 2;  // ���㸸�ڵ�λ��
        int winner = leaf;  // ��ǰӮ��Ϊ����ӵ�Ҷ�ӽڵ�

        while (parent > 0) {
            // �Ƚϵ�ǰӮ�Һ͸��ڵ��Ҷ��
            if (leaves[winner] > leaves[tree[parent]]) {
                swap(winner, tree[parent]);  // Ӯ���������棬���ߴ��븸�ڵ�
            }
            parent /= 2;  // �ƶ�����һ�����ڵ�
        }
        tree[0] = winner;  // ���յ�Ӯ�ұ����ڸ��ڵ�
    }

    // ��ȡӮ��Ԫ�أ�����С��Ԫ�أ�
    T getWinner() {
        return leaves[tree[0]];
    }

    // ��һ����ֵ�滻Ӯ��
    void replaceWinner(T newValue) {
        leaves[tree[0]] = newValue;  // ��Ҷ�ӽ��ֵ�滻Ϊ��ֵ
        update(tree[0]);  // ���� loser tree
    }
};
#endif // !LOSER_TREE_H
