#include <random>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unordered_set>

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
  static const size_t static_size = 16;
  public:
  typedef uint8_t* iterator;
  typedef const uint8_t* const_iterator;

  iterator begin() { return data; }
  iterator end() { return data + static_size; }
  const_iterator begin() const { return data; }
  const_iterator end() const { return data + static_size; }

  constexpr size_t size() const { return static_size; }

  private:
  //gcc bug?
  uint8_t data[static_size];
};

inline bool operator==(const uuid& lhs, const uuid& rhs)
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

inline bool operator<(const uuid& lhs, const uuid& rhs)
{
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), 
    rhs.end());
}

inline bool operator!=(const uuid& lhs, const uuid& rhs)
{
  return !(lhs == rhs);
}

inline std::size_t hash_value(uuid const& u) /* throw() */
{
    std::size_t seed = 0;
    for(uuid::const_iterator i=u.begin(); i != u.end(); ++i)
    {
        seed ^= static_cast<std::size_t>(*i) + 
          0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

namespace std
{
  template <>
  struct hash<uuid>
  {
    size_t operator()(const uuid& u) const
    {
      return hash_value(u);
    }
  };
}

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
    for (auto it = u.begin(); it != u.end(); ++it, ++i)
    {
      if (i == sizeof(unit_type))
      {
        random = generator();
        i = 0;
      }

      *it = (random >> (i*8)) & 0xFF;
    }

    return u;
  }

  private:
  full_bits_generator generator;
};

std::ostream&
operator<<(std::ostream& os, const uuid& id)
{
  os << std::setfill('0');
  for (auto it = id.begin(); it != id.end(); ++it)
  {
    os << std::setw(2) << std::hex << static_cast<size_t>(*it);
  }
  return os;
}

typedef uuid_random_generator<std::mt19937> random_generator;

int collision_test(int n)
{
  random_generator gen;

  int tick = n / 100;

  if (tick == 0)
  {
    tick = 1;
  }

  std::unordered_set<uuid> uuid_set;

  int collisions = 0;

  std::unordered_set<uuid> collision_set;

  for (int i = 0; i != n; ++i)
  {
    if (i % tick == 0)
    {
      std::cout << "." << std::flush;
    }

    uuid u = gen();

    if (!uuid_set.insert(u).second)
    {
      std::cout << "x" << std::endl;
      ++collisions;
      collision_set.insert(u);
    }

  }

  std::cout << std::endl;

  if (collisions == 0)
  {
    std::cout << "there were no collisions" << std::endl;
  }
  else if (collisions == 1)
  {
    std::cout << "there was one collision" << std::endl;
  }
  else if (collisions >= 2)
  {
    std::cout << "there were " << collisions << "collisions" << std::endl;
  }

  for (auto id : collision_set)
  {
    std::cout << id << std::endl;
  }

  return collisions == 0 ? 0 : 1;
}

void
generate(int n)
{
  random_generator gen;
  for (int i = 0; i != n; ++i)
  {
    std::cout << gen() << std::endl;
  }
}

void
usage(char* prog)
{
  std::cout << "TransLucid uuid tester" << std::endl << std::endl;
  std::cout << prog << "[options]" << std::endl << std::endl;
  std::cout << "Program options:" << std::endl;
  std::cout <<
  "  --collision-test N         run a collision test with N uuids generated\n"
  "  --generate N               output N uuids to stdout\n"
  ;
}

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    usage(argv[0]);
    exit(1);
  }

  if (argv[1] == std::string("--collision-test"))
  {
    return collision_test(std::atoi(argv[2]));
  }
  else if (argv[1] == std::string("--generate"))
  {
    generate(std::atoi(argv[2]));
  }
  else
  {
    usage(argv[0]);
    exit(1);
  }

  return 0;
}
