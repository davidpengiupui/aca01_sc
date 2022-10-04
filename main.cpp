#include "include/Unit.h"
#include "include/utils.h"
#include "include/Pseudo_LRU.h"


int graph = 0;
int base = 2;
vector<int> l1_stream;
vector<int> l2_stream;

void extend_cache(int bit_adr, string mode, int trace_idx, int level);

class Cache
{
private:
    int cache_level;

    int read_times = 0;
    int read_misses = 0;
    int write_times = 0;
    int write_misses = 0;
    int write_back_times = 0;

    vector<int> order_counts;
    vector<Pseudo_LRU> pseudo_lru_trees;
    vector<int> trace;

public:
    int size = 0;
    int total_traffic = 0;
    int block_size, assoc_size;
    int replc_policy, inclu_policy;
    int num_sets, tag_bits, idx_bits, ost_bits;

    vector<vector<Unit>> cache;
    Cache(int level, int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop, vector<int> access_stream);
    void print_cache();
    void access_cache(int32_t bit_address, string mode, int trace_index);
    void results_visual();
};

Cache l1(0, 0, 0, 0, 0, 0, {0});
Cache l2(0, 0, 0, 0, 0, 0, {0});

void Print_Int_Vector(const vector<int> &v)
{
    // vector<int> v;
    for (int i = 0; i < v.size(); i++)
    {
        cout << v[i] << endl;
    }
    cout << v.size() << endl;
}

Cache::Cache(int level, int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop, vector<int> access_stream)
{
    cache_level = level;
    block_size = b_size;
    size = cache_size;
    assoc_size = cache_assoc;
    replc_policy = rep_pol;
    inclu_policy = inc_prop;
    trace = access_stream;

    // cout << "size: " << size << endl;

    if (size > 0)
    {
        // cout << "size: " << size << endl; size = 1024
        // cout << "assoc: " << assoc << endl; assoc = 2
        // cout << "block_size: " << block_size << endl; block_size = 16

        num_sets = size / (assoc_size * block_size);
        // cout << "num_sets: " << num_sets << endl; num_sets = 32
        // int base = 2;
        idx_bits = log(num_sets) / log(base); // 5 bits
        // cout << "index_bits: " << index_bits << endl;

        // bool judge = (log(num_sets) / log(base) == index_bits);
        // cout << "judge: " << (log(num_sets) / log(base)) << endl;
        // cout << "judge: " << judge << endl;

        ost_bits = log(block_size) / log(base); // 4 bits
        // cout << "offset_bits: " << offset_bits << endl;
        // judge = (log(block_size) / log(base) == offset_bits);
        // cout << "judge: " << (log(block_size) / log(base)) << endl;
        // out << "judge: " << judge << endl;

        tag_bits = 32 - idx_bits - ost_bits; // 32 - 5 - 4 = 23 bits
        // cout << "tag_bits: " << tag_bits << endl;

        // cout << "cache size: " << cache.size() << endl;
        cache.resize(num_sets); // 32
        // cout << "cache size: " << cache.size() << endl;

        if (replc_policy == 0 || replc_policy == 3)
        {
            // cout << "lru before replacement: " << endl;
            // Print(lru_counter); 0
            order_counts.resize(num_sets);
            // cout << "lru after replacement: " << endl;
            // Print(lru_counter); 32
        }
        else if (replc_policy == 1)
        {
            for (int i = 0; i < num_sets; i++)
            {
                pseudo_lru_trees.push_back(Pseudo_LRU(assoc_size));
            }
        }

        for (int i = 0; i < cache.size(); i++)
        {
            cache[i].resize(assoc_size);
            // cout << "i = " << i << endl;
            // Print_Line(cache[i][0]);
        }
    }
}

void Cache::print_cache()
{
    cout << "===== L" << cache_level << " contents =====" << endl;

    for (int i = 0; i < cache.size(); i++)
    {
        cout << "Set " << i << ":\t";

        for (int j = 0; j < cache[i].size(); j++)
        {
            // cout << "i = " << i << endl;
            // cout << "j = " << j << endl;
            // Print_Unit(cache[i][j]);
            stringstream ss;
            ss << hex << cache[i][j].tag;
            string dirty;

            if (cache[i][j].dty)
                dirty = " D";
            else
                dirty = "  ";

            cout << std::setw(5) << ss.str() << dirty << '\t';
        }

        cout << endl;
    }
}

