#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <bitset>
#include <iomanip>

using namespace std;

class Unit
{
    public:
        int vld = 0;
        int dty = 0;
        int tag = 0; 
        int adr = 0;
        int order_counts = 0;

        Unit();

        Unit(int v, int d, int t, int a);

        Unit(int v, int d, int t, int a, int c);

        void Print_Unit();
};