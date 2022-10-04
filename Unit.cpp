#include "../include/Unit.h"

Unit::Unit(): vld(0), dty(0) {}

Unit::Unit(int v, int d, int t, int a): vld(v), dty(d), tag(t), adr(a) {}

Unit::Unit(int v, int d, int t, int a, int c): vld(v), dty(d), tag(t), adr(a), order_counts(c) {}

void Unit::Print_Unit()
{
    cout << "valid: " << vld << endl;
    cout << "dirty: " << dty << endl;
    cout << "tag: " << tag << endl;
    cout << "addr: " << adr << endl;
    cout << "lru_count: " << order_counts << endl;
}

