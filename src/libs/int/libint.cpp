#include "libint.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/parameter.hpp>
#include <limits>
#include <gmpxx.h>
#include <boost/ref.hpp>
#include <tl/builtin_types.hpp>
#include <tl/interpreter.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/utility.hpp>
#include <tl/consthd.hpp>

namespace std {
   template <class T>
   class plus<IntLib::Int<T> > {
      public:
      typedef typename IntLib::Int<T> value_type;
      value_type operator()(const value_type& lhs, const value_type& rhs, int) {
         return lhs + rhs;
      }
   };
}

namespace IntLib {

using boost::assign::list_of;
namespace TL = TransLucid;

namespace {

BOOST_PARAMETER_TEMPLATE_KEYWORD(pre_type);
BOOST_PARAMETER_TEMPLATE_KEYWORD(post_type);

typedef boost::parameter::parameters <
   boost::optional<tag::pre_type>,
   boost::optional<tag::post_type>
> int_bin_op_signature;

template <class To, class From>
To convert(const From& f) {
   return To(f);
}

//convert 64 to mp
template <class From>
mpz_class convert_64_mp(const From& f) {
   std::ostringstream os;
   os << f;
   return mpz_class(os.str());
}

template <class To>
To convert_mp_64(const mpz_class& f) {
   return boost::lexical_cast<To>(f.get_str());
}

template <>
mpz_class convert<mpz_class, uint64_t>(const uint64_t& f) {
   return convert_64_mp(f);
}

template <>
uint64_t convert<uint64_t, mpz_class>(const mpz_class& f) {
   return convert_mp_64<uint64_t>(f);
}

template <>
mpz_class convert<mpz_class, int64_t>(const int64_t& f) {
   return convert_64_mp(f);
}

template<>
int64_t convert<int64_t, mpz_class>(const mpz_class& f) {
   return convert_mp_64<int64_t>(f);
}

namespace ValueCheck {

struct Null {
   template <class Actual, class T>
   bool operator()(const T& lhs, const T& rhs, int) {
      return false;
   }

