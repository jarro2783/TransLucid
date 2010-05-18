/* Integer library.
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

#include "libint.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/parameter.hpp>
#include <limits>
#include <gmpxx.h>
#include <boost/ref.hpp>
#include <tl/builtin_types.hpp>
#include <tl/system.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/utility.hpp>
#include <tl/valuehd.hpp>
#include <boost/function.hpp>
#include <tl/utility.hpp>
#include <tl/compiled_functors.hpp>

namespace std
{
  template <class T>
  class plus<IntLib::Int<T>>
  {
    public:
    typedef typename IntLib::Int<T> value_type;
    value_type
    operator()(const value_type& lhs, const value_type& rhs, int)
    {
      return lhs + rhs;
    }
  };
}

namespace IntLib
{

using boost::assign::list_of;
namespace TL = TransLucid;

namespace
{

typedef boost::function
  <
  TL::Constant(const TL::Constant&, const TL::Constant&, const TL::Tuple&)
  > OpFunction;

BOOST_PARAMETER_TEMPLATE_KEYWORD(pre_type);
BOOST_PARAMETER_TEMPLATE_KEYWORD(post_type);

typedef boost::parameter::parameters
<
  boost::optional<tag::pre_type>,
  boost::optional<tag::post_type>
> int_bin_op_signature;

template <class To, class From>
To
convert(const From& f)
{
  return To(f);
}

//convert 64 to mp
template <class From>
mpz_class
convert_64_mp(const From& f)
{
  std::ostringstream os;
  os << f;
  return mpz_class(os.str());
}

template <class To>
To
convert_mp_64(const mpz_class& f)
{
  return boost::lexical_cast<To>(f.get_str());
}

template <>
mpz_class
convert<mpz_class, uint64_t>(const uint64_t& f)
{
  return convert_64_mp(f);
}

template <>
uint64_t
convert<uint64_t, mpz_class>(const mpz_class& f)
{
  return convert_mp_64<uint64_t>(f);
}

template <>
mpz_class
convert<mpz_class, int64_t>(const int64_t& f)
{
  return convert_64_mp(f);
}

template<>
int64_t
convert<int64_t, mpz_class>(const mpz_class& f)
{
  return convert_mp_64<int64_t>(f);
}

namespace ValueCheck
{

struct Null
{
  template <class Actual, class T>
  bool
  operator()(const T& lhs, const T& rhs, const TL::Tuple& k)
  {
    return false;
  }

  template <class Actual, class T>
  bool
  operator()(const T& value, const TL::Tuple& k)
  {
    return false;
  }
};

struct Overflow
{
  template <class Actual, class T>
  bool
  operator()(const T& value, const TL::Tuple& k)
  {
    return value < convert<T>(std::numeric_limits<Actual>::min()) ||
      std::greater<T>()
        (value, convert<T>(std::numeric_limits<Actual>::max()));
  }
};

struct DivByZero
{
  template <class Actual, class T>
  bool
  operator()(const T& lhs, const T& rhs, const TL::Tuple& k)
  {
    return rhs == 0;
  }
};

} //namespace ValueCheck

template <typename T>
struct IntTraits
{
};

template <>
struct IntTraits<Int<uint8_t>>
{
  typedef uint16_t big;
  typedef uint8_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int8_t>>
{
  typedef int16_t big;
  typedef int8_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<uint16_t>>
{
  typedef uint32_t big;
  typedef uint16_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int16_t>>
{
  typedef int32_t big;
  typedef int16_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<uint32_t>>
{
  typedef uint64_t big;
  typedef uint32_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int32_t>>
{
  typedef int64_t big;
  typedef int32_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<uint64_t>>
{
  typedef mpz_class big;
  typedef uint64_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<int64_t>>
{
  typedef mpz_class big;
  typedef int64_t type;
  typedef ValueCheck::Overflow post;
};

template <>
struct IntTraits<Int<mpz_class>>
{
  typedef mpz_class big;
  typedef mpz_class type;
  typedef ValueCheck::Null post;
};

template <>
struct IntTraits<TL::Intmp>
{
  typedef mpz_class big;
  typedef mpz_class type;
  typedef ValueCheck::Null post;
};

template <class T>
struct BigStore
{
  typedef typename IntTraits<T>::big type;
};

template <>
struct BigStore <Int<mpz_class>>
{
  typedef mpz_class& type;
};

template <>
struct BigStore <TL::Intmp>
{
  typedef mpz_class& type;
};

template <class T, template <typename> class Op, class Arg1, class Arg2>
//TL::Constant
//int_bin_op(const T& lhs, const T& rhs, TL::TypeManager& m, int)
//{
TL::Constant int_bin_op
(
  //const std::vector<TL::Constant>& operands,
  size_t index,
  const TL::Constant& l,
  const TL::Constant& r,
  const TL::Tuple& k
)
{

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

  bigstore lhs = convert<bigstore>(l.value<T>().value());
  bigstore rhs = convert<bigstore>(r.value<T>().value());

  if (Pre().template operator()<typename IntTraits<T>::type>(lhs, rhs, k))
  {
    return TL::Constant(TL::Special(U"aritherr"), TL::TYPE_INDEX_SPECIAL);
  }

  typename IntTraits<T>::big value =
    Op<typename IntTraits<T>::big>()(lhs, rhs);

  if (Post().template operator()<typename IntTraits<T>::type>(value, k))
  {
    return TL::Constant(TL::Special(U"aritherr"), TL::TYPE_INDEX_SPECIAL);
  }

  return TL::Constant(T(convert<type>(value)), index);
};

template <class T, template <typename> class Op, class Arg1>
TL::Constant
int_bin_op
(
  //const std::vector<TL::Constant>& operands,
  //const TL::TypeManager& m,
  //int
  size_t index,
  const TL::Constant& l,
  const TL::Constant& r,
  const TL::Tuple& k
)
{
  return int_bin_op<T, Op, Arg1, boost::parameter::void_>(index, l, r, k);
}

template <class T, template <typename> class Op>
TL::Constant
int_bin_op
(
  //const std::vector<TL::Constant>& operands,
  //const TL::TypeManager& m,
  //int
  size_t index,
  const TL::Constant& l,
  const TL::Constant& r,
  const TL::Tuple& k)
{
  return int_bin_op<T, Op, boost::parameter::void_, boost::parameter::void_>
         (index, l, r, k);
}

template <class T, template <typename> class Op>
OpFunction
bindBinOp(TL::HD& i, size_t index)
{
  return boost::bind(int_bin_op<T, Op>, index, _1, _2, _3);
}

template <class T, template <typename> class Op, class Arg1>
OpFunction
bindBinOp(TL::HD& i, size_t index)
{
  //when adding context make the third _2
  return boost::bind(int_bin_op<T, Op, Arg1>, index, _1, _2, _3);
}

template <class T, template <typename> class Op, class Arg1, class Arg2>
OpFunction
bindBinOp(TL::HD& i, size_t index)
{
  //when adding context make the third _2
  return boost::bind(int_bin_op<T, Op, Arg1, Arg2>, index, _1, _2, _3);
}

#if 0
template <class T, template <typename> class Op>
TL::Constant
int_comp_op
(
  const std::vector<TL::Constant>& values,
  const TL::TypeRegistry& r
)
{
  bool result;
  //this is the WOAT
  result = Op<typename IntTraits<T>::type>()(
    values.at(0).value<T>().value(),
    values.at(1).value<T>().value());
  return TL::Constant(TL::Boolean(result), r.indexBoolean());
}
#endif

#if 0
template <class T, template <typename> class Op>
OpFunction
bindCompOp(const TL::TypeManager& m)
{
  return boost::bind(int_comp_op<T, Op>, _1, boost::cref(m.registry()));
}
#endif

template <typename T>
class OpHD : public TL::HD
{
  public:

  OpHD(TL::HD& system, const T& op)
  : m_op(op), m_system(system)
  {}

  //takes the two operands out of the tuple and passes them to the
  //op as arguments
  //variadic templates would be really really good here
  TL::TaggedConstant
  operator()(const TL::Tuple& k)
  {
    try
    {
      size_t index_arg0 = TL::get_dimension_index(&m_system, U"arg0");
      size_t index_arg1 = TL::get_dimension_index(&m_system, U"arg1");

      return TL::TaggedConstant(m_op(TL::get_dimension(k, index_arg0),
                                  TL::get_dimension(k, index_arg1),
                                  k),
                             k);
    }
    catch (TL::DimensionNotFound& e)
    {
      //H @ [id : "CONST", type : "special", value : "dimension"]
      //or we could leave these since it is slightly more efficient,
      // however it will all come out in the wash when we compile anyway
      return TL::TaggedConstant(TL::Constant(TL::Special(
               TL::Special::DIMENSION), TL::TYPE_INDEX_SPECIAL), k);
    }
  }

  private:
  T m_op;
  TL::HD& m_system;
};

template <typename Operator>
TL::HD*
build_op_hd(TL::HD& i, const Operator& op)
{
  return new OpHD<Operator>(i, op);
}

template <typename T, template <typename> class Op>
void
register_one_op
(
  TL::HD& system,
  const TL::u32string& name,
  size_t index
)
{
  TL::HD* op = build_op_hd(system, bindBinOp<T, Op>(system, index));
  //build a guard with arg0:T, arg1:T

  //std::cerr << "adding OP @ [name : " << TL::utf32_to_utf8(name) << "..."
  //<< std::endl;

  TL::tuple_t guard =
  {
    {
      TL::get_dimension_index(&system, U"arg0"),
      TL::Constant(TL::Type(index), TL::TYPE_INDEX_TYPE)
    },
    {
      TL::get_dimension_index(&system, U"arg1"),
      TL::Constant(TL::Type(index), TL::TYPE_INDEX_TYPE)
    },
    {
      TL::DIM_NAME,
      TL::generate_string(name)
    }
  };

  TL::tuple_t context =
  {
    {
      TL::DIM_VALID_GUARD,
      TL::Constant(TL::Guard(TL::GuardHD(TL::Tuple(guard))),
                     TL::TYPE_INDEX_GUARD)
    },
    {
      TL::DIM_ID,
      TL::Constant(TL::String(U"OP"), TL::TYPE_INDEX_USTRING)
    }
  };

  system.addExpr(TL::Tuple(context), op);
}

template<class T>
void
register_int_ops(TL::SystemHD& i, size_t index)
{
  register_one_op<T, std::plus>(i, U"operator+", index);
  register_one_op<T, std::minus>(i, U"operator-", index);
  register_one_op<T, std::multiplies>(i, U"operator*", index);

  #if 0
  r.registerOp("operator+", ops, bindBinOp<T, std::plus>(m));
  r.registerOp("operator-", ops, bindBinOp<T, std::minus>(m));
  r.registerOp("operator*", ops, bindBinOp<T, std::multiplies>(m));
  r.registerOp("operator/", ops,
    bindBinOp<T, std::divides, pre_type<ValueCheck::DivByZero> >(m));
  r.registerOp("operator%", ops,
    bindBinOp<T, std::modulus, pre_type<ValueCheck::DivByZero> >(m));
  r.registerOp("operator<", ops, bindCompOp<T, std::less>(m));
  r.registerOp("operator>", ops, bindCompOp<T, std::greater>(m));
  #endif
}

template <typename T>
class IntHD : public TL::HD
{
  public:
  IntHD(const TL::u32string& name, size_t index, TL::SystemHD& system)
  : m_system(system), m_index(index)
  {
    //m_index = system.typeRegistry().registerType(name);
  }

  TL::TaggedConstant
  operator()(const TL::Tuple& k)
  {
    //retrieve the text and parse it
    size_t tdim = TL::DIM_TEXT;

    TL::Tuple::const_iterator text = k.find(tdim);

    if (text == k.end())
    {
      return TL::TaggedConstant(TL::Constant(TL::Special(
        TL::Special::DIMENSION), TL::TYPE_INDEX_SPECIAL), k);
    }

    try
    {
      return TL::TaggedConstant(TL::Constant(
        Int<T>(boost::lexical_cast<T>(TL::utf32_to_utf8(
          text->second.value<TL::String>().value()))), m_index), k)
         ;
    }
    catch (...)
    {
      return TL::TaggedConstant(TL::Constant(TL::Special(
        TL::Special::CONST), TL::TYPE_INDEX_SPECIAL), k);
    }
  }

  private:
  TL::SystemHD& m_system;
  size_t m_index;
};

template <class T>
void
registerType(const TL::u32string& name, TL::SystemHD& i)
{
  mpz_class unique = TL::get_unique(&i);

  TL::HD* h = new IntHD<T>(name, unique.get_ui(), i);

  TL::tuple_t k;
  k.insert(std::make_pair(TL::DIM_TYPE, TL::generate_string(name)));
  k.insert(std::make_pair(TL::DIM_ID, TL::generate_string(U"CONST")));
  i.addExpr(TransLucid::Tuple(k), h);

  k.clear();
  k.insert(std::make_pair(TL::DIM_TYPE, TL::generate_string(name)));
  k.insert(std::make_pair(TL::DIM_ID, TL::generate_string(U"TYPE_INDEX")));
  i.addExpr
  (
    TransLucid::Tuple(k), 
    new TL::Hyperdatons::IntmpConstHD(unique)
  );

  register_int_ops<Int<T>>(i, unique.get_ui());
}

} //namespace <unnamed>

template <class T>
void
Int<T>::print(std::ostream& os) const
{
  os << "int<" << m_value << ">";
}

void
registerTypes(TransLucid::SystemHD& i)
{
  #if 0
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
  const TL::TypeManager* m = r.findType("intmp");
  //RegisterIntOps<TL::Intmp> intmpops;
  //intmpops(*m);
  #endif

  registerType<uint8_t>(U"uint8", i);
  registerType<int8_t>(U"int8", i);
  registerType<uint16_t>(U"uint16", i);
  registerType<int16_t>(U"int16", i);
  registerType<uint32_t>(U"uint32", i);
  registerType<int32_t>(U"int32", i);
  registerType<uint64_t>(U"uint64", i);
  registerType<int64_t>(U"int64", i);
  register_int_ops<TL::Intmp>(i, TL::TYPE_INDEX_INTMP);
}

} //namespace IntLib

extern "C"
{

void
lib_int_init(TransLucid::SystemHD& i)
{
  IntLib::registerTypes(i);
}

}
