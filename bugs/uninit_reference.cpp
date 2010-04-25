#define TEST 5

class Outer 
{
  public:
  Outer()
  : i(*this)
  {
  }

  class Inner 
  {
    public:
    Inner(Outer& o)
    : o(o)
    {
    }

    private:
    Outer& o;
  };

  private:
  Inner i;
};

class A {
  Outer o;
};

int main()
{
  A *a = new A;
  int x = TEST;

  return 0;
}
