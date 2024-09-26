#ifndef LOSER_TREE_H
#define LOSER_TREE_H
#include<iostream>
#include<vector>
using namespace std;


template<typename T>
class LoserTree {
public:
    vector<int> tree;           // Loser Tree 数组, [0]用于存储胜者, 存储的是节点索引
    vector<T> leaves;           // 叶子结点，存储要排序的数据
    vector<bool> competitor;    // 叶子是否能参加比赛数组
    int banCount;
    int k;                      // 叶子结点的数量

public:
    // 构造函数
    LoserTree(int k, vector<T>& input) : k(k), leaves(input), banCount(0) {
        tree.resize(k, -1);          // 初始化 loser tree 的内部节点为 -1
        competitor.resize(k, true);  // 初始化所有叶子节点为可参加比赛
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
        int winner = leaf;             // 当前赢家为新添加的叶子节点

        while (parent > 0) {
            // 如果当前叶子节点不能参加比赛，则跳过
            if (!competitor[winner]) {
                winner = tree[parent]; // 选择父节点的赢家
            }
            else if (tree[parent] != -1 && !competitor[tree[parent]]) {
                // 如果父节点的赢家被禁赛，则选择当前赢家
                // 比较当前赢家和父节点的叶子
                winner = leaf; // 当前叶子节点作为赢家
            }
            else if (leaves[winner] > leaves[tree[parent]]) {
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

    // 添加一个新值，并重新计算赢家
    void replaceWinner(T newValue) {
        // 检查当前赢家是否能参加比赛
        if (competitor[tree[0]]) {
            leaves[tree[0]] = newValue;  // 将叶子结点值替换为新值
            update(tree[0]);  // 更新 loser tree
        }
        else {
            // 当前赢家禁赛，寻找其他可参赛的赢家
            int newWinner = -1;
            for (int i = 0; i < k; ++i) {
                if (competitor[i]) {
                    if (newWinner == -1 || leaves[i] < leaves[newWinner]) {
                        newWinner = i;
                    }
                }
            }
            if (newWinner != -1) {
                leaves[newWinner] = newValue; // 更新新的赢家
                update(newWinner);  // 更新 loser tree
            }
        }
    }

    //当所有节点都被禁赛，说明已经完全产生一个runfile,这时候需要重新开始比赛
    void reCompete()
    {
        fill(competitor.begin(), competitor.end(), true);
        banCount = 0;
        build();
    }

    // 禁用指定的叶子节点
    void disqualify(int index) {
        if (index >= 0 && index < k) {
            competitor[index] = false; // 将指定叶子节点标记为禁赛
            banCount++;
            // 更新树以确保当前赢家仍然是有效的
            update(tree[0]);
        }
    }
};
#endif // !LOSER_TREE_H
