/* The function types.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#ifndef TYPES_FUNCTION_HPP_INCLUDED
#define TYPES_FUNCTION_HPP_INCLUDED

#include <tl/context.hpp>
#include <tl/system.hpp>
#include <tl/types/special.hpp>
#include <tl/types.hpp>

#include <vector>
#include <functional>

namespace TransLucid
{
  class BaseFunctionType
  {
    public:
    BaseFunctionType
    (
    )
    {
    }

    virtual ~BaseFunctionType() = default;
    //virtual ~BaseFunctionType() throw() {}

    BaseFunctionType*
    clone() const
    {
      return cloneSelf();
    }

    //we ignore the context
    Constant
    apply(const Constant& c) const
    {
      return applyFn(c);
    }

    Constant
    apply(const std::vector<Constant>& args) const
    {
      return applyFn(args);
    }

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(this);
    }

    private:
    virtual Constant
    applyFn(const Constant& c) const = 0;

    virtual Constant
    applyFn(const std::vector<Constant>& args) const = 0;

    virtual BaseFunctionType*
    cloneSelf() const = 0;
  };

  class BaseFunctionAbstraction : public BaseFunctionType
  {
    public:
    BaseFunctionAbstraction
    (
      dimension_index argDim,
      const std::vector<dimension_index>& scope,
      WS* expr,
      const Context& k
    )
    : m_dim(argDim)
    , m_expr(expr)
    , m_k(k)
    {
      for (auto d : scope)
      {
        m_scope.push_back(std::make_pair(d, k.lookup(d)));
      }
    }

    ~BaseFunctionAbstraction() throw() {}

    private:
    Constant
    applyFn(const Constant& c) const;

    Constant
    applyFn(const std::vector<Constant>& args) const
    {
      //TODO fix this
      return Constant();
    }

    BaseFunctionAbstraction*
    cloneSelf() const
    {
      return new BaseFunctionAbstraction(*this);
    }

    dimension_index m_dim;
    std::vector<std::pair<dimension_index, Constant>> m_scope;
    WS* m_expr;
    Tuple m_k;
  };

  namespace detail
  {
    template <size_t N, typename... Args>
    struct make_const_func_n
    {
      //we generate 0--N, so we need to go from 0--(N-1)
      typedef typename make_const_func_n<N-1, Args..., const Constant&>::type
        type;
    };

    template <typename... Args>
    struct make_const_func_n<0, Args...>
    {
      typedef typename std::function<Constant(Args...)> type;
    };

    template <size_t N, size_t... Args>
    struct n_args_caller : public n_args_caller<N-1, N, Args...>
    {
    };

    template <size_t... Args>
    struct n_args_caller<-1, Args...>
    {
      template <typename F>
      Constant 
      operator()(F f, const std::vector<Constant>& args)
      {
        return f(args.at(Args)...);
      }
    };

    template <size_t N, typename F>
    Constant 
    call_n_args(F f, const std::vector<Constant>& args)
    {
      return n_args_caller<N-1>()(f, args);
    }

    template <size_t N>
    struct apply_one_func
    {
      template <typename F>
      Constant
      operator()(F f, const Constant&)
      {
        return Types::Special::create(SP_TYPEERROR);
      }
    };

    template <>
    struct apply_one_func<1>
    {
      template <typename F>
      Constant
      operator()(F f, const Constant& c)
      {
        return f(c);
      }
    };
  }

  //all the base functions that are function pointers in C++
  template <size_t NumArgs>
  class BuiltinBaseFunction : public BaseFunctionType
  {
    public:

    typedef typename detail::make_const_func_n<NumArgs>::type func_type;

    template <typename Fun>
    constexpr BuiltinBaseFunction(Fun&& f)
    : m_fn(f)
    {
    }

    ~BuiltinBaseFunction() throw() {}

    BuiltinBaseFunction*
    cloneSelf() const
    {
      return new BuiltinBaseFunction(*this);
    }

    #if 0
    template <typename E = typename std::enable_if<NumArgs == 1, void>::type>
    Constant
    apply_one_func
    (
      const Constant& c,
      int = 0
    ) const
    {
      return m_fn(c);
    }

    template <typename E = typename std::enable_if<NumArgs != 1, void>::type>
    Constant
    apply_one_func
    (
      const Constant& c,
      float = 0.0
    ) const
    {
      return Types::Special::create(SP_TYPEERROR);
    }
    #endif

    Constant
    applyFn(const Constant& c) const
    {
      if (NumArgs != 1)
      {
        return Types::Special::create(SP_TYPEERROR);
      }
      return detail::apply_one_func<NumArgs>()(m_fn, c);
    }

    Constant
    applyFn(const std::vector<Constant>& args) const
    {
      if (NumArgs != args.size())
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return detail::call_n_args<NumArgs>(m_fn, args);
    }

    private:
    func_type m_fn;
  };

  class ValueFunctionType
  {
    public:
    ValueFunctionType
    (
      System* system,
      const u32string& name, 
      dimension_index argDim, 
      const std::vector<dimension_index>& scope,
      const std::vector<std::pair<dimension_index, Constant>>& free,
      WS* expr,
      Context& k
    )
    : m_name(name), m_dim(argDim), m_expr(expr)
    {
      for (auto d : scope)
      {
        m_scopeDims.push_back(std::make_pair(d, k.lookup(d)));
      }

      std::copy(free.begin(), free.end(), std::back_inserter(m_scopeDims));
    }

    ValueFunctionType*
    clone() const
    {
      return new ValueFunctionType(*this);
    }

    Constant
    apply(Context& k, const Constant& value) const;

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(m_expr);
    }

    private:
    u32string m_name;
    dimension_index m_dim;
    std::vector<std::pair<dimension_index, Constant>> m_scopeDims;
    WS* m_expr;
  };

  class NameFunctionType
  {
    public:
    NameFunctionType
    (
      System* system,
      const u32string& name, 
      dimension_index argDim, 
      dimension_index odometerDim, 
      const std::vector<dimension_index>& scope,
      const std::vector<std::pair<dimension_index, Constant>>& free,
      WS* expr,
      Context& k
    );

    Constant
    apply
    (
      Context& k, 
      const Constant& c, 
      std::vector<dimension_index>& Lall
    ) const;

    NameFunctionType*
    clone() const
    {
      return new NameFunctionType(*this);
    }

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(m_expr);
    }

    private:
    u32string m_name;
    dimension_index m_argDim;
    dimension_index m_odometerDim;
    WS* m_expr;

    std::vector<std::pair<dimension_index, Constant>> m_scopeDims;
  };

  Constant
  createValueFunction
  (
    System *system,
    const u32string& name, 
    dimension_index argDim, 
    const std::vector<dimension_index>& scope,
    const std::vector<std::pair<u32string, dimension_index>>& free,
    WS* expr,
    Context& kappa
  );

  //check for scope dimensions
  //evaluate free variables, checking for demands
  //then create the actual function
  Constant
  createValueFunctionCached
  (
    System *system,
    const u32string& name, 
    dimension_index argDim, 
    const std::vector<dimension_index>& scope,
    const std::vector<std::pair<u32string, dimension_index>>& free,
    WS* expr,
    Context& kappa,
    Context& delta
  );

  Constant
  createNameFunction
  (
    System *system,
    const u32string& name, 
    dimension_index argDim, 
    dimension_index odometerDim,
    const std::vector<dimension_index>& scope,
    const std::vector<std::pair<u32string, dimension_index>>& free,
    WS* expr,
    Context& kappa
  );

  //check for scope dimensions
  //evaluate free variables, checking for demands
  //then create the actual function
  Constant
  createNameFunctionCached
  (
    System *system,
    const u32string& name, 
    dimension_index argDim, 
    dimension_index odometerDim,
    const std::vector<dimension_index>& scope,
    const std::vector<std::pair<u32string, dimension_index>>& free,
    WS* expr,
    Context& kappa,
    Context& delta
  );

  namespace Types
  {
    namespace BaseFunction
    {
      Constant
      create(const BaseFunctionType& f);

      const BaseFunctionType&
      get(const Constant& c);

      bool
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);
    }
    
    namespace ValueFunction
    {
      Constant
      create(const ValueFunctionType& f);

      const ValueFunctionType&
      get(const Constant& c);

      bool
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);
    }

    namespace NameFunction
    {
      Constant
      create(const NameFunctionType& f);

      const NameFunctionType&
      get(const Constant& c);

      bool
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);
    }
  }
  
  //all this does is return a bang abstraction type object with the
  //pointer that it is constructed with
  class BangAbstractionWS : public WS
  {
    public:
    constexpr BangAbstractionWS(BaseFunctionType* fn)
    : m_fn(fn)
    {
    }

    ~BangAbstractionWS()
    {
      delete m_fn;
    }

    Constant
    operator()(Context& k)
    {
      return Types::BaseFunction::create(*m_fn);
    }

    Constant
    operator()(Context& kappa, Context& delta)
    {
      return operator()(kappa);
    }

    private:
    BaseFunctionType* m_fn;
  };
}

#endif


