#include<iostream>

using namespace std;

class Unit
{
    public:
        int valid = 0;
        int dirty = 0; 
        int tag = 0;
        int addr = 0;
        inr lru_count = 0;


        Unit(int val, int dir, int tag, int adr, int luc);
};
