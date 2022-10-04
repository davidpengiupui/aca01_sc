#include "../include/Pseudo_LRU.h"
int base_two = 2;

Pseudo_LRU::Pseudo_LRU(int assoc)
{
    tree_bits.resize(assoc - 1);
    depth = ceil(log(assoc) / log(base_two));
    set_size = assoc;
}

void Pseudo_LRU::tree_access(int index)
{
    vector<int> idx_bits;
    int idx_copy = index;

    for (int i = 0; i < depth; i++)
    {
        idx_bits.push_back(idx_copy & 1);
        idx_copy >>= 1;
    }

    reverse(idx_bits.begin(), idx_bits.end());

    int j = 0;

    for (int i = 0; i < depth; i++)
    {
        if (idx_bits[i])
        {
            tree_bits[j] = idx_bits[i];
            j = 2 * j + 2;
        }
        else
        {
            tree_bits[j] = idx_bits[i];
            j = 2 * j + 1;
        }
    }
}

int Pseudo_LRU::tree_insert()
{
    
    int j = 0;
    int l = 0;
    int m = 0;
    int h = set_size - 1;

    for (int i = 0; i < depth; i++)
    {
        m = (l + h) / 2;
        if (tree_bits[j])
        {
            tree_bits[j] = 0;
            j = 2 * j + 1;
            h = m;
            m = h;
        }
        else
        {
            tree_bits[j] = 1;
            j = 2 * j + 2;
            l = m + 1;
            m = l;
        }
    }

    return m;
}