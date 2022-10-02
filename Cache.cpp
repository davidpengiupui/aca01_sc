#include"Cache.h"

Cache::Cache(int level, int b_size, int cache_size, int cache_assoc, int rep_pol, int inc_prop, vector<int> access_stream)
{
cache_level = level;
block_size = b_size;
size = cache_size;
assoc = cache_assoc;
replacement = rep_pol;
inclusion = inc_prop;
trace = access_stream;

//cout << "size: " << size << endl; 

if (size > 0)
{
    //cout << "size: " << size << endl; size = 1024
    //cout << "assoc: " << assoc << endl; assoc = 2
    //cout << "block_size: " << block_size << endl; block_size = 16

    num_sets = size / (assoc * block_size); 
    // cout << "num_sets: " << num_sets << endl; num_sets = 32
    int base = 2;
    index_bits = log(num_sets) / log(base); // 5 bits
    // cout << "index_bits: " << index_bits << endl;

    //bool judge = (log(num_sets) / log(base) == index_bits);
    //cout << "judge: " << (log(num_sets) / log(base)) << endl;
    //cout << "judge: " << judge << endl;


    offset_bits = log(block_size) / log(base); // 4 bits
    //cout << "offset_bits: " << offset_bits << endl;
    //judge = (log(block_size) / log(base) == offset_bits);
    //cout << "judge: " << (log(block_size) / log(base)) << endl;
    //out << "judge: " << judge << endl;

    tag_bits = 32 - index_bits - offset_bits; // 32 - 5 - 4 = 23 bits 
    //cout << "tag_bits: " << tag_bits << endl;

    //index_mask = pow(base, index_bits) - 1; // 31
    //offset_mask = pow(base, offset_bits) - 1; // 15
    //cout << "offset_mask: " << offset_mask << endl;

    //cout << "index_mask: " << index_mask << endl;

    //cout << "cache size: " << cache.size() << endl;
    cache.resize(num_sets); // 32
    //cout << "cache size: " << cache.size() << endl;


    if (replacement == 0)
    {
        //cout << "lru before replacement: " << endl;
        //Print(lru_counter); 0
        lru_counter.resize(num_sets);
        //cout << "lru after replacement: " << endl;
        //Print(lru_counter); 32
    }
    else if (replacement == 1)
    {
        for (int i = 0; i < num_sets; i++)
        {
            trees.push_back(PseudoLRU(assoc));
        }
    }

    for (int i = 0; i < cache.size(); i++)
    {
        cache[i].resize(assoc);
        //cout << "i = " << i << endl;
        //Print_Line(cache[i][0]);

    }
}
}

// print the cache's contents in the following format:
// Set n: <line 0 hex tag> <dirty?> ... <line m hex tag> <dirty?>
// where n is the current set number and m is the associativity of the cache
void Cache::print_details()
{
cout << "===== L" << cache_level << " contents =====" << endl;

for (int i = 0; i < cache.size(); i++)
{
    cout << "Set " << i << ":\t\t";

    for (int j = 0; j < cache[i].size(); j++)
    {
        //cout << "i = " << i << endl;
        //cout << "j = " << j << endl;
        //Print_Line(cache[i][j]);
        stringstream ss;
        ss << hex << cache[i][j].tag;
        string dirty;

        if (cache[i][j].dirty)
        {
            dirty = " D";
        }
        else
        {
            dirty = "  ";
        }

        cout << setw(8) << ss.str() << dirty << '\t';
    }

    cout << endl;
}
}

