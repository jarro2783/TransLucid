/* OpWS class.
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

#include <tl/workshop.hpp>
#include <sstream>

/**
 * @file ophd.hpp
 * Operation hyperdaton classes.
 */

namespace TransLucid
{
  namespace OpWSImp
  {
    /**
     * Transforms a list of something into a list of something else
     * defined by Mod. Its type is a type of List of Args transformed to
     * the type decided by Mod.
     */
    template 
    <
      template <typename...> class List,
      template<type_index> class Mod,
      type_index ...Args
    >
    struct modify_type_index_list
    {
      /**
       * The type of the list.
       */
      typedef List<typename Mod<Args>::type...> type;
    };

    template <type_index T>
    struct make_size_t
    {
      /**
       * The type of make_size_t, just size_t.
       */
      typedef size_t type;
    };

    /**
     * Creates a tuple with length of Args integers as parameters.
     */
    template <type_index ...Args>
    struct generate_args_tuple_type
    {
      /**
       * The type of the list generated.
       */
      typedef typename modify_type_index_list
        <std::tuple, make_size_t, Args...>::type type;
    };

    template 
    <
      int N, 
      typename Ret,
      template <typename...> class List,
      type_index ...Args
    >
    struct lookup_index;

    /**
     * The end case of dimension index lookup.
     * Just returns a list of the arguments.
     */
    template
    <
      int N,
      typename Ret,
      template <typename...> class List
    >
    struct lookup_index<N, Ret, List>
    {
      /**
       * Transforms the template pack of indices into a list.
       */
      template <typename ...DimIndexes>
      Ret
      operator()(System& i, DimIndexes... dims) const
      {
        return Ret(dims...);
      }
    };
 
    /**
     * Looks up the dimension index of "argN".
     */
    template 
    <
      int N, 
      typename Ret,
      template <typename...> class List,
      type_index First,
      type_index ...Args
    >
    struct lookup_index<N, Ret, List, First, Args...>
    {
      /**
       * Does the actual index lookup.
       * @param i The system.
       * @param dims The indices looked up so far.
       */
      template
      <
        typename ...DimIndexes
      >
      Ret 
      operator()(System& i, DimIndexes... dims) const
      {
        std::ostringstream os;
        os << "arg" << N;
        return lookup_index<N+1, Ret, List, Args...>()
          (i, dims..., get_dimension_index(&i, to_u32string(os.str())));
      }
    };

    /*
     * Gets a value out of the context. Given the dimension index and a
     * metafunction which can give us the type of the value given the
     * TransLucid type index, it pulls the value of that type out of the
     * context at that dimension.
     */
    template
    <
      template <int> class ArgType,
      type_index type
    >
    struct make_constant
    {
      /**
       * The return type of the function.
       */
      typedef typename ArgType<type>::type ret;

      /**
       * Does the actual operation.
       * @param c The context.
       * @param index The TransLucid type index of the value.
       * @throw DimensionNotFound if the dimension is not found or if it is
       * of the wrong type.
       */
      ret 
      operator()(const Tuple& c, size_t index)
      {
        Tuple::const_iterator iter = c.find(index);
        if (iter == c.end() || iter->second.index() != type)
        {
          throw DimensionNotFound();
        }
        else
        {
          return iter->second.value<ret>();
        }
      }
    };

    template
    <
      int N, 
      template <int> class ArgType,
      type_index ...Args
    >
    struct build_arguments_imp;

    /**
     * Call the actual operation.
     * The specialisation for when we have run out of arguments to process.
     */
    template
    <
      int N,
      template <int> class ArgType
    >
    struct build_arguments_imp<N, ArgType>
    {
      /**
       * Calls the actual operation.
       * @param f The function.
       * @param args The dimension indices.
       * @param c The context.
       * @param constants The arguments.
       */
      template
      <
        typename F,
        typename List,
        typename ...Constants
      >
      TaggedConstant
      operator()
      (
        const F& f, const List& args, const Tuple& c, Constants... constants
      )
      {
        return f(constants..., c);
      }
    };

    /**
     * The implementation of build arguments.
     * Checks that the Nth argument is the right type and passes execution
     * on to getting the N+1th argument. Pulls that argument out of the
     * context.
     * @param N The Nth argument to retrieve.
     * @param The type of the Nth value.
     * @param First The type of the current value.
     * @param Args The remaining arguments.
     */
    template
    <
      int N, 
      template <int> class ArgType,
      type_index First,
      type_index ...Args
    >
    struct build_arguments_imp<N, ArgType, First, Args...>
    {
      /**
       * Build the actual arguments.
       * @param f The functor.
       * @param args The dimension indices of the arguments.
       * @param c The context.
       * @param constants The parameters seen so far.
       */
      template 
      <
        typename F, 
        typename List,
        typename ...Constants
      >
      TaggedConstant 
      operator()
      (
        const F& f, 
        const List& args, 
        const Tuple& c,
        Constants... constants
      )
      {
        return build_arguments_imp<N+1, ArgType, Args...>()
          (f, args, c, constants..., 
            make_constant<ArgType, First>()(c, std::get<N>(args)));
      }
    };
 
    /**
     * Builds the arguments to an operation WS.
     * @param ArgType A template metafunction which returns the type of the
     * nth argument.
     * @param Args The TransLucid type indices of the arguments.
     */
    template
    <
      template <int> class ArgType,
      type_index ...Args
    >
    struct build_arguments
    {
      /**
       * Builds the actual arguments and calls the operation.
       * @param f The functor of the operation.
       * @param args A list of the arguments.
       * @param c The context.
       */
      template 
      <
        typename F, 
        typename List
      >
      TaggedConstant 
      operator()
      (
        const F& f, 
        const List& args, 
        const Tuple& c
      )
      {
        return build_arguments_imp<0, ArgType, Args...>()(f, args, c);
      }
    };
  }

  /**
   * An operation hyperdaton. The indices must be known at compile time.
   * @param T A functor which carries out the operation.
   * @param Args The variadic template pack of the TransLucid argument indices.
   */
  template <typename T, 
    template <int> class ArgType, type_index ...Args>
  class OpWS : public WS
  {
    private:
    //a tuple of all the argN indexes
    typedef typename OpWSImp::generate_args_tuple_type<Args...>::type DimsType;
    DimsType m_args;

    //generates all of the args, then extracts them and checks that they
    //are the right type and passes them all to the functor
    public:
    OpWS(System& i)
    {
      m_args = OpWSImp::lookup_index<0, DimsType, std::tuple, Args...>()(i);
    }

    TaggedConstant operator()(const Tuple& c)
    {
      //uses variadic template parameters passed to operator() to call
      //T::operator()(args...)
      //it extracts the arguments out of the context and checks that they are
      //of the right type
      return OpWSImp::build_arguments<ArgType, Args...>()(m_t, m_args, c);
    }

    private:
    //the op functor
    T m_t;
  };
}