void Cache::access_cache(int32_t bit_address, string mode, int trace_index)
{
    int ost;
    int idx;
    int tag;
    int32_t adr_copy = bit_address;

    // cout << "bit_address: " << bit_address << endl; // 1073940652

    ost %= block_size;
    // cout << "offset: " << offset << endl;

    // get block # (/ 16) = 67121290
    adr_copy /= block_size;
    // cout << "address_copy: " << address_copy << endl;

    // get # inside the block = 10 (% 32) last 5 index
    idx = adr_copy % num_sets;
    // cout << "index: " << index << endl;

    // get tag (/ 32) = 2097540
    // cout << "address_copy: " << address_copy << endl;

    tag = adr_copy / num_sets;
    // cout << "tag: " << tag << endl;

    if (mode == "w")
    {
        write_times += 1;
    }
    else
    {
        read_times += 1;
    }

    int sign_idx = -1;

    for (int i = 0; i < cache[idx].size(); i++)
    {
        if (cache[idx][i].vld)
        {
            // hit part
            if (cache[idx][i].tag == tag)
            {
                // cout << "index: " << index << endl;
                // cout << "size: " << cache[index].size() << endl; // size = 2(assoc)
                // cout << "bit_address: " << bit_address << endl;
                // cout << "i: " << i << endl;
                // cout << "tag: " << tag << endl;
                // cout << "check 1: " << endl;
                // Print_Unit(cache[index][i]);
                if (mode == "w") cache[idx][i].dty = 1;
                
                // cout << "check 2: " << endl;
                // Print_Unit(cache[index][i]);

                // update counts
                if (replc_policy == 0)
                {

                    cache[idx][i].order_counts = order_counts[idx];
                    order_counts[idx] += 1;
                    // cout << "check 3: " << endl;
                    // Print_Line(cache[index][i]);
                }

                else if (replc_policy == 1)
                    pseudo_lru_trees[idx].tree_access(i);
                

                return;
            }
        }
        else if (sign_idx == -1)
        {
            // cout << "fill out invalid to i" << endl;
            // cout << "i = " << i << endl;
            sign_idx = i;
        }
    }

    // miss and non-touch part
    if (sign_idx != -1)
    {

        if (replc_policy == 0)
        {
            // Print_Unit(cache[index][invalid_index]);
            cache[idx][sign_idx] = Unit(1, 1, tag, bit_address, order_counts[idx]);
            order_counts[idx] += 1;
        }

        else if (replc_policy == 1)
        {
            pseudo_lru_trees[idx].tree_access(sign_idx);
            cache[idx][sign_idx] = Unit(1, 1, tag, bit_address);
        }

        else if (replc_policy == 2)
        {
            cache[idx][sign_idx] = Unit(1, 1, tag, bit_address);
        }

        else if (replc_policy == 3)
        {
            // Print_Unit(cache[index][invalid_index]);
            cache[idx][sign_idx] = Unit(1, 1, tag, bit_address, order_counts[idx]);
            order_counts[idx] += 1;
        }

        if (cache_level == 1)
        {
            extend_cache(bit_address, "r", trace_index, 2);
        }

        if (mode == "r")
        {
            cache[idx][sign_idx].dty = 0;
        }
    }
    else
    {
        // miss & replace
        int replc_idx;

        if (replc_policy == 0 || replc_policy == 3)
        {
            int min_count = INT32_MAX;

            for (int i = 0; i < cache[idx].size(); i++)
            {
                if (cache[idx][i].order_counts < min_count)
                {
                    min_count = cache[idx][i].order_counts;
                    replc_idx = i;
                }
            }
        }

        else if (replc_policy == 1)
            replc_idx = pseudo_lru_trees[idx].tree_insert();

        else if (replc_policy == 2)
        {
            vector<int> ost;
            int max_ost = -1;
            int next_use;

            for (int i = 0; i < cache[idx].size(); i++)
            {
                next_use = preview_trace(cache[idx][i].adr >> ost_bits, trace_index, trace);
                ost.push_back(next_use);
            }

            for (int i = 0; i < ost.size(); i++)
            {
                if (ost[i] > max_ost)
                {
                    max_ost = ost[i];
                    replc_idx = i;
                }
            }
        }

        // write back
        if (cache[idx][replc_idx].dty)
        {
            write_back_times += 1;

            if (cache_level == 1)
            {
                extend_cache(cache[idx][replc_idx].adr, "w", trace_index, 2);
            }

            if (cache_level == 2 && inclu_policy)
            {
                extend_cache(cache[idx][replc_idx].adr, "w", trace_index, 1);
            }
        }

        if (replc_policy == 0 || replc_policy == 3)
        {
            cache[idx][replc_idx] = Unit(1, 0, tag, bit_address, order_counts[idx]);
            order_counts[idx] += 1;
        }
        else
            cache[idx][replc_idx] = Unit(1, 0, tag, bit_address);

        if (cache_level == 1)
            extend_cache(bit_address, "r", trace_index, 2);

        if (mode == "w")
            cache[idx][replc_idx].dty = 1;
    }

    if (mode == "w")
    {
        write_misses += 1;
    }

    else
    {
        read_misses += 1;
    }

    return;
}

