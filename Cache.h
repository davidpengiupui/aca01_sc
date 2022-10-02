#pregma once
#include "Unit.h"


class Cache
{
    private:
        int cache_level;

        // simulation results
        int reads = 0, read_misses = 0, writes = 0, write_misses = 0, writebacks = 0;

        vector<int> lru_counter;
        vector<PseudoLRU> trees;
        vector<int> trace;

    public:
        int size, total_mem_traffic = 0;
        int block_size, assoc, replacement, inclusion;
        int num_sets, tag_bits, index_bits, offset_bits;
        int tag_mask, index_mask, offset_mask;
        
        vector<vector<Unit>> cache;

        Cache(int level, int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop, vector<int> access_stream);
        void print_details();
        void access(int32_t bit_address, string mode, int trace_index);
        void print_results();
};
