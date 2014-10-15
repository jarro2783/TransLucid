/* tltext main file.
   Copyright (C) 2011 Jarryd Beck

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

/**
 * \file src/tltext/main.cpp
 * The main tltext driver file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include "tltext.hpp"
#include <fstream>
#include <signal.h>

#include <tl/output.hpp>

#include "gettext.h"
#include "cxxopts.hpp"

#define _(String) gettext (String)

namespace
{

std::unique_ptr<std::ifstream> openInput(const std::string& input)
{
  std::unique_ptr<std::ifstream> is(new std::ifstream(input.c_str()));
  if (is->fail())
  {
    throw "Could not open file";
  }
  return is;
}

std::unique_ptr<std::ofstream> openOutput(const std::string& output)
{
  std::unique_ptr<std::ofstream> os(new std::ofstream(output.c_str()));
  if (os->fail())
  {
    throw "Unable to open file";
  }
  return os;
}

std::unique_ptr<char[]> altstack;

void
handleSignals(int signal)
{
  switch (signal)
  {
    case SIGSEGV:
    std::cerr << _("TLtext has encountered a segfault, goodbye...") 
              << std::endl;
    exit(1);
    break;
  }
}

void
setSignals()
{
  //set up an alternate stack because when we have a stack overflow we
  //overflow the stack trying to handle the stack overflow

  //ask for 512kB
  altstack.reset(new char[512*1024]);

  stack_t ss;

  ss.ss_sp = altstack.get();
  ss.ss_size = 512*1024;
  ss.ss_flags = 0;

  if (sigaltstack(&ss, nullptr) == -1)
  {
    std::cerr << "error: could not allocate alternate signal stack" 
              << std::endl;
    perror("sigaltstack: ");
    exit(1);
  }

  struct sigaction action;

  action.sa_handler = &handleSignals;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_ONSTACK;

  if (sigaction(SIGSEGV, &action, nullptr) == -1)
  {
    std::cerr << "Could not set signal handler:";
    perror("sigaction");
  }
}

void
init_gettext()
{
  bindtextdomain(TRANSLATE_DOMAIN, LOCALEDIR);
  textdomain(TRANSLATE_DOMAIN);
}

}

/**
 * The main function. tltext starts here.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return The program exit code.
 */
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  init_gettext();

  setSignals();

  //void (*foo)() = nullptr;
  //(*foo)();

  cxxopts::Options options(argv[0]);
  options.add_options()
    ("args", 
      /* TRANSLATORS: the help message for --args */
      _("arguments to pass to TransLucid in the CLARGS variable"),
      cxxopts::value<std::vector<std::string>>()
      )
    /* TRANSLATORS: the help message for --cache */
    ("cache", _("use cache, no testing is done to check if this is valid"))
    /* TRANSLATORS: the help message for --debug */
    ("d,debug", _("debug mode"))
    /* TRANSLATORS: the help message for --deps */
    ("deps", _("compute dependencies"))
    /* TRANSLATORS: the help message for --help */
    ("h,help", _("show this message"))
    /* TRANSLATORS: the help message for --no-builtin-header */
    ("no-builtin-header", _("don't use the standard header"))
    /* TRANSLATORS: the help message for --header */
    ("header", _("load another header"), 
      cxxopts::value<std::vector<std::string>>())
    /* TRANSLATORS: the help message for --input */
    ("i,input", _("input file"), cxxopts::value<std::string>())
    /* TRANSLATORS: the help message for --output */
    ("o,output", _("output file"), cxxopts::value<std::string>())
    /* TRANSLATORS: the help message for --tyinf */
    ("tyinf", _("enable type inference"))
    /* TRANSLATORS: the help message for --full-types */
    ("fulltypes", _("print full non-display types"))
    /* TRANSLATORS: the help message for --uuid */
    ("uuid", _("print uuids"))
    /* TRANSLATORS: the help message for --verbose */
    ("v,verbose", _("level of verbosity"), cxxopts::value<int>())
    /* TRANSLATORS: the help message for --version */
    ("version", _("show version"))
  ;

  try
  {
    options.parse(argc, argv);
  }
  catch (std::exception& e)
  {
    std::cerr << "error parsing command line arguments: " << 
      e.what() << std::endl;
    std::cerr << options.help() << std::endl;

    return -1;
  }

  if (options.count("help"))
  {
    std::cerr << options.help() << std::endl;
    return 0;
  }

  if (options.count("version"))
  {
    std::cerr << "tltext " << PACKAGE_VERSION << std::endl;
    return 0;
  }

  try
  {
    bool tyinf = false;
    bool cached = false;

    if (options.count("cache"))
    {
      cached = true;
    }

    if (options.count("tyinf"))
    {
      tyinf = true;
    }

    TransLucid::TLText::TLText tltext(argv[0], "TLtext...", cached, tyinf);
 
    //TODO fix this and sort out CLARGS
    #if 0
    for (const auto& s : options)
    {
      if (s.unregistered)
      {
        if (s.value.size() == 1)
        {
          tltext.add_argument(TransLucid::utf8_to_utf32(s.string_key),
            TransLucid::utf8_to_utf32(s.value.at(0)));
        }
        else
        {
          //if it doesn't have one thing, then it must have zero
          tltext.add_argument(TransLucid::utf8_to_utf32(s.string_key));
        }
      }
    }
    #endif

    if (options.count("verbose"))
    {
      tltext.verbose(options["verbose"].as<int>());
    }

    if (options.count("debug"))
    {
      tltext.debug();
    }

    if (options.count("fulltypes"))
    {
      tltext.print_full_types(true);
    }

    if (options.count("uuid"))
    {
      tltext.uuids(true);
    }

    if (options.count("deps"))
    {
      tltext.compute_deps();
    }

    std::unique_ptr<std::ifstream> input;
    if (options.count("input"))
    {
      std::string inputName = options["input"].as<std::string>();
      input = openInput(inputName);
      tltext.set_input(input.get(), inputName);
    }

    std::unique_ptr<std::ofstream> output;
    if (options.count("output"))
    {
      output = openOutput(options["output"].as<std::string>());
      tltext.set_output(output.get());
    }

    if (options.count("args"))
    {
      tltext.set_clargs(options["args"].as<std::vector<std::string>>());
    }
    
    if (options.count("no-builtin-header") == 0)
    {
      if (tyinf)
      {
        tltext.add_header(PREFIX "/share/tl/tltext/header-tyinf.tl");
      }
      else
      {
        tltext.add_header(PREFIX "/share/tl/tltext/header.tl");
      }
    }

    if (options.count("header") > 0)
    {
      std::vector<std::string> headers = options["header"].
        as<std::vector<std::string>>();

      for (const auto& header : headers)
      {
        tltext.add_header(header);
      }
    }

    tltext.run();
  }
  catch (const char* c)
  {
    std::cerr << "terminated with exception: " << c << std::endl;
    return 1;
  }
  catch (const TransLucid::u32string& error)
  {
    std::cerr << "terminated with exception: " << error << std::endl;
    return 1;
  }
  catch (TransLucid::TLText::ReturnError& ret)
  {
    return ret.m_code;
  }
  catch (std::exception& e)
  {
    std::cerr << "std::exception running system: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "error running system" << std::endl;
    return 1;
  }

  return 0;
}
