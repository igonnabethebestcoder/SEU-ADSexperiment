#ifndef LOSER_TREE_H
#define LOSER_TREE_H
#include<iostream>
#include<vector>
#include<stdexcept> // �����쳣����
#include<algorithm>
using namespace std;


template<typename T>
class LoserTree {
public:
    vector<int> tree;       // Loser Tree ����, [0]���ڴ洢����ʤ��, �洢���ǽڵ�����
    vector<T> leaves;           // Ҷ�ӽ�㣬�洢Ҫ���������
    vector<bool> competitor;    // Ҷ���Ƿ��ܲμӱ�������
    int banCount;               // �������ļ���
    int k;                      // Ҷ�ӽ�������
    int pos;                    // ���Ķѵ���
    bool isSort;                // Ҷ���Ƿ���������pop

public:
    // ���캯��
    LoserTree(int k, vector<T>& input) : k(k), leaves(input), banCount(0) {
        tree.resize(k, -1);          // ��ʼ�� loser tree ���ڲ��ڵ�Ϊ -1
        //wintree.resize(k, -1);
        competitor.resize(k, true);  // ��ʼ������Ҷ�ӽڵ�Ϊ�ɲμӱ���
        pos = 0;
        isSort = false;
        build();
    }

    // ���� loser tree
    void build() {
        vector<int> wintree(k, -1);
        for (int i = 0; i < k; ++i)
            build_update(wintree, i);
        constructLoserTree(wintree);
    }

    void constructLoserTree(vector<T>& wintree)
    {
        for (int i = 0; i < k; ++i)
        {
            if (i == 0)
                tree[0] = wintree[1];
            else
            {
                int leftChild = i * 2;
                int rightChild = i * 2 + 1;
                if (rightChild < k)
                    tree[i] = (wintree[i] == wintree[leftChild]) ? wintree[rightChild] : wintree[leftChild];
                else if (rightChild >= k && leftChild < k)
                    tree[i] = (wintree[i] == wintree[leftChild]) ? rightChild - k : wintree[leftChild];
                else if (leftChild >= k)
                    tree[i] = (leaves[wintree[i]] == leaves[leftChild - k]) ? rightChild - k : leftChild - k;
            }
        }
    }

    void build_update(vector<T>& wintree, int& idx)
    {
        int winner = idx;//��ǰʤ������leaves�����е�ʵ������
        int curTreeIdx = idx + k;//��ǰ�������������Ľڵ�ĳ�������
        int parent = curTreeIdx / 2;//��ǰ�����ڵ�ĸ��ڵ�
        while (parent > 0)
        {
            int bro = (curTreeIdx % 2 == 0) ? parent * 2 + 1 : parent;//��ǰ�����ڵ���ֵܽڵ�
            int realbroIdx = -1;
            if (bro >= k)//��ǰ����Ҷ�ڵ��
                realbroIdx = bro - k;
            else
                realbroIdx = wintree[bro];//������-1

            if (realbroIdx != -1)
            {
                if (leaves[winner] >= leaves[realbroIdx])
                {
                    wintree[parent] = realbroIdx;
                    tree[parent] = winner;
                    winner = realbroIdx;
                }
                else
                {
                    wintree[parent] = winner;
                    tree[parent] = realbroIdx;
                }
            }
            else
            {
                wintree[parent] = winner;
            }
            curTreeIdx = parent;//ԭ�ȵĸ��ڵ�����һ�ֵĲ����ڵ�
            parent /= 2;
        }
        tree[0] = winner;
    }


    //ʹ���ڲ�����ͬ�Ĺ鲢�Σ����滻������
    void replaceWinnerAndBan(T newVal)
    {
        if (banCount >= k)
        {
            cerr << "all competitor has been banned!" << endl;
            return;
        }
        cout << "ban " << leaves[tree[0]];
        leaves[tree[0]] = newVal;
        competitor[tree[0]] = false;
        update(tree[0]);
        cout << " ,after ban the winner is: " << leaves[tree[0]] << endl;
        banCount++;
    }

    // ���� loser tree��ָ���ĸ�Ҷ�ӽ�㷢���˱仯
    void update(int leaf) {
        int parent = (leaf + k) / 2;  // ���㸸�ڵ�λ��
        int winner = leaf;             // ��ǰӮ��Ϊ����ӵ�Ҷ�ӽڵ�

        //�½��������һ������С��
        //���Ը��µ�ʱ���ڵ�İ��߾ͱ����
        while (parent > 0) {
            if (!competitor[winner])
                winner = tree[parent];
            else if (tree[parent] == -1)
                tree[parent] = winner;
            else if (leaves[winner] > leaves[tree[parent]])
            {
                if (competitor[tree[parent]])
                    swap(winner, tree[parent]);
                else
                    tree[parent] = winner;
            }
            parent /= 2; // �ƶ�����һ�����ڵ�
        }
        tree[0] = winner; // ���յ�Ӯ�ұ����ڸ��ڵ�
    }

    // ��ȡӮ��Ԫ�أ�����С��Ԫ�أ�
    T getWinner() {
        return leaves[tree[0]];
    }

    // ���һ����ֵ�������¼���Ӯ��
    void replaceWinner(T newValue) {
        if (competitor[tree[0]]) {
            leaves[tree[0]] = newValue;  // ��Ҷ�ӽ��ֵ�滻Ϊ��ֵ
            update(tree[0]);  // ���� loser tree
        }
        else {
            // ��ǰӮ�ҽ�����Ѱ�������ɲ�����Ӯ��
            int newWinner = -1;
            for (int i = 0; i < k; ++i) {
                if (competitor[i] && (newWinner == -1 || leaves[i] < leaves[newWinner])) {
                    newWinner = i;
                }
            }
            if (newWinner != -1) {
                leaves[newWinner] = newValue; // �����µ�Ӯ��
                update(newWinner);  // ���� loser tree
            }
        }
    }

    // �����нڵ㶼��������˵���Ѿ���ȫ����һ�� runfile����ʱ����Ҫ���¿�ʼ����
    void reCompete() {
        fill(competitor.begin(), competitor.end(), true);
        fill(tree.begin(), tree.end(), -1);
        banCount = 0;
        build();
    }

    // ����ָ����Ҷ�ӽڵ�
    int disqualify(int index) {
        int oldWinner = tree[0];
        if (index >= 0 && index < k) {
            competitor[index] = false; // ��ָ��Ҷ�ӽڵ���Ϊ����
            banCount++;
            // ��������ȷ����ǰӮ����Ȼ����Ч��
            update(tree[0]);
        }
        return oldWinner;
    }

    T pop()
    {
        if (!isSort)
        {
            sort(leaves.begin(), leaves.end());
            isSort = true;
        }

        if (pos < k)
            return leaves[pos++];
        else
            throw out_of_range("Loser Tree is empty!\n");
        
    }
};
#endif // !LOSER_TREE_H



