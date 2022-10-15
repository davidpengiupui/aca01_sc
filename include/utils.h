#pragma once
#include "Unit.h"


vector<int> optim_trace_file(string trace_file_path, int tag_shift_bits)
{
    vector<int> access_stream;
    fstream trace_file;

    trace_file.open(trace_file_path, std::ios::in);

    if (trace_file.is_open())
    {
        string file_line; 
        string mode;
        string adr;
        int32_t bit_adr;

        while (trace_file >> mode >> address)
        {
            if (!isalpha(mode[0])) 
                mode = mode[mode.length() - 1];
            
            bit_adr = stoi(adr, nullptr, 16);
            bit_adr >>= tag_shift_bits;
            access_stream.push_back(bit_adr);
        }
    }

    trace_file.close();

    return access_stream;
}


int preview_trace(int query_adr, int trace_idx, vector<int> trace)
{
    for (int i = trace_idx + 1; i < trace.size(); i++)
        if (query_adr == trace[i])
            return i;
    
    return trace.size();
};

