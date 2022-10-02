#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <bitset>
#include <iomanip>
#include "Pseudo_LRU.h"
#include "Unit.h"
#include "Cache.h"

using namespace std;

void external_cache_access(int bit_address, string mode, int trace_index, int level);

vector<int> access_stream_l1;
vector<int> access_stream_l2;
int silent = 0;


void Print (const vector<int>& v){
  //vector<int> v;
  for (int i=0; i<v.size();i++){
    cout << v[i] << endl;
  }
  cout << v.size() << endl;
}



Cache l1(0, 0, 0, 0, 0, 0, {0});
Cache l2(0, 0, 0, 0, 0, 0, {0});

// Wrapper method for cache access that enables accessing another cache within 
// one (i.e. writing back to L2 within L1)
/*
 * bit_address (int): the full hex address from the address sequence/evicted block, in decimal form
 * mode (string): either "r" (read) or "w" (write); only applicable when writing back to L2
 * trace_index (int): index of the address we're processing in the address sequence; only for optimal replacement pol
 * level (int): which cache level we are trying to access; 2 for L2 (acc. from L1), 1 for accessing L1 from L2
 */
void external_cache_access(int bit_address, string mode, int trace_index, int level)
{
    if (level == 2 && l2.size > 0)
    {
        l2.access(bit_address, mode, trace_index);
    }
    else if (level == 1)
    {
        int offset, index, tag;
        
        offset = bit_address & l1.offset_mask;
        bit_address >>= l1.offset_bits;
        index = bit_address & l1.index_mask;
        bit_address >>= l1.index_bits;
        tag = bit_address;

        // find the block in the L1 cache where it's supposed to be
        for (int i = 0; i < l1.cache[index].size(); i++)
        {
            if (l1.cache[index][i].valid)
            {
                // compare tags
                if (l1.cache[index][i].tag == tag)
                {
                    // allow this block to be replaced later on by marking it invalid
                    l1.cache[index][i].valid = 0;

                    // also, if it's dirty, write it to main memory
                    if (l1.cache[index][i].dirty)
                    {
                        l2.total_mem_traffic++;
                    }

                    return;
                }
            }
        }
    }

    return;
}

// commandline args:
/* 
 * BLOCKSIZE: int
 * L1_SIZE: int
 * L1_ASSOC: int
 * L2_SIZE: int
 * L2_ASSOC: int
 * REPLACEMENT_POLICY: int (0: LRU, 1: PLRU, 2: optimal)
 * INCLUSION_PROPERTY: int (0: non-inclusive, 1: inclusive)
 * trace_file: string (trace file path with extension)
 * silent: int; whether to print full simulation results (0/don't specify), or print
 *         results needed for the report graphs (to be fed into my python script):
 *         (1) - graph #1 (L1/L2 (if necessary) MR & assoc. vs. log2(SIZE))
 *         (2) - graph #2 (prints reads, read misses, writes, and write misses)
 *         (3) - graph #3 (same as 2)
 *         (4) - graph #4 (same as 1)
 */
int main(int argc, char *argv[])
{
    //int block_size = stoi(argv[1]);
    int block_size = 16;
    //int l1_size = stoi(argv[2]);
    int l1_size = 1024;
    //int l1_assoc = stoi(argv[3]);
    int l1_assoc = 2;
    //int l2_size = stoi(argv[4]);
    int l2_size = 0;
    //int l2_assoc = stoi(argv[5]);
    int l2_assoc = 0;
    //int replacement = stoi(argv[6]);
    int replacement = 0;
    //int inclusion = stoi(argv[7]);
    int inclusion = 0;
    //string trace_path = argv[8];
    string trace_path = "gcc_trace.txt";
    
    /*if (argc > 9)
    {
        silent = stoi(argv[9]);
    }*/
    silent = false;

    // sample run cmd:
    // ./sim_cache 16 1024 2 0 0 0 0 ../trace_files/gcc_trace.txt

    if (!silent)
    {
        cout << "Qucheng Peng is here!!!" << endl;
        cout << "===== Simulator configuration =====" << endl;
        cout << "BLOCKSIZE:\t\t\t" << block_size << endl;
        cout << "L1_SIZE:\t\t\t" << l1_size << endl;
        cout << "L1_ASSOC:\t\t\t" << l1_assoc << endl;
        cout << "L2_SIZE:\t\t\t" << l2_size << endl;
        cout << "L2_ASSOC:\t\t\t" << l2_assoc << endl;
        cout << "REPLACEMENT POLICY:\t";

        switch (replacement)
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

            default:
                cout << "LRU";
                break;
        }

        cout << endl;
        cout << "INCLUSION PROPERTY:\t";

        if (inclusion)
        {
            cout << "inclusive";
        }
        else
        {
            cout << "non-inclusive";
        }

        cout << endl;
        cout << "trace_file:\t\t" << trace_path << endl;
    }
    
    // preprocess the trace files for optimal replacement pol
    if (replacement == 2)
    {
        access_stream_l1 = preprocesses_trace(trace_path, log2(block_size));
        if (l2_size > 0)
        {
            access_stream_l2 = preprocesses_trace(trace_path, log2(block_size));
        }
    }

    l1 = Cache(1, block_size, l1_size, l1_assoc, replacement, inclusion, access_stream_l1);
    l2 = Cache(2, block_size, l2_size, l2_assoc, replacement, inclusion, access_stream_l2);

    // read address sequence from file, line by line
    fstream trace_file;

    trace_file.open(trace_path, ios::in);

    if (trace_file.is_open())
    {
        int count = 0;
        string file_line;
        string mode;
        string address;
        int res;

        while (trace_file >> mode >> address)
        {

            // in case the file/line starts with some garbage chars, skip them
            if (!isalpha(mode[0]))
            {
                mode = mode[mode.length() - 1];
            }
            
            cout << "address: " << address << endl;
            cout << "input: " << stoi(address, nullptr, 16) << endl;
            int32_t address_trans = stoi(address, nullptr, 16);

            l1.access(address_trans, mode, count);
            count++;
        }

        if (!silent)
        {
            l1.print_details();
            if (l2_size > 0)
            {
                l2.print_details();
            }
        }
    }

    trace_file.close();

    if (!silent)
    {
        cout << "===== Simulation results (raw) =====" << endl;
    }

    l1.print_results();
    l2.print_results();
    
    if (!silent)
    {
        if (l2_size > 0)
        {
            cout << "m. total memory traffic:\t" << l2.total_mem_traffic << endl;
        }
        else
        {
            cout << "m. total memory traffic:\t" << l1.total_mem_traffic << endl;
        }
    }

    return 0;
}
