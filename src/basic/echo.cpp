#include <tl/interpreter.hpp>
#include <boost/assign/list_of.hpp>
#include <iostream>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/compiled_functors.hpp>
#include <tl/utility.hpp>

using namespace TransLucid;
using boost::assign::map_list_of;

class Receiver : public HD {
   public:
   TaggedValue operator()(const Tuple& k);

   void addExpr(const Tuple& k, HD *h);

   private:
   TaggedValue m_value;
};

class Sender : public HD {
   public:
   TaggedValue operator()(const Tuple& k);

   void addExpr(const Tuple& k, HD *h);

   private:
   static const int BUF_SIZE = 1000;
   char m_buf[BUF_SIZE];
};

TaggedValue Receiver::operator()(const Tuple& k) {
   return m_value;
}

void Receiver::addExpr(const Tuple& k, HD *h) {
   Tuple::const_iterator iter = k.find(DIM_VALUE);
   m_value = TaggedValue(iter->second, k);
}

TaggedValue Sender::operator()(const Tuple& k) {
   std::cin.getline(m_buf, BUF_SIZE);
   return TaggedValue(TypedValue(String(m_buf), TYPE_INDEX_USTRING), k);
}

void Sender::addExpr(const Tuple& k, HD *h) {
}

int main(int argc, char *argv[]) {
   Interpreter i;

   Receiver r;
   Sender s;

   i.addOutput(map_list_of("out", &r));
   i.addInput(map_list_of("keyboard", &s));

   i.addDemand("out", EquationGuard());

   //set out = keyboard
   CompiledFunctors::Ident ident(&i, "keyboard");
   tuple_t context = map_list_of(size_t(DIM_ID), generate_string("out"))
   (get_dimension_index(&i, "_validguard"), TypedValue(EquationGuardType(EquationGuard()), TYPE_INDEX_GUARD));
   i.addExpr(Tuple(context), &ident);

   while (true) {
      i.tick();
      TaggedValue result = r(Tuple());
      std::cout << result.first.value<String>().value() << std::endl;
   }

   return 0;
}
