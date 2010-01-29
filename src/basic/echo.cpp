#include <tl/interpreter.hpp>
#include <boost/assign/list_of.hpp>
#include <iostream>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>

using namespace TransLucid;
using boost::assign::map_list_of;

class Receiver : public HD {
   public:
   TaggedValue operator()(const Tuple& k);

   void addExpr(const Tuple& k, HD *h);
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
}

void Receiver::addExpr(const Tuple& k, HD *h) {
}

TaggedValue Sender::operator()(const Tuple& k) {
   std::cin.get(m_buf, BUF_SIZE);
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
   return 0;
}
