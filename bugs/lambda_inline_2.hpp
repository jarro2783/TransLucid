
inline int f(int x) 
{
  return ([] (int x) {return x+1;}) (x);
}

void h();
