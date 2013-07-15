/* The function types.
   Copyright (C) 2011,2012 Jarryd Beck

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
#include <tl/fixed_indexes.hpp>
#include <tl/system.hpp>
#include <tl/types/special.hpp>
#include <tl/types.hpp>

#include <tl/tyinf/type_inference.hpp>

#include <vector>
#include <functional>

namespace TransLucid
{
  class BaseFunctionType
  {
    public:
    BaseFunctionType() = default;

    BaseFunctionType(const std::vector<type_index>& type)
    : m_funtype(type)
    {
    }

    virtual ~BaseFunctionType() = default;
    //virtual ~BaseFunctionType() throw() {}

    BaseFunctionType*
    clone() const
    {
      return cloneSelf();
    }

    Constant
    apply(const Constant& c) const
    {
      return applyFn(c);
    }

    Constant
    apply(const std::vector<Constant>& params) const
    {
      return applyFn(params);
    }

    //we ignore the context
    template <typename... T>
    TimeConstant
    apply(const Constant& c, T&&... args) const
    {
      return applyFn(c, args...);
    }

    template <typename... T>
    TimeConstant
    apply(const std::vector<Constant>& params, T&&... args) const
    {
      return applyFn(params, args...);
    }

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(this);
    }

    virtual size_t
    arity() const = 0;

    TypeInference::TypeScheme
    type() const;

    private:

    virtual Constant
    applyFn(const Constant& c) const = 0;

    virtual Constant
    applyFn(const std::vector<Constant>& args) const = 0;

    virtual TimeConstant
    applyFn(const Constant& c, Delta& d, const Thread& w, size_t t) const = 0;

    virtual TimeConstant
    applyFn(const std::vector<Constant>& args, Delta& d, 
      const Thread& w, size_t t) const = 0;

    virtual BaseFunctionType*
    cloneSelf() const = 0;

    std::vector<type_index> m_funtype;
  };

  class BaseFunctionAbstraction : public BaseFunctionType
  {
    public:
    BaseFunctionAbstraction
    (
      System* system,
      const std::vector<dimension_index>& dims,
      const std::vector<dimension_index>& scope,
      const std::vector<WS*>& binds,
      WS* expr,
      Context& k
    )
    : m_dims(dims)
    , m_expr(expr)
    {
      if (m_expr == nullptr)
      {
        std::cerr << "base function built with nullptr body" << std::endl;
      }

      RhoManager rho(k);
      uint8_t index = 1;
      for (auto ws : binds)
      {
        rho.changeTop(index);
        auto c = (*ws)(k);
        auto d = system->getDimensionIndex(c); 

        m_binds.push_back(std::make_pair(d, k.lookup(d)));

        ++index;
      }

      //std::cerr << "binding in function:" << std::endl;
      for (auto d : scope)
      {
        //std::cerr << d << " ";
        m_binds.push_back(std::make_pair(d, k.lookup(d)));
      }
      //std::cerr << std::endl;

      //hold on to rho
      m_binds.push_back(std::make_pair(DIM_RHO, k.lookup(DIM_RHO)));
      m_binds.push_back(std::make_pair(DIM_TIME, k.lookup(DIM_TIME)));
    }

    BaseFunctionAbstraction
    (
      const std::vector<dimension_index>& dims,
      const std::vector<dimension_index>& scope,
      const std::vector<dimension_index>& binds,
      WS* expr,
      Context& k
    )
    : m_dims(dims)
    , m_expr(expr)
    {
      for (auto b : binds)
      {
        m_binds.push_back(std::make_pair(b, k.lookup(b)));
      }

      for (auto d : scope)
      {
        m_binds.push_back(std::make_pair(d, k.lookup(d)));
      }
      m_binds.push_back(std::make_pair(DIM_TIME, k.lookup(DIM_TIME)));
    }

    ~BaseFunctionAbstraction() throw() {}

    size_t
    arity() const
    {
      return 1;
    }

    private:
    Constant
    applyFn(const Constant& c) const;

    Constant
    applyFn(const std::vector<Constant>& args) const
    {
      if (m_dims.size() != args.size())
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      Context newk;
      ContextPerturber p(newk, m_binds);

      //these ranges are the same length because we just checked it
      auto dimIter = m_dims.begin();
      auto argIter = args.begin();

      while (dimIter != m_dims.end())
      {
        p.perturb(*dimIter, *argIter);
        ++dimIter;
        ++argIter;
      }

      return (*m_expr)(newk);
    }

    TimeConstant
    applyFn(const Constant& c, Delta& d, const Thread& w, size_t t) const
    {
      if (m_dims.size() != 1)
      {
        return std::make_pair(t, Types::Special::create(SP_TYPEERROR));
      }

      Context newk;
      ContextPerturber p(newk, m_binds);
      DeltaPerturber pd(d);

      p.perturb({{m_dims.at(0), c}});
      pd.perturb(m_dims.at(0));

      return (*m_expr)(newk, d, w, t);
    }

    TimeConstant
    applyFn(const std::vector<Constant>& args, Delta& d, 
      const Thread& w, size_t t) 
      const
    {
      if (m_dims.size() != args.size())
      {
        return std::make_pair(t, Types::Special::create(SP_TYPEERROR));
      }

      Context newk;
      ContextPerturber p(newk, m_binds);
      DeltaPerturber pd(d);

      //these ranges are the same length because we just checked it
      auto dimIter = m_dims.begin();
      auto argIter = args.begin();

      while (dimIter != m_dims.end())
      {
        p.perturb(*dimIter, *argIter);
        pd.perturb(*dimIter);
        ++dimIter;
        ++argIter;
      }

      return (*m_expr)(newk, d, w, t);
    }

    BaseFunctionAbstraction*
    cloneSelf() const
    {
      return new BaseFunctionAbstraction(*this);
    }

    std::vector<dimension_index> m_dims;
    std::vector<std::pair<dimension_index, Constant>> m_binds;
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

    template <int N, int... Args>
    struct n_args_caller : public n_args_caller<N-1, N, Args...>
    {
    };

    template <int... Args>
    struct n_args_caller<0, Args...>
    {
      template <typename F>
      Constant 
      operator()(F f, const std::vector<Constant>& args)
      {
        return f(args.at(Args-1)...);
      }
    };

    template <int N, typename F>
    Constant 
    call_n_args(F f, const std::vector<Constant>& args)
    {
      return n_args_caller<N>()(f, args);
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
    BuiltinBaseFunction(Fun&& f, std::vector<type_index> type)
    : BaseFunctionType(type)
    , m_fn(f)
    {
    }

    ~BuiltinBaseFunction() throw() {}

    BuiltinBaseFunction*
    cloneSelf() const
    {
      return new BuiltinBaseFunction(*this);
    }

    size_t
    arity() const
    {
      return NumArgs;
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

    TimeConstant
    applyFn(const Constant& c, Delta& d, const Thread& w, size_t t) const
    {
      //ignore thread and time for builtin functions
      return std::make_pair(t, applyFn(c));
    }

    TimeConstant
    applyFn(const std::vector<Constant>& args, Delta& d,
      const Thread& w, size_t t) const
    {
      //ignore thread and time for builtin functions
      return std::make_pair(t, applyFn(args));
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
      WS* expr,
      const std::vector<WS*>& binds,
      const std::vector<dimension_index>& scope,
      Context& k
    )
    : m_system(system), m_name(name), m_dim(argDim), m_expr(expr)
    {
      RhoManager rho(k);
      uint8_t index = 1;
      for (auto ws : binds)
      {
        rho.changeTop(index);
        auto c = (*ws)(k);
        auto d = system->getDimensionIndex(c); 

        m_binds.push_back(std::make_pair(d, k.lookup(d)));

        ++index;
      }

      //std::cerr << "binding in function:" << std::endl;
      for (auto d : scope)
      {
        //std::cerr << d << " ";
        m_binds.push_back(std::make_pair(d, k.lookup(d)));
      }
      //std::cerr << std::endl;

      //hold on to rho
      m_binds.push_back(std::make_pair(DIM_RHO, k.lookup(DIM_RHO)));
    }

    ValueFunctionType
    (
      System* system,
      const u32string& name, 
      dimension_index argDim, 
      WS* expr,
      const std::vector<dimension_index>& binds,
      const std::vector<dimension_index>& scope,
      Context& k
    )
    : m_system(system)
    , m_name(name)
    , m_dim(argDim)
    , m_expr(expr)
    {
      for (auto d : binds)
      {
        m_binds.push_back(std::make_pair(d, k.lookup(d)));
      }

      for (auto d : scope)
      {
        m_binds.push_back(std::make_pair(d, k.lookup(d)));
      }
    }

    ValueFunctionType*
    clone() const
    {
      return new ValueFunctionType(*this);
    }

    Constant
    apply(Context& k, const Constant& value) const;

    Constant
    apply(Context& kappa, Context& delta, const Constant& value) const;

    TimeConstant
    apply(Context& kappa, Delta& d, const Thread& w, size_t t,
      const Constant& value) const;

    size_t
    hash() const
    {
      return std::hash<u32string>()(m_name);
    }

    bool
    less(const ValueFunctionType& rhs) const;

    private:
    System* m_system;
    u32string m_name;
    dimension_index m_dim;
    WS* m_expr;
    std::vector<std::pair<dimension_index, Constant>> m_binds;
  };

  Constant
  createValueFunction
  (
    System *system,
    const u32string& name, 
    dimension_index argDim, 
    WS* expr,
    const std::vector<WS*>& binds,
    const std::vector<dimension_index>& scope,
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
    WS* expr,
    Context& kappa,
    Context& delta
  );
  
  TimeConstant
  createValueFunctionCachedNew
  (
    System *system,
    const u32string& name, 
    dimension_index argDim, 
    WS* expr,
    const std::vector<WS*>& binds,
    const std::vector<dimension_index>& scope,
    Context& kappa,
    Delta& delta,
    const Thread& w, 
    size_t t
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

      bool
      less(const Constant& lhs, const Constant& rhs);
    }
  }
  
  //all this does is return a bang abstraction type object with the
  //pointer that it is constructed with
  class BangAbstractionWS : public WS
  {
    public:
    BangAbstractionWS(BaseFunctionType* fn)
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
  
  enum FnType
  {
    FUN_BASE,
    FUN_VALUE,
    FUN_NAME
  };

  namespace detail
  {
    namespace apply_fun
    {
      template <FnType Type>
      struct FunIndex;

      template <>
      struct FunIndex<FUN_BASE>
      {
        static constexpr auto value = TYPE_INDEX_BASE_FUNCTION;
      };

      template <>
      struct FunIndex<FUN_VALUE>
      {
        static constexpr auto value = TYPE_INDEX_VALUE_FUNCTION;
      };

      template <FnType Type>
      struct GetValue;

      template <>
      struct GetValue<FUN_BASE>
      {
        static
        auto
        get(const Constant& fun)
          -> decltype(Types::BaseFunction::get(fun))
        {
          return Types::BaseFunction::get(fun);
        }

      };

      template <>
      struct GetValue<FUN_VALUE>
      {
        static
        auto
        get(const Constant& fun)
          -> decltype(Types::ValueFunction::get(fun))
        {
          return Types::ValueFunction::get(fun);
        }
      };

      template <FnType Type>
      struct ApplyFunction;

      template <>
      struct ApplyFunction<FUN_BASE>
      {
        template <typename Fun>
        static
        Constant
        apply(Fun&& fun, Context& k, const Constant& rhs)
        {
          return fun.apply(rhs);
        }
      };

      template <>
      struct ApplyFunction<FUN_VALUE>
      {
        template <typename Fun>
        static
        Constant
        apply(Fun&& fun, Context& k, const Constant& rhs)
        {
          return fun.apply(k, rhs);
        }
      };
    }
  }

  template <FnType Type>
  Constant
  applyFunction(Context& k, const Constant& lhs, const Constant& rhs)
  {
    if (lhs.index() == detail::apply_fun::FunIndex<Type>::value)
    {
      const auto& fnval = detail::apply_fun::GetValue<Type>::get(lhs);

      return detail::apply_fun::ApplyFunction<Type>::apply(fnval, k, rhs);
    }
    else
    {
      return Types::Special::create(SP_TYPEERROR);
    }
  }

  template <FnType Type>
  Constant
  applyFunction
  (
    Context& kappa, 
    Context& delta, 
    const Constant& lhs, 
    const Constant& rhs
  )
  {
    if (lhs.index() == detail::apply_fun::FunIndex<Type>::value)
    {
      const auto& fnval = detail::apply_fun::GetValue<Type>::get(lhs);

      return fnval.apply(kappa, delta, rhs);
    }
    else
    {
      return Types::Special::create(SP_TYPEERROR);
    }
  }

  template <FnType Type>
  TimeConstant
  applyFunction(Context& kappa, Delta& d, const Thread& w, size_t t,
    const Constant& lhs, const Constant& rhs)
  {
    if (lhs.index() == detail::apply_fun::FunIndex<Type>::value)
    {
      const auto& fnval = detail::apply_fun::GetValue<Type>::get(lhs);

      return fnval.apply(kappa, d, w, t, rhs);
    }
    else
    {
      return TimeConstant(t, Types::Special::create(SP_TYPEERROR));
    }
  }
}

#endif


