#include <tl/interpreter.hpp>
#include <iostream>
#include <ltdl.h>
#include <boost/assign.hpp>
#include <boost/program_options.hpp>
#include <glibmm/miscutils.h>
#include "directory_system.hpp"
#include <tl/fixed_indexes.hpp>
#include <tl/utility.hpp>

namespace TL = TransLucid;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
   using boost::assign::list_of;

   setlocale(LC_ALL, "");

   std::string input;

   po::options_description desc("tl options");
   desc.add_options()
      ("help,h", "show this message")
      ("input,i", po::value<std::string>(), "input directory")
      ("verbose,v", "verbose output")
      ("library-path", po::value<std::vector<std::string> >(), "add a library search path")
   ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);

   if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
   }

   if (vm.count("input")) {
      input = vm["input"].as<std::string>();
   } else {
      std::cerr << "an input directory must be specified" << std::endl;
      return 1;
   }

   TL::DirectoryParser::DirectorySystem interpreter;
   TL::Tuple context;

   if (vm.count("verbose")) {
      interpreter.verbose();
      std::clog << "running in source directory: " <<
      Glib::get_current_dir() + "/" + input << std::endl;
   }

   if (vm.count("library-path")) {
      std::vector<std::string> paths =
         vm["library-path"].as<std::vector<std::string> >();
      BOOST_FOREACH(const std::string& s, paths) {
         interpreter.addLibrarySearchPath(s);
      }
   }

   typedef std::pair<TL::TypedValue, TL::Tuple> ValueContextPair;

   std::vector<ValueContextPair> evaluated;

   bool evaluate = true;
   try {
      if (!interpreter.parseSystem(input)) {
         std::cerr << interpreter.errorCount() << " errors parsing input: "
         "demands not evaluated" << std::endl;
         evaluate = false;
      }
   } catch (const char *c) {
      std::cerr << "exception parsing system: " << c << std::endl;
   }

   if (evaluate) {
      interpreter.evaluateSystem(std::back_inserter(evaluated));

      //TL::TypeRegistry& registry = interpreter.typeRegistry();

      BOOST_FOREACH(ValueContextPair& p, evaluated) {
         //const TL::TypeManager* m = registry.findType(p.first.index());
         //m->print(std::cout, p.first, p.second);
         TL::tuple_t k;
         k[TL::DIM_ID] = TL::generate_string("PRINT");
         k[TL::DIM_VALUE] = p.first;
         TL::TypedValue s = interpreter(TL::Tuple(k)).first;
         if (s.index() != TL::TYPE_INDEX_USTRING) {
            std::cout << "oops";
         } else {
            std::cout << s.value<TL::String>().value();
         }
         std::cout << std::endl;
      }
   }

   return evaluate ? 0 : 1;
}
