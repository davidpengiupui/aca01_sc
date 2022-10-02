#include"Unit.cpp"

Unit::Unit(int val, int dir, int tat, int adr, int luc): valid(val), dirty(dir), tag(tat), addr(adr), lru_count(luc)
{
  
}

void Unit::Print_Unit()
{
    cout << "valid: " << valid << endl;
    cout << "dirty: " << dirty << endl;
    cout << "tag: " << tag << endl;
    cout << "addr: " << addr << endl;
    cout << "lru_count: " << lru_count << endl;
}
