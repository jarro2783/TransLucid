
#if 0
#include "interactive.hpp"
#include <tl/interpreter.hpp>
#include <boost/program_options.hpp>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace po = boost::program_options;
#endif

int
main(int argc, char *argv[])
{
  #if 0
  setlocale(LC_ALL, "");

  TLInteractive::System system;

  po::options_description desc("tl options");
  desc.add_options()
    ("header,h",
     po::value<std::string>(),
     "add external header for parsing")
    ("library-path",
     po::value<std::vector<std::string>>(),
     "add a library search path")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("header"))
  {
    std::cout << "parsing header “"
              << vm["header"].as<std::string>()
              << "”"
              << std::endl;
    system.parseHeader(vm["header"].as<std::string>());
  }

  std::cout << "TransLucid " PACKAGE_VERSION << std::endl;

  system.run();
  #endif
  #warning fix interactive
  return 0;
}