// attempt to read or write to this cache, handling misses and writebacks as necessary
// address (string): the full hex address from the address sequence
// mode (string): the access mode, either read (r) or write (w); also extracted from the address sequence
// trace_index(int): the current number of addresses in the addr. sequence (trace) we have progressed through
void Cache::access(int32_t bit_address, string mode, int trace_index)
{
int offset, index, tag;

int32_t address_copy = bit_address;

//cout << "bit_address: " << bit_address << endl; // 1073940652

//offset = address_copy & offset_mask; // remainder = 12 (% 16) last 4 bits
offset %= block_size;
//cout << "offset: " << offset << endl;

//address_copy >>= offset_bits; // get block # (/ 16) = 67121290
address_copy /= block_size;
//cout << "address_copy: " << address_copy << endl;

//index = address_copy & index_mask; // get # inside the block = 10 (% 32) last 5 index
index = address_copy % num_sets;
//cout << "index: " << index << endl;

//address_copy >>= index_bits; // get tag (/ 32) = 2097540 
//cout << "address_copy: " << address_copy << endl;

tag = address_copy / num_sets;
//cout << "tag: " << tag << endl;



if (mode == "w")
{
    writes += 1;
} 
else 
{
    reads += 1;
}

// for both reads and writes
int invalid_index = -1;

// find available valid blocks with matching tag
for (int i = 0; i < cache[index].size(); i++)
{
    if (cache[index][i].valid)
    {
        // if valid, compare the tags
        // hit
        if (cache[index][i].tag == tag)
        {
            //cout << "index: " << index << endl;
            //cout << "size: " << cache[index].size() << endl; // size = 2(assoc)
            //cout << "bit_address: " << bit_address << endl;
            //cout << "i: " << i << endl;
            //cout << "tag: " << tag << endl;
            //cout << "check 1: " << endl;
            //Print_Line(cache[index][i]);
            // we have a hit; if writing, mark the block as dirty
            if (mode == "w")
            {
                cache[index][i].dirty = 1;
            }
            //cout << "check 2: " << endl;
            //Print_Line(cache[index][i]);

            // do replacement policy-related updates
            if (replacement == 0)
            {
                // LRU
                //cout << "lru_count before: " << lru_counter[index];
                //lru_counter[index] += 1;
                cache[index][i].lru_count = lru_counter[index];
                lru_counter[index] += 1;
                //cout << "lru_count after: " << lru_counter[index];
                //cout << "check 3: " << endl;
                //Print_Line(cache[index][i]);
            }

            else if (replacement == 1)
            {
                // PLRU
                trees[index].access(i);
            }

            // no changes necessary for optimal (only consult oracle 
            // for foresight on evictions)

            // we had a hit
            return;
        }
    }
    else if (invalid_index == -1)
    {
        // maintain the earliest-seen invalid index in case it needs to be filled
        //cout << "fill out invalid to i" << endl;
        //cout << "i = " << i << endl;
        invalid_index = i;
    }
}

// if there was a miss and an invalid index, write to it and make it valid
if (invalid_index != -1)
{
    // for write misses, valid = 1 and dirty = 1
    // update LRU
    if (replacement == 0)
    {
        //Print_Line(cache[index][invalid_index]);
        cache[index][invalid_index] = Unit(1, 1, tag, bit_address, lru_counter[index]++);
    }

    // update Pseudo-LRU
    else if (replacement == 1)
    {
        // since this is an access, update the tree bit array
        trees[index].access(invalid_index);
        cache[index][invalid_index] = Unit(1, 1, tag, bit_address);
    }

    // no updates necessary for optimal, just write to the block
    else if (replacement == 2)
    {
        cache[index][invalid_index] = Unit(1, 1, tag, bit_address);
    }

    // for both non-inclusive and inclusive, issue a read to the next level 
    // cache (if not there already)
    if (cache_level == 1)
    {
        external_cache_access(bit_address, "r", trace_index, 2);
    }

    // if reading instead, mem will be up-to-date, so dirty = 0
    if (mode == "r")
    {
        cache[index][invalid_index].dirty = 0;
    }
}
else
{
    // if there were no invalid indices, evict the block as determined by the 
    // replacement policy
    int replacement_index;

    if (replacement == 0)
    {
        // find the LRU block
        int min_count = 1000000000000000;

        for (int i = 0; i < cache[index].size(); i++)
        {
            if (cache[index][i].lru_count < min_count)
            {
                min_count = cache[index][i].lru_count;
                replacement_index = i;
            }
        }                    
    }

    else if (replacement == 1)
    {
        // find the LRU block according to PLRU
        replacement_index = trees[index].replace();
    }

    else if (replacement == 2)
    {
        // optimal - use foresight to determine the block to replace
        // and replace the leftmost one in case multiple are not reused
        vector<int> offsets;
        int max_offset = -1;
        int next_use;

        for (int i = 0; i < cache[index].size(); i++)
        {
            // find the index at which this block is needed again (within THIS SET INDEX)
            next_use = foresight(cache[index][i].addr >> offset_bits, trace_index, trace);
            offsets.push_back(next_use);
        }

        // of all the offsets, find the maximum value; if any are equal 
        // to the size of the trace, they won't be used again so use the 
        // leftmost such block (in case there are ties)
        for (int i = 0; i < offsets.size(); i++)
        {
            if (offsets[i] > max_offset)
            {
                max_offset = offsets[i];
                replacement_index = i;
            }
        }
    }

    if (cache[index][replacement_index].dirty)
    {
        // if dirty, we must first write-back to memory (or next level cache) before evicting
        writebacks++;

        if (cache_level == 1)
        {
            // write the victim block to L2 (if it exists) - step 1 of allocation
            external_cache_access(cache[index][replacement_index].addr, "w", trace_index, 2);
        }

        // for inclusive outer caches only
        if (cache_level == 2 && inclusion)
        {
            external_cache_access(cache[index][replacement_index].addr, "w", trace_index, 1);
        }
    }

    if (replacement == 0)
    {
        //cache[index][replacement_index] = Line(1, 0, tag, bit_address, lru_counter[index]++);
        cache[index][replacement_index] = Unit(1, 0, tag, bit_address, lru_counter[index]);
        lru_counter[index] += 1;
    }
    else
    {
        cache[index][replacement_index] = Unit(1, 0, tag, bit_address);
    }

    // step 2 of allocating a block
    if (cache_level == 1)
    {
        external_cache_access(bit_address, "r", trace_index, 2);
    }

    if (mode == "w")
    {
        // if we wrote instead of read, dirty bit must be set
        cache[index][replacement_index].dirty = 1;
    }
}

// we had a miss
if (mode == "w") write_misses++; else read_misses++;

return;
}

