#pragma once
#include "Unit.h"


vector<int> optim_trace_file(string trace_file_path, int tag_shift_bits)
{
    vector<int> access_stream;
    fstream trace_file;

    trace_file.open(trace_file_path, std::ios::in);

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
            bit_address >>= tag_shift_bits;
            access_stream.push_back(bit_address);
        }
    }

    trace_file.close();

    return access_stream;
}


int preview_trace(int query_adr, int trace_idx, vector<int> trace)
{
    // determine when in the future this block is needed again
    for (int i = trace_idx + 1; i < trace.size(); i++)
    {
        if (query_adr == trace[i])
        {
            return i;
        }
    }

    // otherwise, it was never used again, so it should be replaced
    return trace.size();
};

