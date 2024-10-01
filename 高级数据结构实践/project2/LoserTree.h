#ifndef LOSER_TREE_H
#define LOSER_TREE_H
#include<iostream>
#include<vector>
#include<stdexcept> // 用于异常处理
#include<algorithm>
#include"../fileprocess/FileProcessor.h"
using namespace std;


template<typename T>
class LoserTree {
public:
    vector<int> tree;       // Loser Tree 数组, [0]用于存储最终胜者, 存储的是节点索引
    vector<T> leaves;           // 叶子结点，存储要排序的数据
    vector<bool> competitor;    // 叶子是否能参加比赛数组
    int banCount;               // 被禁赛的计数
    int k;                      // 叶子结点的数量
    int pos;                    // 树的堆弹出
    bool isSort;                // 叶子是否排序，用于pop

public:
    // 构造函数
    LoserTree(int k, vector<T>& input) : k(k), leaves(input), banCount(0) {
        tree.resize(k, -1);          // 初始化 loser tree 的内部节点为 -1
        //wintree.resize(k, -1);
        competitor.resize(k, true);  // 初始化所有叶子节点为可参加比赛
        pos = 0;
        isSort = false;
        build();
    }

    // 构建 loser tree
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
        int winner = idx;//当前胜利者在leaves数组中的实际索引
        int curTreeIdx = idx + k;//当前操作的整个树的节点的抽象索引
        int parent = curTreeIdx / 2;//当前操作节点的父节点
        while (parent > 0)
        {
            int bro = (curTreeIdx % 2 == 0) ? parent * 2 + 1 : parent;//当前操作节点的兄弟节点
            int realbroIdx = -1;
            if (bro >= k)//当前还在叶节点层
                realbroIdx = bro - k;
            else
                realbroIdx = wintree[bro];//可能是-1

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
            curTreeIdx = parent;//原先的父节点变成下一轮的操作节点
            parent /= 2;
        }
        tree[0] = winner;
    }


    //使用于产生不同的归并段，当替换并禁赛
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

    // 更新 loser tree，指定哪个叶子结点发生了变化
    void update(int leaf) {
        int parent = (leaf + k) / 2;  // 计算父节点位置
        int winner = leaf;             // 当前赢家为新添加的叶子节点

        //新进入禁赛的一定是最小的
        //所以更新的时父节点的败者就变成了
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
            parent /= 2; // 移动到下一个父节点
        }
        tree[0] = winner; // 最终的赢家保存在根节点
    }

    // 获取赢家元素（即最小的元素）
    T getWinner() {
        return leaves[tree[0]];
    }

    // 添加一个新值，并重新计算赢家
    void replaceWinner(T newValue) {
        if (competitor[tree[0]]) {
            leaves[tree[0]] = newValue;  // 将叶子结点值替换为新值
            update(tree[0]);  // 更新 loser tree
        }
        else {
            // 当前赢家禁赛，寻找其他可参赛的赢家
            int newWinner = -1;
            for (int i = 0; i < k; ++i) {
                if (competitor[i] && (newWinner == -1 || leaves[i] < leaves[newWinner])) {
                    newWinner = i;
                }
            }
            if (newWinner != -1) {
                leaves[newWinner] = newValue; // 更新新的赢家
                update(newWinner);  // 更新 loser tree
            }
        }
    }

    // 当所有节点都被禁赛，说明已经完全产生一个 runfile，这时候需要重新开始比赛
    void reCompete() {
        fill(competitor.begin(), competitor.end(), true);
        fill(tree.begin(), tree.end(), -1);
        banCount = 0;
        build();
    }

    // 禁用指定的叶子节点
    int disqualify(int index) {
        int oldWinner = tree[0];
        if (index >= 0 && index < k) {
            competitor[index] = false; // 将指定叶子节点标记为禁赛
            banCount++;
            // 更新树以确保当前赢家仍然是有效的
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



