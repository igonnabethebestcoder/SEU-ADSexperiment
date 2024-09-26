#ifndef LOSER_TREE_H
#define LOSER_TREE_H
#include<iostream>
#include<vector>
using namespace std;


template<typename T>
class LoserTree {
public:
    vector<int> tree;           // Loser Tree ����, [0]���ڴ洢ʤ��, �洢���ǽڵ�����
    vector<T> leaves;           // Ҷ�ӽ�㣬�洢Ҫ���������
    vector<bool> competitor;    // Ҷ���Ƿ��ܲμӱ�������
    int banCount;
    int k;                      // Ҷ�ӽ�������

public:
    // ���캯��
    LoserTree(int k, vector<T>& input) : k(k), leaves(input), banCount(0) {
        tree.resize(k, -1);          // ��ʼ�� loser tree ���ڲ��ڵ�Ϊ -1
        competitor.resize(k, true);  // ��ʼ������Ҷ�ӽڵ�Ϊ�ɲμӱ���
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
        int winner = leaf;             // ��ǰӮ��Ϊ����ӵ�Ҷ�ӽڵ�

        while (parent > 0) {
            // �����ǰҶ�ӽڵ㲻�ܲμӱ�����������
            if (!competitor[winner]) {
                winner = tree[parent]; // ѡ�񸸽ڵ��Ӯ��
            }
            else if (tree[parent] != -1 && !competitor[tree[parent]]) {
                // ������ڵ��Ӯ�ұ���������ѡ��ǰӮ��
                // �Ƚϵ�ǰӮ�Һ͸��ڵ��Ҷ��
                winner = leaf; // ��ǰҶ�ӽڵ���ΪӮ��
            }
            else if (leaves[winner] > leaves[tree[parent]]) {
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

    // ���һ����ֵ�������¼���Ӯ��
    void replaceWinner(T newValue) {
        // ��鵱ǰӮ���Ƿ��ܲμӱ���
        if (competitor[tree[0]]) {
            leaves[tree[0]] = newValue;  // ��Ҷ�ӽ��ֵ�滻Ϊ��ֵ
            update(tree[0]);  // ���� loser tree
        }
        else {
            // ��ǰӮ�ҽ�����Ѱ�������ɲ�����Ӯ��
            int newWinner = -1;
            for (int i = 0; i < k; ++i) {
                if (competitor[i]) {
                    if (newWinner == -1 || leaves[i] < leaves[newWinner]) {
                        newWinner = i;
                    }
                }
            }
            if (newWinner != -1) {
                leaves[newWinner] = newValue; // �����µ�Ӯ��
                update(newWinner);  // ���� loser tree
            }
        }
    }

    //�����нڵ㶼��������˵���Ѿ���ȫ����һ��runfile,��ʱ����Ҫ���¿�ʼ����
    void reCompete()
    {
        fill(competitor.begin(), competitor.end(), true);
        banCount = 0;
        build();
    }

    // ����ָ����Ҷ�ӽڵ�
    void disqualify(int index) {
        if (index >= 0 && index < k) {
            competitor[index] = false; // ��ָ��Ҷ�ӽڵ���Ϊ����
            banCount++;
            // ��������ȷ����ǰӮ����Ȼ����Ч��
            update(tree[0]);
        }
    }
};
#endif // !LOSER_TREE_H
