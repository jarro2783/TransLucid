/* Constraint graph for type inference.
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

#ifndef TL_TYINF_CONSTRAINT_GRAPH_HPP_INCLUDED
#define TL_TYINF_CONSTRAINT_GRAPH_HPP_INCLUDED

#include <map>
#include <queue>
#include <vector>

#include <tl/tyinf/type.hpp>
#include <tl/tyinf/type_variable.hpp>

#include <tl/utility.hpp>

#include <boost/intrusive_ptr.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    //represents the constraint that lhs <= rhs
    struct Constraint
    {
      Type lhs;
      Type rhs;
    };

    // s <= a ? lhs <= rhs
    // s is less than a implies that lhs is less than rhs
    // the same constraint will be added for multiple a's, but this is
    // an external interface to a conditional constraint, so we store 
    // everything
    struct CondConstraint
    {
      Type s;
      TypeVariable a; 
      TypeVariable lhs;
      TypeVariable rhs;
    };

    void
    subc(const Constraint& c, std::vector<Constraint>& result);

    //a constraint graph is usually sparse, so it will be stored here as
    //a map from type variables alpha to a struct holding three things:
    //1. a list of <= type variables
    //2. the set of constraints in C\uparrow(alpha)
    //3. the set of constraints in C\downarrow(alpha)
    class ConstraintGraph
    {
      public:

      ConstraintGraph() = default;
      ConstraintGraph(const ConstraintGraph&);

      ConstraintGraph&
      operator=(const ConstraintGraph& rhs);

      //Makes a union with another constraint graph.
      //The type variables must be disjoint.
      //The current one becomes the result.
      void
      make_union(const ConstraintGraph&);

      //this one only works if t1 and t2 are one of the above three cases
      void
      add_to_closure(const Constraint& c);

      void
      add_conditional(const CondConstraint& cc);

      void
      collect(const VarSet& pos, const VarSet& neg);

      Type
      upper(TypeVariable a) const;

      Type
      lower(TypeVariable b) const;

      void
      erase_var(TypeVariable a)
      {
        m_graph.erase(a);
      }

      std::vector<TypeVariable>
      predecessor(TypeVariable a) const
      {
        auto iter = m_graph.find(a);

        if (iter != m_graph.end())
        {
          return iter->second.less;
        }
        else
        {
          return {};
        }
      }

      std::vector<TypeVariable>
      successor(TypeVariable a) const
      {
        auto iter = m_graph.find(a);

        if (iter != m_graph.end())
        {
          return iter->second.greater;
        }
        else
        {
          return {};
        }
      }

      //is a < b in this graph?
      bool
      less(TypeVariable a, TypeVariable b) const;

      u32string
      print(System& system) const;

      void
      setUpper(TypeVariable a, Type t);

      void
      setLower(TypeVariable a, Type t);

      void
      setPredecessor(TypeVariable a, const std::vector<TypeVariable>& pred);

      void
      setSuccessor(TypeVariable a, const std::vector<TypeVariable>& succ);

      void
      setVariable
      (
        TypeVariable v,
        const Type& lower,
        const Type& upper,
        const std::vector<TypeVariable>& pred,
        const std::vector<TypeVariable>& succ
      )
      {
        m_graph[v] = ConstraintNode
          {
            pred, succ, lower, upper
          };
      }

      template <typename Lower, typename Upper>
      void
      rewrite_bounds(Lower rl, Upper ru)
      {
        for (auto& c : m_graph)
        {
          c.second.lower = rl(c.second.lower);
          c.second.upper = ru(c.second.upper);
        }
      }

      template <typename Rewriter>
      static
      ConstraintGraph
      rename_vars(const ConstraintGraph& C, Rewriter rewrite)
      {
        ConstraintGraph result;

        std::map<ConditionalNode*, ConditionalNode*> noderewrites;

        //rewrite the conditional constraints
        for (auto ccn : C.m_conditionals)
        {
          std::unique_ptr<ConditionalNode> p(new ConditionalNode(
            ccn->s, 
            rewrite.rename_var(ccn->lhs),
            rewrite.rename_var(ccn->rhs),
            &result
          ));

          result.m_conditionals.insert(p.get());
          
          noderewrites[ccn] = p.get();

          p.release();
        }

        for (const auto& v : C.m_graph)
        {
          ConstraintNode node;

          auto r = rewrite.rename_var(v.first);

          for (auto t : v.second.less)
          {
            insert_sorted(node.less, rewrite.rename_var(t));
          }

          for (auto t : v.second.greater)
          {
            insert_sorted(node.greater, rewrite.rename_var(t));
          }

          node.lower = rewrite.rewrite_type(v.second.lower);
          node.upper = rewrite.rewrite_type(v.second.upper);

          for (const auto& cc : v.second.conditions)
          {
            auto rewritten = noderewrites[cc.get()];
            assert(rewritten != nullptr);
            node.conditions.insert(CondNodeP(rewritten));
          }

          result.m_graph[r] = node;
        }

        return result;
      }

      //when anything in S is less than any variable a in the graph, set
      //gamma < a
      void
      rewrite_less(TypeVariable gamma, const VarSet& S);

      //when any variable a is less than anything in S, set a < lambda
      void
      rewrite_greater(TypeVariable lambda, const VarSet& S);

      //if anything in S is less than anything in T then gamma < lambda
      void
      rewrite_lub_glb(TypeVariable gamma, const VarSet& S,
        TypeVariable lambda, const VarSet& T);

      template <typename F, typename Cond>
      void
      for_each_lower_if(F f, Cond c) const
      {
        for (const auto& v : m_graph)
        {
          if (c(v))
          {
            f(v.second.lower);
          }
        }
      }

      template <typename F, typename Cond>
      void
      for_each_upper_if(F f, Cond c) const
      {
        for (const auto& v : m_graph)
        {
          if (c(v))
          {
            f(v.second.upper);
          }
        }
      }

      template <typename F, typename Cond>
      void
      for_each_condition_if(F f, Cond c) const
      {
        for (const auto& v : m_graph)
        {
          if (c(v))
          {
            std::vector<CondConstraint> result;
            std::transform(v.second.conditions.begin(), 
              v.second.conditions.end(),
              std::back_inserter(result),
              [&v] (const CondNodeP& n) -> CondConstraint
              {
                return CondConstraint{n->s, v.first, n->lhs, n->rhs};
              }
            );

            f(result);
          }
        }
      }

      std::vector<TypeVariable>
      domain() const
      {
        std::vector<TypeVariable> d;

        for (const auto& v : m_graph)
        {
          d.push_back(v.first);
        }

        return d;
      }

      //finish this
      #if 0
      template <typename Vars, typename Types>
      static
      void
      rewrite_variables(const ConstraintGraph& g, Vars vars, Types types)
      {
        ConstraintGraph C;

        for (const auto& v : g.m_graph)
        {
          ConstraintNode node;
        }
      }
      #endif

      private:

      typedef std::queue<Constraint> ConstraintQueue;
      
      struct ConditionalNode
      {
        ConditionalNode
        (
          Type s, 
          TypeVariable lhs, 
          TypeVariable rhs,
          ConstraintGraph* owner
        )
        : s(s)
        , lhs(lhs)
        , rhs(rhs)
        , m_owner(owner)
        {
        }

        Type s;
        TypeVariable lhs;
        TypeVariable rhs;

        int counter = 0;
        ConstraintGraph *m_owner;
      };

      friend void intrusive_ptr_add_ref(ConditionalNode*);
      friend void intrusive_ptr_release(ConditionalNode*);

      typedef boost::intrusive_ptr<ConditionalNode> CondNodeP;

      struct ConstraintNode
      {
        ConstraintNode()
        : lower(TypeBot())
        , upper(TypeTop())
        {
        }

        ConstraintNode
        (
          const std::vector<TypeVariable>& less,
          const std::vector<TypeVariable>& greater,
          const Type& lower,
          const Type& upper
        )
        : less(less)
        , greater(greater)
        , lower(lower)
        , upper(upper)
        {
        }

        ConstraintNode
        (
          const std::vector<TypeVariable>& less,
          const std::vector<TypeVariable>& greater,
          const Type& lower,
          const Type& upper,
          const std::set<CondNodeP>& conditions
        )
        : less(less)
        , greater(greater)
        , lower(lower)
        , upper(upper)
        , conditions(conditions)
        {
        }

        //store both the less and the greater because we need
        //to look up both
        std::vector<TypeVariable> less;
        std::vector<TypeVariable> greater;
        Type lower;
        Type upper;

        //all the conditional constraints relating to this type var
        std::set<CondNodeP> conditions;
      };

      void
      copy_contents(const ConstraintGraph& other);

      void
      add_constraint(TypeVariable a, TypeVariable b, ConstraintQueue&);

      void
      add_constraint(Type t, TypeVariable b, ConstraintQueue&);

      void
      add_constraint(TypeVariable a, Type t, ConstraintQueue&);

      void
      add_less(TypeVariable a, TypeVariable b);

      void
      add_less_closed(TypeVariable a, TypeVariable b);

      std::set<ConditionalNode*> m_conditionals;
      std::map<TypeVariable, ConstraintNode> m_graph;

      decltype(m_graph)::iterator
      get_make_entry(TypeVariable a);

      void
      new_lower_closed(decltype(m_graph)::iterator var, Type bound)
      {
        var->second.lower = bound;
        check_conditionals(var);
      }

      void
      check_conditionals(decltype(m_graph)::iterator var);

      void
      check_single_conditional
      (
        decltype(m_graph)::iterator var,
        const CondNodeP& cc
      );
    };
  }
}

#endif
