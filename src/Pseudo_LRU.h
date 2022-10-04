#pragma once
#include "Unit.h"

class Pseudo_LRU
{
    private:
        vector<int> tree_bits;
        int depth;
        int set_size;

    public:

        Pseudo_LRU(int assoc);
        void tree_access(int index);
        int tree_insert();
        
};