#include <string>
#include <unordered_map>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

class Pseudo_LRU
{
    private:
        vector<int> tree_bits;
        int depth;
        int set_size;

    public:
        Pseudo_LRU(int assoc);
        void access(int index);
        int replace();
};

vector<int> preprocesses_trace(string filepath, int tagshift)
{
    vector<int> access_stream;
    fstream trace_file;

    trace_file.open(filepath, ios::in);

    if (trace_file.is_open())
    {
        string file_line, mode, address;

        while (trace_file >> mode >> address)
        {
            // in case there are extra chars in the front of the line
            if (!isalpha(mode[0]))
            {
                mode = mode[mode.length() - 1];
            }

            int32_t bit_address = stoi(address, nullptr, 16);
            bit_address >>= tagshift;
            access_stream.push_back(bit_address);
        }
    }

    trace_file.close();

    return access_stream;
}

// TODO: optimize this using a hashset or store next-usages on a set-by-set basis if time permits

// "predict the future" in the Belady optimal replacement algorithm by determining 
// which block in the line will be least urgently needed in the future
/*
 * query_address(int): the address we want to find when searching in the future
 * trace_index(int): our current progress through the access stream (the timestep t)
 * trace(vector<int>): the array of accesses from the preprocessing stage
 */
int foresight(int query_address, int trace_index, vector<int> trace)
{
    // determine when in the future this block is needed again
    for (int i = trace_index + 1; i < trace.size(); i++)
    {
        if (query_address == trace[i])
        {
            return i;
        }
    }

    // otherwise, it was never used again, so it should be replaced
    return trace.size();
};
