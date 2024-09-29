#include"LoserTree.h"
//#define LOSERTREE_MAIN
#ifndef LOSERTREE_MAIN
int main() {
    vector<int> leaves({ 3, 4, 2, 1, 99, 8, 7, 11, 66, 54, 31});
    int k = 11;
    LoserTree<int> lt(k, leaves);
    while (1)
    {
        if (lt.banCount >= k)
        {
            cout << "all ban" << endl;
            break;
        }
        try {
            int popVal = lt.getWinner();
            //lt.replaceWinnerAndBan(-1);//³É¹¦
            lt.replaceWinner(100);
            lt.banCount++;
            cout << popVal << endl;
        }
        catch (const out_of_range& e)
        {
            cout << "Caught an exception: " << e.what() << endl;
            break;
        }
    }
    return 0;
}
#endif // !LOSERTREE_MAIN