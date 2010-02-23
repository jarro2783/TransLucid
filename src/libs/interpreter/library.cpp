#include <tl/library.hpp>
#include <ltdl.h>
#include <iostream>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace
{
  bool
  attemptLibraryOpen
  (
    const u32string& file,
    const u32string& name,
    HD* system
  )
  {
    #warning fix library loading
    bool success = true;
    std::string filename = utf32_to_utf8(file);

    std::cout << "opening " << filename << std::endl;

    lt_dlhandle h = lt_dlopenext(filename.c_str());

    if (h != 0)
    {
      void* init = lt_dlsym(
        h,
        utf32_to_utf8(U"lib_" + name + U"_init").c_str()
      );

      if (init != 0)
      {
        (*(library_loader)init)(system);
      }
      else
      {
        std::cerr << lt_dlerror() << std::endl;
        success = false;
      }
    }
    else
    {
      std::cerr << lt_dlerror() << std::endl;
      success = false;
    }

#if 0
    if (!success)
    {
      std::cerr << "error opening " << name << ":" << std::endl;
      const char* error = lt_dlerror();
      if (error)
      {
        std::cerr << error << std::endl;
      }
    }
#endif

    return success;
  }
} //namespace anonymous

Libtool::Libtool()
{
  lt_dlinit();
  lt_dladdsearchdir(".");
  //m_searchDirs.push_back(U".");
  #ifdef PKGLIBDIR
  //m_searchDirs.push_back(U PKGLIBDIR);
  #endif
}

Libtool::~Libtool()
{
  lt_dlexit();
}

void
Libtool::loadLibrary(const u32string& name, HD* system)
{

  #if 0
  bool success = false;

  std::list<u32string>::const_iterator iter = m_searchDirs.begin();
  while (!success && iter != m_searchDirs.end())
  {
    std::cerr << "attempting to open " << utf32_to_utf8(*iter + U"/lib" + name) << std::endl;
    if (attemptLibraryOpen(*iter + U"/lib" + name, name, system))
    {
      success = true;
    }
    ++iter;
  }

  if (!success)
  {
    //std::cerr << "warning: unable to open library " << name << std::endl;
  }
  #endif
  char wd[1000];
  getcwd(wd, 1000);
  std::cout << "working directory: " << wd << std::endl;
  attemptLibraryOpen(U"lib" + name, name, system);
}

void
Libtool::addSearchPath(const u32string& path)
{
  m_searchDirs.push_back(path);
}

} //namespace TransLucid
