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

#include <tl/variant.hpp>
#include <tl/types.hpp>

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
      };

      template <typename Prop>
      struct Base
      {
        std::vector<dimension_index> dims;
        Prop property;
        FunctorList<Prop> functions;
      };

      template <typename Prop>
      struct Up
      {
        Prop property;
        FunctorList<Prop> functions;
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
      std::pair<Prop, FunctorList<Prop>>
      eval(const Functor<Prop>& f);

      template <typename Prop>
      class EvalHelper
      {
        public:

        typedef std::pair<Prop, FunctorList<Prop>> result_type;

        template <typename T>
        result_type
        operator()(const T& t)
        {
          return std::make_pair(Prop(), FunctorList<Prop>{t});
        }

        result_type
        operator()(const Down<Prop>& down)
        {
          Prop property;

          auto a = eval(down.body);
          auto b = evals_down(a.second);

          prop_append(property, a.first);
          prop_append(property, b.first);

          return std::make_pair(property, b.second);
        }

        result_type
        operator()(const ApplyB<Prop>& applyb)
        {
          Prop property;

          auto a = eval(applyb.lhs);
          auto b = evals_applyb(a.second, applyb.params);

          prop_append(property, a.first);
          prop_append(property, b.first);

          return std::make_pair(property, b.second);
        }

        result_type
        operator()(const ApplyV<Prop>& applyv)
        {
          Prop property;

          auto a = eval(applyv.lhs);
          auto b = evals_applyv(a.second, applyv.rhs);

          prop_append(property, a.first);
          prop_append(property, b.first);

          return std::make_pair(property, b.second);
        }
      };

      template <typename Prop>
      std::pair<Prop, FunctorList<Prop>>
      eval(const Functor<Prop>& f)
      {
        EvalHelper<Prop> helper;
        return apply_visitor(helper, f);
      }

      template<typename Prop>
      std::pair<Prop, FunctorList<Prop>>
      evalall(const FunctorList<Prop>& F)
      {
        Prop prop;
        FunctorList<Prop> funcs;

        for (const auto& f : F)
        {
          auto result = eval(f);
          prop_append(prop, result.first);
          funcs.insert(funcs.end(), result.second.begin(), 
            result.second.end());
        }

        return std::make_pair(prop, funcs);
      }
      
      template<typename Prop>
      std::pair<Prop, FunctorList<Prop>>
      evals_applyv(const FunctorList<Prop>& F_0, const FunctorList<Prop>& F_1)
      {
        Prop property;
        FunctorList<Prop> funcs;

        for (const auto& f : F_0)
        {
          auto c = get<const CBV<Prop>*>(f);

          if (c == nullptr)
          {
            throw "non cbv found in applyv";
          }
          else
          {
            prop_append(property, c->property);
            Substitution<Prop> p{{c->dim, F_1}};
            auto result = evalall(subs(c->functions, p));
            prop_append(property, result.first);
            funcs.insert(funcs.end(), result.second.begin(), 
              result.second.end());
          }
        }

        return std::make_pair(property, funcs);
      }

      template <typename Prop>
      std::pair<Prop, FunctorList<Prop>>
      evals_applyb(const FunctorList<Prop>& F_0, 
        const std::vector<FunctorList<Prop>>& args)
      {
        Prop property;
        FunctorList<Prop> funcs;

        for (const auto& f : F_0)
        {
          auto base = get<const Base<Prop>*>(f);

          if (base == nullptr)
          {
            throw "non base in applyb";
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
            }

            auto result = evalall(subs(base->functions, p));

            prop_append(property, result.first);
            prop_append(property, base->property);
            funcs.insert(funcs.end(), result.second.begin(), 
              result.second.end());
          }
        }

        return std::make_pair(property, funcs);
      }

      template <typename Prop>
      std::pair<Prop, FunctorList<Prop>>
      evals_down(const FunctorList<Prop>& F_0)
      {
        Prop property;
        FunctorList<Prop> funcs;

        for (const auto& f : F_0)
        {
          auto up = get<const Up<Prop>*>(f);

          if (up == nullptr)
          {
            throw "non up found in down";
          }
          else
          {
            prop_append(property, up->property);
            funcs.insert(funcs.end(), up->functions.begin(), 
              up->functions.end());
          }
        }

        return std::make_pair(property, funcs);
      }
    }
  }
}