void Cache::results_visual()
{
    float miss_rate;

    miss_rate = (float)(read_misses + write_misses) / (float)(read_times + write_times);
    total_traffic += read_misses + write_misses + write_back_times;

    if (size > 0 && graph != 0)
    {
        if (graph == 1 || graph == 4)
        {

            if (size == 0)
                cout << "0";

            else
            {
                if (cache_level == 2)
                    cout << (float)(read_misses) / (float)(read_times);

                else
                    cout << miss_rate;
            }
            cout << endl;
        }
        else if (graph >= 2 && graph < 4)
            cout << read_times << "," << read_misses << "," << write_times << "," << write_misses << endl;
    }

    if (!graph)
    {
        if (cache_level == 1)
        {
            cout << "a. number of L1 reads:\t" << read_times << endl;
            cout << "b. number of L1 read misses:\t" << read_misses << endl;
            cout << "c. number of L1 writes:\t" << write_times << endl;
            cout << "d. number of L1 write misses:\t" << write_misses << endl;
            cout << "e. L1 miss rate:\t" << fixed << setprecision(6) << miss_rate << endl;
            cout << "f. number of L1 writebacks:\t" << write_back_times << endl;
        }
        else
        {
            cout << "g. number of L2 reads:\t" << read_times << endl;
            cout << "h. number of L2 read misses:\t" << read_misses << endl;
            cout << "i. number of L2 writes:\t" << write_times << endl;
            cout << "j. number of L2 write misses:\t" << write_misses << endl;
            cout << "k. L2 miss rate:\t";

            if (size == 0)
            {
                cout << "0" << endl;
            }
            else
            {
                miss_rate = (float)(read_misses) / (float)(read_times);
                cout << fixed << setprecision(6) << miss_rate << endl;
            }

            cout << "l. number of L2 writebacks:\t" << write_back_times << endl;
        }
    }
}

void extend_cache(int bit_adr, string mode, int trace_idx, int level)
{
    if (level == 2 && l2.size > 0)
    {
        l2.access_cache(bit_adr, mode, trace_idx);
    }
    else if (level == 1)
    {
        int ost;
        int idx;
        int tag;

        ost = bit_adr % l1.block_size;

        bit_adr /= l1.block_size;

        idx = bit_adr % l1.num_sets;

        tag = bit_adr / l1.num_sets;

        for (int i = 0; i < l1.cache[idx].size(); i++)
        {
            if (l1.cache[idx][i].vld)
            {

                if (l1.cache[idx][i].tag == tag)
                {

                    l1.cache[idx][i].vld = 0;

                    if (l1.cache[idx][i].dty) l2.total_traffic += 1;
                    

                    return;
                }
            }
        }
    }

    return;
}