// print the results of the simulation for this cache (depending on cache level)
void Cache::print_results()
{
float miss_rate = (float)(read_misses + write_misses) / (float)(reads + writes);
total_mem_traffic += read_misses + write_misses + writebacks;

// for arrangements of output, see my python script
if (size > 0 && silent != 0)
{
    if (silent == 1 || silent == 4)
    {
        // graph #1: L1 MR and assoc. vs size
        // graph #4: requires MR for AAT calculation
        if (size == 0)
        {
            cout << "0";
        }
        else
        {
            if (cache_level == 2)
            {
                cout << (float)(read_misses) / (float)(reads);
            }
            else
            {
                cout << miss_rate;
            }
        }
        cout << endl;
    }
    else if (silent >= 2 && silent < 4)
    {
        // graphs 2 and 3 (deal with AAT)
        // requires some postprocessing on the python side using 
        // CACTI table values and these values
        cout << reads << "," << read_misses << "," << writes << "," << write_misses << endl;
    }
}

if (!silent)
{
    if (cache_level == 1)
    {
        cout << "a. number of L1 reads:\t\t" << reads << endl;
        cout << "b. number of L1 read misses:\t" << read_misses << endl;
        cout << "c. number of L1 writes:\t\t" << writes << endl;
        cout << "d. number of L1 write misses:\t" << write_misses << endl;
        cout << "e. L1 miss rate:\t\t" << fixed << setprecision(6) << miss_rate << endl;
        cout << "f. number of L1 writebacks:\t" << writebacks << endl;
    }
    else
    {
        cout << "g. number of L2 reads:\t\t" << reads << endl;
        cout << "h. number of L2 read misses:\t" << read_misses << endl;
        cout << "i. number of L2 writes:\t\t" << writes << endl;
        cout << "j. number of L2 write misses:\t" << write_misses << endl;
        cout << "k. L2 miss rate:\t\t";

        if (size == 0)
        {
            cout << "0" << endl;
        }
        else
        {
            miss_rate = (float)(read_misses) / (float)(reads);
            cout << fixed << setprecision(6) << miss_rate << endl;
        }

        cout << "l. number of L2 writebacks:\t" << writebacks << endl;
    }
}
}