   template <class Actual, class T>
   bool operator()(const T& value, int) {
      return false;
   }
};

struct Overflow {
   template <class Actual, class T>
   bool operator()(const T& value, int) {
      return value < convert<T>(std::numeric_limits<Actual>::min()) ||
         std::greater<T>()(value, convert<T>(std::numeric_limits<Actual>::max()));
   }
};

struct DivByZero {
   template <class Actual, class T>
   bool operator()(const T& lhs, const T& rhs, int) {
      return rhs == 0;
   }
};

}

template <typename T>
struct IntTraits {
};

template <>
struct IntTraits<Int<uint8_t> > {
   typedef uint16_t big;
   typedef uint8_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int8_t> > {
   typedef int16_t big;
   typedef int8_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<uint16_t> > {
   typedef uint32_t big;
   typedef uint16_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int16_t> > {
   typedef int32_t big;
   typedef int16_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<uint32_t> > {
   typedef uint64_t big;
   typedef uint32_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int32_t> > {
   typedef int64_t big;
   typedef int32_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<uint64_t> > {
   typedef mpz_class big;
   typedef uint64_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int64_t> > {
   typedef mpz_class big;
   typedef int64_t type;
   typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<mpz_class> > {
   typedef mpz_class big;
   typedef mpz_class type;
   typedef ValueCheck::Null post;
};

template <>
struct IntTraits<TL::Intmp> {
   typedef mpz_class big;
   typedef mpz_class type;
   typedef ValueCheck::Null post;
};

template <class T>
struct BigStore {
   typedef typename IntTraits<T>::big type;
};

template <>
struct BigStore <Int<mpz_class> > {
   typedef mpz_class& type;
};

template <>
struct BigStore <TL::Intmp> {
   typedef mpz_class& type;
};

template <class T, template <typename> class Op, class Arg1, class Arg2>
//TL::TypedValue int_bin_op(const T& lhs, const T& rhs, TL::TypeManager& m, int) {
TL::TypedValue int_bin_op(const std::vector<TL::TypedValue>& operands, const TL::TypeManager& m, int) {

   typedef typename
      int_bin_op_signature::bind<Arg1, Arg2>::type
   args;

   typedef typename boost::parameter::binding<
      args, tag::pre_type, ValueCheck::Null>::type Pre;

   typedef typename boost::parameter::binding<
      args, tag::post_type, typename IntTraits<T>::post>::type Post;

   typedef typename IntTraits<T>::big big;
   typedef typename IntTraits<T>::type type;

   typedef typename BigStore<T>::type bigstore;

   bigstore lhs = convert<bigstore>(operands.at(0).value<T>().value());
   bigstore rhs = convert<bigstore>(operands.at(1).value<T>().value());

   if (Pre().template operator()<typename IntTraits<T>::type>(lhs, rhs, 0)) {
      return TL::TypedValue(TL::Special("aritherr"), m.registry().indexSpecial());
   }

   typename IntTraits<T>::big value = Op<typename IntTraits<T>::big>()(lhs, rhs);

   if (Post().template operator()<typename IntTraits<T>::type>(value, 0)) {
      return TL::TypedValue(TL::Special("aritherr"), m.registry().indexSpecial());
   }

   return TL::TypedValue(T(convert<type>(value)), m.index());
};

template <class T, template <typename> class Op, class Arg1>
TL::TypedValue int_bin_op(const std::vector<TL::TypedValue>& operands, const TL::TypeManager& m, int) {
   return int_bin_op<T, Op, Arg1, boost::parameter::void_>(operands, m, 0);
}

template <class T, template <typename> class Op>
TL::TypedValue int_bin_op(const std::vector<TL::TypedValue>& operands, const TL::TypeManager& m, int) {
   return int_bin_op<T, Op, boost::parameter::void_, boost::parameter::void_>(operands, m, 0);
}

template <class T, template <typename> class Op>
TL::OpFunction bindBinOp(const TL::TypeManager& m) {
   //when adding context make the third _2
   return boost::bind(int_bin_op<T, Op>, _1, boost::ref(m), 0);
}

template <class T, template <typename> class Op, class Arg1>
TL::OpFunction bindBinOp(const TL::TypeManager& m) {
   //when adding context make the third _2
   return boost::bind(int_bin_op<T, Op, Arg1>, _1, boost::ref(m), 0);
}

template <class T, template <typename> class Op, class Arg1, class Arg2>
TL::OpFunction bindBinOp(const TL::TypeManager& m) {
   //when adding context make the third _2
   return boost::bind(int_bin_op<T, Op, Arg1, Arg2>, _1, boost::ref(m), 0);
}

template <class T, template <typename> class Op>
TL::TypedValue int_comp_op(const std::vector<TL::TypedValue>& values, const TL::TypeRegistry& r) {
   bool result;
   //this is the WOAT
   result = Op<typename IntTraits<T>::type>()(
      values.at(0).value<T>().value(),
      values.at(1).value<T>().value());
   return TL::TypedValue(TL::Boolean(result), r.indexBoolean());
}

template <class T, template <typename> class Op>
TL::OpFunction bindCompOp(const TL::TypeManager& m) {
   return boost::bind(int_comp_op<T, Op>, _1, boost::cref(m.registry()));
}

template <typename T>
class OpHD : public TL::HD {
   public:

   OpHD(TL::Interpreter& system)
   : m_system(system)
   {}

   //takes the two operands out of the tuple and passes them to the
   //op as arguments
   //variadic templates would be really really good here
   TL::TaggedValue operator()(const TL::Tuple& k) {
      try {
         size_t index_arg0 = TL::get_dimension_index(&m_system, "arg0");
         size_t index_arg1 = TL::get_dimension_index(&m_system, "arg1");

         return TL::TaggedValue(m_op(TL::get_dimension(k, index_arg0),
                                     TL::get_dimension(k, index_arg1),
                                     k));
      } catch (TL::DimensionNotFound& e) {
         #warning maybe use the system to build these specials
         //H @ [id : "CONST", type : "special", value : "dimension"]
         //or we could leave these since it is slightly more efficient, however it will
         //all come out in the wash when we compile anyway
         return TL::TaggedValue(TL::TypedValue(TL::Special(TL::Special::DIMENSION), TL::TYPE_INDEX_SPECIAL), k);
      }
   }

   void addExpr(const TL::Tuple& k, HD *h) {
   }

   private:
   T m_op;
   TL::Interpreter& m_system;
};

template<class T>
class RegisterIntOps {
   public:
   void operator()(const TL::TypeManager& m) {
      std::vector<TL::type_index> ops = list_of(m.index())(m.index());
      TL::TypeRegistry& r = m.registry();
      //r.registerOp("operator+", ops, TL::BinaryOp<T, int_bin_op<T, std::plus> >(m));
      #if 0
      r.registerOp("operator+", ops, bindBinOp<T, std::plus>(m));
      r.registerOp("operator-", ops, bindBinOp<T, std::minus>(m));
      r.registerOp("operator*", ops, bindBinOp<T, std::multiplies>(m));
      r.registerOp("operator/", ops, bindBinOp<T, std::divides, pre_type<ValueCheck::DivByZero> >(m));
      r.registerOp("operator%", ops, bindBinOp<T, std::modulus, pre_type<ValueCheck::DivByZero> >(m));
      r.registerOp("operator<", ops, bindCompOp<T, std::less>(m));
      r.registerOp("operator>", ops, bindCompOp<T, std::greater>(m));
      #endif
   }
};

template <typename T>
class IntHD : public TL::HD {
   public:
   IntHD(const TL::ustring_t& name, TL::Interpreter& system)
   : m_system(system)
   {
      m_index = system.typeRegistry().registerType(name);
   }

   TL::TaggedValue operator()(const TL::Tuple& k) {
      //retrieve the text and parse it
      size_t tdim = m_system.dimTranslator().lookup("text");

      TL::Tuple::const_iterator text = k.find(tdim);

      if (text == k.end()) {
         return TL::TaggedValue(TL::TypedValue(
            TL::Special(TL::Special::DIMENSION), m_system.typeRegistry().indexSpecial()), k);
      }

      try {
         return TL::TaggedValue(TL::TypedValue(
            Int<T>(boost::lexical_cast<T>(text->second.value<TL::String>().value())), m_index), k);
      } catch (...) {
         return TL::TaggedValue(TL::TypedValue(
            TL::Special(TL::Special::CONST), m_system.typeRegistry().indexSpecial()), k);
      }
   }

   void addExpr(const TL::Tuple& k, HD *h) {
   }

   private:
   TL::Interpreter& m_system;
   size_t m_index;
};

template <class T>
void makeTypeManager(const TL::ustring_t& name, TL::TypeRegistry& r) {
   new TL::TemplateTypeManager<Int<T> >(r, name, RegisterIntOps<Int<T> >());
}

template <class T>
void registerType(const TL::ustring_t& name, TL::Interpreter& i) {
   TL::HD *h = new IntHD<T>(name, i);

   TL::tuple_t k;
   k.insert(std::make_pair(TL::DIM_TYPE, TL::generate_string(name)));
   k.insert(std::make_pair(TL::DIM_ID, TL::generate_string("CONST")));
   i.addExpr(TransLucid::Tuple(k), h);
   mpz_class unique = TL::get_unique(&i);

   k.clear();
   k.insert(std::make_pair(TL::DIM_TYPE, TL::generate_string(name)));
   k.insert(std::make_pair(TL::DIM_ID, TL::generate_string("TYPEINDEX")));
   i.addExpr(TransLucid::Tuple(k), new TL::ConstHD::IntmpConst(i, unique));
}

}

template <class T>
void Int<T>::print(std::ostream& os, const TL::Tuple& context) const {
   os << m_value;
}

void registerTypes(TransLucid::Interpreter& i) {
   TransLucid::TypeRegistry& r = i.typeRegistry();
   makeTypeManager<uint8_t>("uint8", r);
   makeTypeManager<int8_t>("int8", r);
   makeTypeManager<uint16_t>("uint16", r);
   makeTypeManager<int16_t>("int16", r);
   makeTypeManager<uint32_t>("uint32", r);
   makeTypeManager<int32_t>("int32", r);
   makeTypeManager<uint64_t>("uint64", r);
   makeTypeManager<int64_t>("int64", r);
   //makeTypeManager<mpz_class>("intmp", r);
   const TL::TypeManager *m = r.findType("intmp");
   RegisterIntOps<TL::Intmp> intmpops;
   intmpops(*m);

   registerType<uint8_t>("uint8", i);
}

} //namespace IntLib

extern "C" {

void lib_int_init(TransLucid::Interpreter& i) {
   IntLib::registerTypes(i);
}

}
