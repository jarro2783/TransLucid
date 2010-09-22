/* Library loading.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

#include <tl/charset.hpp>
#include <tl/library.hpp>
#include <tl/utility.hpp>

#include <iostream>

#include <ltdl.h>

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
  bool result = attemptLibraryOpen(U"lib" + name, name, system);

  if (!result)
  {
    throw utf32_to_utf8(U"unable to open library: " + name);
  }
}

void
Libtool::addSearchPath(const u32string& path)
{
  m_searchDirs.push_back(path);
}

} //namespace TransLucid
