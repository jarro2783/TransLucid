#ifndef LIBRARY_HPP_INCLUDED
#define LIBRARY_HPP_INCLUDED

#include <tl/types.hpp>
#include <ltdl.h>
#include <list>
#include <tl/hyperdaton.hpp>

namespace TransLucid
{

  typedef void (*library_loader)(HD*);

  class Libtool
  {
    public:
    Libtool();

    ~Libtool();

    void
    loadLibrary(const u32string& name, HD* system);

    void
    addSearchPath(const u32string& path);

    private:
    std::list<u32string> m_searchDirs;
  };
} //namespace TransLucid

#endif // LIBRARY_HPP_INCLUDED
