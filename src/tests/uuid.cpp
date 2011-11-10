#include <random>
#include <iostream>
#include <iomanip>

class RandomIterator
{
  public:
  RandomIterator(std::random_device* rand)
  : m_rand(rand), m_count(0)
  {}

  RandomIterator(int count)
  : m_rand(nullptr), m_count(count)
  {}

  private:
  std::random_device* m_rand;

  public:
  decltype((*m_rand)()) operator*()
  {
    return (*m_rand)();
  }

  RandomIterator&
  operator++()
  { 
    ++m_count;
    return *this; 
  }

  bool
  operator==(const RandomIterator& rhs) const
  {
    return m_count == rhs.m_count;
  }

  bool
  operator!=(const RandomIterator& rhs) const
  {
    return !(*this == rhs);
  }

  private:
  int m_count;
};

class uuid
{
  static constexpr size_t static_size() { return 16; }
  public:
  typedef uint8_t* iterator;
  typedef const uint8_t* const_iterator;

  iterator begin() { return data; }
  iterator end() { return data + static_size(); }
  const_iterator begin() const { return data; }
  const_iterator end() const { return data + static_size(); }

  constexpr size_t size() { return static_size(); }

  private:
  //gcc bug?
  //uint8_t data[static_size()];
  uint8_t data[16];
};

template <typename Generator>
void
seed(Generator* generator)
{
  std::random_device rand("/dev/urandom");
  RandomIterator begin(&rand);
  RandomIterator end(101);

  std::seed_seq seq(begin, end);

  generator->seed(seq);
}

template <class Generator>
class uuid_random_generator
{
  typedef typename Generator::result_type unit_type; 
  typedef std::independent_bits_engine
  <
    Generator,
    sizeof(unit_type)*8,
    unit_type
  >full_bits_generator;

  public:
  uuid_random_generator()
  {
    seed(&generator);
  }

  uuid
  operator()()
  {
    uuid u;

    int i = 0;
    unit_type random = generator();
    std::cerr << "random number = " << std::hex << random << std::endl;
    for (auto it = u.begin(); it != u.end(); ++it, ++i)
    {
      if (i == sizeof(unit_type))
      {
        random = generator();
        std::cerr << "random number = " << std::hex << random << std::endl;
        i = 0;
      }

      *it = (random >> (i*8)) & 0xFF;
    }

    return u;
  }

  private:
  full_bits_generator generator;
};

typedef uuid_random_generator<std::mt19937> random_generator;

int main()
{
  random_generator gen;

  for (int i = 0; i != 1000; ++i)
  {
    uuid u = gen();

    std::cout << std::setfill('0');
    for (auto it = u.begin(); it != u.end(); ++it)
    {
      std::cout << std::setw(2) << std::hex << static_cast<size_t>(*it);
    }

    std::cout << std::endl;
  }

  return 0;
}