int main(int argc, char *argv[])
{
    int BLOCKSIZE = stoi(argv[1]);
    // int block_size = 16;
    int L1_SIZE = stoi(argv[2]);
    // int l1_size = 1024;
    int L1_ASSOC = stoi(argv[3]);
    // int l1_assoc = 2;
    int L2_SIZE = stoi(argv[4]);
    // int l2_size = 0;
    int L2_ASSOC = stoi(argv[5]);
    // int l2_assoc = 0;
    int REPLACEMENT_POLICY = stoi(argv[6]);
    // int replacement = 0;
    int INCLUSION_PROPERTY = stoi(argv[7]);
    // int inclusion = 0;
    string trace_name = argv[8];
    // string trace_name = "gcc_trace.txt";
    string trace_path = "../traces/" + trace_name;

    if (argc > 9)
    {
        graph = stoi(argv[9]);
    }
    // graph = false;

    // ******************************************************************************* //
    // sample execution command:
    // ./sim_cache  16 1024 2 0 0 0 0 "../traces/gcc_trace.txt"
    // ******************************************************************************* //

    if (!graph)
    {
        // cout << "Qucheng Peng is here!!!" << endl;
        cout << "===== Simulator configuration =====" << endl;
        cout << "BLOCKSIZE:\t" << BLOCKSIZE << endl;
        cout << "L1_SIZE:\t" << L1_SIZE << endl;
        cout << "L1_ASSOC:\t" << L1_ASSOC << endl;
        cout << "L2_SIZE:\t" << L2_SIZE << endl;
        cout << "L2_ASSOC:\t" << L2_ASSOC << endl;
        cout << "REPLACEMENT POLICY:\t";

        switch (REPLACEMENT_POLICY)
        {
        case 0:
            cout << "LRU";
            break;

        case 1:
            cout << "Pseudo-LRU";
            break;

        case 2:
            cout << "Optimal";
            break;

        case 3:
            cout << "FIFO";
            break;

        default:
            cout << "LRU";
            break;
        }

        cout << endl;
        cout << "INCLUSION PROPERTY:\t";

        if (INCLUSION_PROPERTY)
            cout << "inclusive";

        else
            cout << "non-inclusive";

        cout << endl;
        cout << "trace_file:\t" << trace_name << endl;
    }

    // optimize trace file for optimal LRU
    if (REPLACEMENT_POLICY == 2)
    {
        l1_stream = optim_trace_file(trace_path, log2(BLOCKSIZE));
        if (L2_SIZE > 0)
            l2_stream = optim_trace_file(trace_path, log2(BLOCKSIZE));
    }

    l1 = Cache(1, BLOCKSIZE, L1_SIZE, L1_ASSOC, REPLACEMENT_POLICY, INCLUSION_PROPERTY, l1_stream);
    l2 = Cache(2, BLOCKSIZE, L2_SIZE, L2_ASSOC, REPLACEMENT_POLICY, INCLUSION_PROPERTY, l2_stream);

    fstream trace_file;

    trace_file.open(trace_path, std::ios::in);

    if (trace_file.is_open())
    {
        int count = 0;
        string file_line;
        string mode;
        string address;
        int res;

        while (trace_file >> mode >> address)
        {

            if (!isalpha(mode[0]))
                mode = mode[mode.length() - 1];

            // cout << "address: " << address << endl;
            // cout << "input: " << stoi(address, nullptr, 16) << endl;
            int32_t address_trans = stoi(address, nullptr, 16);

            l1.access_cache(address_trans, mode, count);
            count += 1;
        }

        if (!graph)
        {
            l1.print_cache();
            if (L2_SIZE > 0)
                l2.print_cache();
        }
    }

    trace_file.close();

    if (!graph)
        cout << "===== Simulation results (raw) =====" << endl;

    l1.results_visual();
    l2.results_visual();

    if (!graph)
    {
        if (L2_SIZE > 0)
            cout << "m. total memory traffic:\t" << l2.total_traffic << endl;

        else
            cout << "m. total memory traffic:\t" << l1.total_traffic << endl;
    }

    return 0;
}