#include"LoserTree.h"
#define LOSERTREE_MAIN
#ifndef LOSERTREE_MAIN
int main() {
    FileProcessor fp;
    Buf buf(INPUT_BUF, 100);
    fp.loadMetaDataAndMallocBuf(buf);
    fp.readfile2buffer(buf);
    vector<int32_t> tempdata(fp.dataAmount, 0);
    int32_t* nums = reinterpret_cast<int32_t*>(buf.buffer);
    for (int i = 0; i < fp.dataAmount && i < 100; ++i)
    {
        tempdata[i] = nums[i];
    }

    int k = 8;
    LoserTree<int> lt(k, tempdata);
    int pos = k;
    int32_t curMin = INT32_MAX;
    while (pos < buf.actualSize)
    {
        if (lt.banCount >= k)
        {
            cout << "all ban" << endl;
            break;
        }
        try {
            int popVal = lt.getWinner();
            cout <<"curWinner :" << popVal << endl;
            curMin = popVal;

            cout << "nums[" << pos << "] :" << nums[pos] << endl;
            if (nums[pos] < curMin)
                lt.replaceWinnerAndBan(nums[pos++]);//³É¹¦
            else
                lt.replaceWinner(nums[pos++]);
            //lt.replaceWinner(100);
            //lt.banCount++;
            buf.actualSize--;
            
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