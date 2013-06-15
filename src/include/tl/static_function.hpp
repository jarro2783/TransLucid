/* Static function template
   Copyright (C) 2013 Jarryd Beck

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

#ifndef TL_STATIC_FUNCTION_HPP
#define TL_STATIC_FUNCTION_HPP

#include <tl/variant.hpp>
#include <tl/types.hpp>

#include <algorithm>
#include <vector>
#include <set>

namespace TransLucid
{
  namespace Static
  {
    namespace Functions
    {
      struct Topfun;

      struct Param;

      template <typename Prop>
      struct ApplyV;

      template <typename Prop>
      struct ApplyB;

      template <typename Prop>
      struct Down;

      template <typename Prop>
      struct Up;

      template <typename Prop>
      struct CBV;

      template <typename Prop>
      struct Base;
    }

    template <typename Prop>
    using Functor = Variant
      <
        Functions::Param,
        Functions::Topfun,
        recursive_wrapper<Functions::ApplyV<Prop>>,
        recursive_wrapper<Functions::ApplyB<Prop>>,
        recursive_wrapper<Functions::Down<Prop>>,
        recursive_wrapper<Functions::Up<Prop>>,
        recursive_wrapper<Functions::CBV<Prop>>,
        recursive_wrapper<Functions::Base<Prop>>
      >;

    template <typename Prop>
    u32string
    print_functor(const Functor<Prop>& f);

    namespace Functions
    {
      template <typename Prop>
      using FunctorList = std::vector<Functor<Prop>>;

      struct Param
      {
        dimension_index dim;
      };

      struct Topfun
      {
      };

      template <typename Prop>
      struct CBV
      {
        dimension_index dim;
        Prop property;
        FunctorList<Prop> functions;
        FunctorList<Prop> fundeps;
      };

      template <typename Prop>
      struct Base
      {
        std::vector<dimension_index> dims;
        Prop property;
        FunctorList<Prop> functions;
        FunctorList<Prop> fundeps;
      };

      template <typename Prop>
      struct Up
      {
        Prop property;
        FunctorList<Prop> functions;
        FunctorList<Prop> fundeps;
      };

      template <typename Prop>
      struct ApplyV
      {
        Functor<Prop> lhs;
        FunctorList<Prop> rhs;
      };

      template <typename Prop>
      struct ApplyB
      {
        Functor<Prop> lhs;
        std::vector<FunctorList<Prop>> params;
      };

      template <typename Prop>
      struct Down
      {
        Functor<Prop> body;
      };

      template <typename P>
      void
      prop_append(std::set<P>& dest, const std::set<P>& source)
      {
        dest.insert(source.begin(), source.end());
      }

      template <typename T>
      struct PropertyCounter;

      namespace detail
      {
        class Collector
        {
          public:

          typedef void result_type;

          //this is for param and topfun
          template <typename T, typename C>
          void
          operator()(const T& t, C& c)
          {
          }

          template <typename Prop, typename C>
          void
          operator()(const ApplyV<Prop>& applyv, C& c)
          {
            apply_visitor(*this, applyv.lhs, c);

            for (const auto& r : applyv.rhs)
            {
              apply_visitor(*this, r, c);
            }
          }

          template <typename Prop, typename C>
          void
          operator()(const ApplyB<Prop>& applyb, C& c)
          {
            apply_visitor(*this, applyb.lhs, c);

            for (const auto& param : applyb.params)
            {
              for (const auto& f : param)
              {
                apply_visitor(*this, f, c);
              }
            }
          }

          template <typename Prop, typename C>
          void
          operator()(const Down<Prop>& down, C& c)
          {
            apply_visitor(*this, down.body, c);
          }

          template <typename Prop, typename C>
          void
          operator()(const Base<Prop>& base, C& c)
          {
            c.insert(base.property.begin(), base.property.end());

            for (const auto& f : base.functions)
            {
              apply_visitor(*this, f, c);
            }

            for (const auto& f : base.fundeps)
            {
              apply_visitor(*this, f, c);
            }
          }

          template <typename Prop, typename C>
          void
          operator()(const Up<Prop>& up, C& c)
          {
            c.insert(up.property.begin(), up.property.end());

            for (const auto& f : up.functions)
            {
              apply_visitor(*this, f, c);
            }

            for (const auto& f : up.fundeps)
            {
              apply_visitor(*this, f, c);
            }
          }

          template <typename Prop, typename C>
          void
          operator()(const CBV<Prop>& cbv, C& c)
          {
            c.insert(cbv.property.begin(), cbv.property.end());

            for (const auto& f : cbv.functions)
            {
              apply_visitor(*this, f, c);
            }

            for (const auto& f : cbv.fundeps)
            {
              apply_visitor(*this, f, c);
            }
          }
        };

        struct Accumulator
        {
          Accumulator(size_t& value)
          : m_value(value)
          {
          }

          void
          operator()(size_t n)
          {
            m_value += n;
          }

          private:

          size_t& m_value;
        };

        //The number of objects is 1 (for self) + the number of objects in
        //the children, plus the size of the properties
        struct ObjectCounter
        {
          typedef size_t result_type;

          //for all the single depth objects
          template <typename T>
          size_t
          operator()(const T& t)
          {
            return 1;
          }

          template <typename Prop>
          size_t
          operator()(const CBV<Prop>& cbv)
          {
            using std::placeholders::_1;
            size_t count = PropertyCounter<Prop>()(cbv.property);

            Accumulator accum(count);

            std::for_each(cbv.functions.begin(), cbv.functions.end(),
              std::bind(accum, std::bind(visitor_applier(), *this, _1)));

            std::for_each(cbv.fundeps.begin(), cbv.fundeps.end(),
              std::bind(accum, std::bind(visitor_applier(), *this, _1)));

            return count + 1;
          }

          template <typename Prop>
          size_t
          operator()(const Base<Prop>& base)
          {
            using std::placeholders::_1;
            size_t count = PropertyCounter<Prop>()(base.property);

            Accumulator accum(count);

            std::for_each(base.functions.begin(), base.functions.end(),
              std::bind(accum, std::bind(visitor_applier(), *this, _1)));

            std::for_each(base.fundeps.begin(), base.fundeps.end(),
              std::bind(accum, std::bind(visitor_applier(), *this, _1)));

            return count + 1;
          }

          template <typename Prop>
          size_t
          operator()(const Up<Prop>& up)
          {
            using std::placeholders::_1;
            size_t count = PropertyCounter<Prop>()(up.property);

            Accumulator accum(count);

            std::for_each(up.functions.begin(), up.functions.end(),
              std::bind(accum, std::bind(visitor_applier(), *this, _1)));

            std::for_each(up.fundeps.begin(), up.fundeps.end(),
              std::bind(accum, std::bind(visitor_applier(), *this, _1)));

            return count + 1;
          }
        };
      }

      template <typename Prop, typename C>
      void
      collect_properties(const Functor<Prop>& f, C& c)
      {
        detail::Collector collect;

        apply_visitor(collect, f, c);
      }

      template <typename Prop>
      size_t
      count_objects(const Functor<Prop>& f)
      {
        detail::ObjectCounter count;

        return apply_visitor(count, f);
      }

      template <typename Prop>
      size_t
      count_objects(const FunctorList<Prop>& F)
      {
        size_t result = 0;
        std::for_each(F.begin(), F.end(), 
          [&result] (const Functor<Prop>& f) -> void
          {
            result += count_objects(f);
          }
        );

        return result;
      }

      template <typename Prop>
      using Substitution = std::map<dimension_index, FunctorList<Prop>>;

      template <typename Prop>
      FunctorList<Prop>
      sub(const Functor<Prop>& f, const Substitution<Prop>& P);

      template <typename Prop>
      FunctorList<Prop>
      subs(const FunctorList<Prop>& F, const Substitution<Prop>& P)
      {
        FunctorList<Prop> all;

        for (const auto& f : F)
        {
          auto result = sub(f, P);
          all.insert(all.end(), result.begin(), result.end());
        }

        return all;
      }

      struct is_app
      {
        template <typename Prop>
        bool
        operator()(const Functor<Prop>& f) const
        {
          if (get<ApplyB<Prop>>(&f) != nullptr)
          {
            return true;
          }
          else if (get<ApplyV<Prop>>(&f) != nullptr)
          {
            return true;
          }
          else if (get<Down<Prop>>(&f) != nullptr)
          {
            return true;
          }
          
          return false;
        }
      };

      namespace detail
      {
        template <typename Prop>
        class SubHelper
        {
          public:
          typedef FunctorList<Prop> result_type;

          result_type
          operator()(const Param& param, const Substitution<Prop>& p)
          {
            auto iter = p.find(param.dim);

            if (iter == p.end())
            {
              return result_type{param};
            }
            else
            {
              return iter->second;
            }
          }

          result_type
          operator()(const Topfun& top, const Substitution<Prop>& p)
          {
            return result_type{top};
          }

          result_type
          operator()(const CBV<Prop>& cbv, const Substitution<Prop>& p)
          {
            return result_type
            {
              CBV<Prop>{cbv.dim, cbv.property, subs(cbv.functions, p)}
            };
          }

          result_type
          operator()(const Base<Prop>& base, const Substitution<Prop>& p)
          {
            return result_type
            {
              Base<Prop>{base.dims, base.property, subs(base.functions, p)}
            };
          }

          result_type
          operator()(const Up<Prop>& up, const Substitution<Prop>& p)
          {
            return result_type{Up<Prop>{up.property, subs(up.functions, p)}};
          }

          result_type
          operator()(const Down<Prop>& down, const Substitution<Prop>& p)
          {
            result_type result;

            auto substituted = sub(down.body, p);

            for (const auto& f : substituted)
            {
              result.push_back(Down<Prop>{f});
            }

            return result;
          }

          result_type
          operator()(const ApplyB<Prop>& applyb, const Substitution<Prop>& p)
          {
            result_type result;

            auto substituted = sub(applyb.lhs, p);

            std::vector<FunctorList<Prop>> params;

            for (const auto& param_j : applyb.params)
            {
              params.push_back(subs(param_j, p));
            }

            for (const auto& f : substituted)
            {
              result.push_back(ApplyB<Prop>{f, params});
            }

            return result;
          }

          result_type
          operator()(const ApplyV<Prop>& applyv, const Substitution<Prop>& p)
          {
            result_type result;

            auto substituted = sub(applyv.lhs, p);

            for (const auto& f : substituted)
            {
              result.push_back(ApplyV<Prop>{f, subs(applyv.rhs, p)});
            }
            
            return result;
          }
        };
      }

      template <typename Prop>
      FunctorList<Prop>
      sub(const Functor<Prop>& f, const Substitution<Prop>& P)
      {
        detail::SubHelper<Prop> helper;
        return apply_visitor(helper, f, P);
      }

      template <typename Prop>
      std::tuple<Prop, FunctorList<Prop>, FunctorList<Prop>>
      eval(const Functor<Prop>& f);

      template <typename Prop>
      class EvalHelper
      {
        public:

        typedef std::tuple<Prop, FunctorList<Prop>, FunctorList<Prop>> 
          result_type;

        template <typename T>
        result_type
        operator()(const T& t)
        {
          return std::make_tuple(Prop(), FunctorList<Prop>{t}, 
            FunctorList<Prop>{});
        }

        result_type
        operator()(const Down<Prop>& down)
        {
          Prop property;
          FunctorList<Prop> Fcal;

          auto a = eval(down.body);
          auto b = evals_down(std::get<1>(a));

          prop_append(property, std::get<0>(a));
          prop_append(property, std::get<0>(b));

          Fcal = std::get<2>(a);
          Fcal.insert(Fcal.end(), std::get<2>(b).begin(), 
            std::get<2>(b).end());

          return std::make_tuple(property, std::get<1>(b), Fcal);
        }

        result_type
        operator()(const ApplyB<Prop>& applyb)
        {
          Prop property;
          FunctorList<Prop> Fcal;

          auto a = eval(applyb.lhs);
          auto b = evals_applyb(std::get<1>(a), applyb.params);

          prop_append(property, std::get<0>(a));
          prop_append(property, std::get<0>(b));

          Fcal = std::get<2>(a);
          Fcal.insert(Fcal.end(), std::get<2>(b).begin(), 
            std::get<2>(b).end());

          return std::make_tuple(property, std::get<1>(b), Fcal);
        }

        result_type
        operator()(const ApplyV<Prop>& applyv)
        {
          Prop property;
          FunctorList<Prop> Fcal;

          auto a = eval(applyv.lhs);
          auto b = evals_applyv(std::get<1>(a), applyv.rhs);

          prop_append(property, std::get<0>(a));
          prop_append(property, std::get<0>(b));

          Fcal = std::get<2>(a);
          Fcal.insert(Fcal.end(), std::get<2>(b).begin(), 
            std::get<2>(b).end());

          return std::make_tuple(property, std::get<1>(b), Fcal);
        }
      };

      template <typename Prop>
      std::tuple<Prop, FunctorList<Prop>, FunctorList<Prop>>
      eval(const Functor<Prop>& f)
      {
        EvalHelper<Prop> helper;
        return apply_visitor(helper, f);
      }

      template<typename Prop>
      std::tuple<Prop, FunctorList<Prop>, FunctorList<Prop>>
      evalall(const FunctorList<Prop>& F)
      {
        Prop prop;
        FunctorList<Prop> funcs;
        FunctorList<Prop> Fcal;

        for (const auto& f : F)
        {
          auto result = eval(f);
          prop_append(prop, std::get<0>(result));
          funcs.insert(funcs.end(), std::get<1>(result).begin(), 
            std::get<1>(result).end());
          funcs.insert(funcs.end(), std::get<2>(result).begin(), 
            std::get<2>(result).end());
        }

        return std::make_tuple(prop, funcs, Fcal);
      }
      
      template<typename Prop>
      std::tuple<Prop, FunctorList<Prop>, FunctorList<Prop>>
      evals_applyv(const FunctorList<Prop>& F_0, const FunctorList<Prop>& F_1)
      {
        Prop property;
        FunctorList<Prop> funcs;
        FunctorList<Prop> Fcal;

        for (const auto& f : F_0)
        {
          auto c = get<const CBV<Prop>>(&f);

          if (c == nullptr)
          {
            throw "non cbv found in applyv";
          }
          else
          {
            prop_append(property, c->property);
            Substitution<Prop> p{{c->dim, F_1}};

            auto result = evalall(subs(c->functions, p));
            prop_append(property, std::get<0>(result));
            funcs.insert(funcs.end(), std::get<1>(result).begin(), 
              std::get<1>(result).end());
            Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
              std::get<2>(result).end());

            result = evalall(subs(c->functions, p));
            prop_append(property, std::get<0>(result));
            Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
              std::get<2>(result).end());
              
            std::copy_if(
              std::get<1>(result).begin(), 
              std::get<1>(result).end(),
              std::back_inserter(Fcal),
              is_app()
            );

          }
        }

        return std::make_tuple(property, funcs, Fcal);
      }

      template <typename Prop>
      std::tuple<Prop, FunctorList<Prop>, FunctorList<Prop>>
      evals_applyb(const FunctorList<Prop>& F_0, 
        const std::vector<FunctorList<Prop>>& args)
      {
        Prop property;
        FunctorList<Prop> funcs;
        FunctorList<Prop> Fcal;

        for (const auto& f : F_0)
        {
          auto base = get<const Base<Prop>>(&f);

          if (base == nullptr)
          {
            throw U"non base in applyb: " + print_functor(f);
            //funcs.push_back(ApplyB<Prop>{f, args});
          }
          else
          {
            if (base->dims.size() != args.size())
            {
              throw "wrong arity in applyb";
            }

            Substitution<Prop> p;

            auto argsiter = args.begin();
            auto dimsiter = base->dims.begin();

            while (dimsiter != base->dims.end())
            {
              p.insert(std::make_pair(*dimsiter, *argsiter));
              ++dimsiter;
            }

            auto result = evalall(subs(base->functions, p));

            prop_append(property, std::get<0>(result));
            prop_append(property, base->property);
            funcs.insert(funcs.end(), std::get<1>(result).begin(), 
              std::get<1>(result).end());

            result = evalall(subs(base->fundeps, p));

            prop_append(property, std::get<0>(result));
            Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
              std::get<2>(result).end());

            std::copy_if(
              std::get<1>(result).begin(), 
              std::get<1>(result).end(),
              std::back_inserter(Fcal),
              is_app()
            );
          }
        }

        return std::make_tuple(property, funcs, Fcal);
      }

      template <typename Prop>
      std::tuple<Prop, FunctorList<Prop>, FunctorList<Prop>>
      evals_down(const FunctorList<Prop>& F_0)
      {
        Prop property;
        FunctorList<Prop> funcs;
        FunctorList<Prop> Fcal;

        for (const auto& f : F_0)
        {
          auto up = get<const Up<Prop>>(&f);

          if (up == nullptr)
          {
            throw U"non up found in down: " + print_functor(f);
          }
          else
          {
            prop_append(property, up->property);
            funcs.insert(funcs.end(), up->functions.begin(), 
              up->functions.end());
            Fcal.insert(Fcal.end(), up->fundeps.begin(), up->fundeps.end());
          }
        }

        return std::make_tuple(property, funcs, Fcal);
      }
    }
  }
}

#endif
