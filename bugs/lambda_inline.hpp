inline int f(int x) 
{
  auto g = [] (int x)
  {
     return x+1;
  };
  return g(x);
}

void h();
