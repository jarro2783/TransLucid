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

#include <boost/program_options.hpp>
#include <iostream>
#include "tltext.hpp"
#include <fstream>
#include <signal.h>

namespace po = boost::program_options;

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

void
handleSignals(int signal)
{
  switch (signal)
  {
    case SIGSEGV:
    std::cerr << "TLText has encountered a segfault, goodbye..." << std::endl;
    exit(1);
    break;
  }
}

void
setSignals()
{
  struct sigaction action;

  action.sa_handler = &handleSignals;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction(SIGSEGV, &action, nullptr) == -1)
  {
    std::cerr << "Could not set signal handler:";
    perror("sigaction");
  }
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

  setSignals();

  //void (*foo)() = nullptr;
  //(*foo)();

  po::variables_map vm;
  po::options_description desc("tltext options");
  desc.add_options()
    ("args", 
      po::value<std::vector<std::string>>()->multitoken(),
      "arguments to pass to TransLucid in the CLARGS variable")
    ("debug,d", "debug mode")
    ("help,h", "show this message")
    ("input,i", po::value<std::string>(), "input file")
    ("output,o", po::value<std::string>(), "output file")
    ("uuid", "print uuids")
    ("verbose,v", "verbose output")
    ("version", "show version")
  ;

  std::vector<po::basic_option<char>> options;

  try
  {
    po::basic_parsed_options<char> parsed = po::command_line_parser(argc, argv).
      options(desc).allow_unregistered().run();
    po::notify(vm);

    po::store(parsed, vm);

    std::vector<std::string> extra_options = 
      po::collect_unrecognized(parsed.options, po::exclude_positional);


    options = parsed.options;
  }
  catch (std::exception& e)
  {
    std::cerr << "error parsing command line arguments: " << 
      e.what() << std::endl;
    std::cerr << desc << std::endl;

    return -1;
  }

  if (vm.count("help"))
  {
    std::cerr << desc << std::endl;
    return 0;
  }

  if (vm.count("version"))
  {
    std::cerr << "tltext " << PACKAGE_VERSION << std::endl;
    return 0;
  }

  try
  {
    TransLucid::TLText::TLText tltext("TLText...");
 
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
   
    if (vm.count("debug"))
    {
      tltext.debug(true);
    }
    
    if (vm.count("verbose"))
    {
      tltext.verbose(true);
    }

    if (vm.count("uuid"))
    {
      tltext.uuids(true);
    }

    std::unique_ptr<std::ifstream> input;
    if (vm.count("input"))
    {
      input = openInput(vm["input"].as<std::string>());
      tltext.set_input(input.get());
    }

    std::unique_ptr<std::ofstream> output;
    if (vm.count("output"))
    {
      output = openOutput(vm["output"].as<std::string>());
      tltext.set_output(output.get());
    }

    if (vm.count("args"))
    {
      tltext.set_clargs(vm["args"].as<std::vector<std::string>>());
    }
    
    tltext.add_header(PREFIX "/share/tl/tltext/header.tl");

    tltext.run();
  }
  catch (const char* c)
  {
    std::cerr << "terminated with exception: " << c << std::endl;
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
