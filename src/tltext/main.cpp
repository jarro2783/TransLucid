/* Core TransLucid application main file.
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

  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
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
    TransLucid::TLText::TLText tltext;
    
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

    tltext.run();
  }
  catch (const char* c)
  {
    std::cerr << "terminated with exception: " << c << std::endl;
    return 1;
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
