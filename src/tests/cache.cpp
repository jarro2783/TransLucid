/* Cache tests.
   Copyright (C) 2012 Jarryd Beck

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

/**
 * @file cache.cpp
 * Cache tests.
 */

#include <tl/cache.hpp>
#include <tl/constws.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/intension.hpp>

#include <gmpxx.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

TEST_CASE ( "upon use case", "mimic how upon uses the cache" )
{
#warning sort out this test
#if 0
  TL::Workshops::BoolConstWS bws{true};
  TL::dimension_index d = 1;
  TL::dimension_index B = 2;
  TL::dimension_index zero = 3;

  TL::Cache cache;

  {
    TL::Context k;

    //get with nothing
    TL::Constant r = cache.get(k);

    //should return calc
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //set that for nothing we need dimension d
    cache.set(k, TL::Types::Demand::create({d}));

    //set d to the dimension for zero
    TL::ContextPerturber p(k);
    p.perturb(d, TL::Types::Dimension::create(zero));

    //get having perturbed d
    r = cache.get(k); 
    //should be a calc again
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //set that now we need dimension zero
    cache.set(k, TL::Types::Demand::create({zero}));

    //now perturb for dimension zero
    p.perturb(zero, TL::Types::Intmp::create(0));

    //get having perturbed zero
    r = cache.get(k);
    //calc again
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //its value is zero now
    cache.set(k, TL::Types::Intmp::create(0));

    r = cache.get(k);
    REQUIRE(r.index() == TL::TYPE_INDEX_INTMP);
    CHECK(cmp(TL::Types::Intmp::get(r), 0) == 0);
  }


  //second round of computing something
  {
    TL::Context k2;

    //get with nothing should return a demand for dimension d
    TL::Constant r = cache.get(k2);

    REQUIRE(r.index() == TL::TYPE_INDEX_DEMAND);
    CHECK(TL::Types::Demand::get(r).dims() == 
      std::set<TL::dimension_index>({d}));

    //set dimension d
    TL::ContextPerturber p{k2};
    p.perturb(d, TL::Types::Dimension::create(zero));

    //get the value now
    r = cache.get(k2);
    REQUIRE(r.index() == TL::TYPE_INDEX_DEMAND);
    CHECK(TL::Types::Demand::get(r).dims() == 
      std::set<TL::dimension_index>({zero}));

    //set dimension zero to 1
    p.perturb(zero, TL::Types::Intmp::create(1));

    r = cache.get(k2);

    //now we need to calc this
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    cache.set(k2, TL::Types::Demand::create({B}));

    //for #!0 > 0 we need B
    p.perturb(B, TL::Types::Intension::create(&bws));

    //again, calc
    r = cache.get(k2);
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //now we want the variable at 0
    {
      TL::ContextPerturber p1{k2, {{zero, TL::Types::Intmp::create(0)}}};

      TL::Constant at1 = cache.get(k2);
      REQUIRE(at1.index() == TL::TYPE_INDEX_INTMP);
      CHECK(cmp(TL::Types::Intmp::get(at1), 0) == 0);
    }

    //now we are ready to set its value
    cache.set(k2, TL::Types::Intmp::create(1));
  }

  //round three, [0 <- 2]
  {
    TL::Context k3;

    //as before, we need d
    TL::Constant r = cache.get(k3);
    REQUIRE(r.index() == TL::TYPE_INDEX_DEMAND);
    CHECK(TL::Types::Demand::get(r).dims() ==
      std::set<TL::dimension_index>{d});

    //set dimension d
    TL::ContextPerturber p{k3};
    p.perturb(d, TL::Types::Dimension::create(zero));

    //get the value now
    r = cache.get(k3);
    REQUIRE(r.index() == TL::TYPE_INDEX_DEMAND);
    CHECK(TL::Types::Demand::get(r).dims() == 
      std::set<TL::dimension_index>({zero}));

    //set dimension zero to 2
    p.perturb(zero, TL::Types::Intmp::create(2));

    r = cache.get(k3);

    //now calc
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //we need B
    cache.set(k3, TL::Types::Demand::create({B}));

    //for #!0 > 0 we need B
    p.perturb(B, TL::Types::Intension::create(&bws));

    //again, calc
    r = cache.get(k3);
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //now we want the variable at 1
    {
      TL::ContextPerturber p2{k3, {{zero, TL::Types::Intmp::create(1)}}};

      TL::Constant at1 = cache.get(k3);
      REQUIRE(at1.index() == TL::TYPE_INDEX_INTMP);
      CHECK(cmp(TL::Types::Intmp::get(at1), 1) == 0);
    }

    //now we are ready to set its value
    cache.set(k3, TL::Types::Intmp::create(2));

    r = cache.get(k3);
    REQUIRE(r.index() == TL::TYPE_INDEX_INTMP);
    CHECK(cmp(TL::Types::Intmp::get(r), 2) == 0);
  }

  //round four, [0 <- 3]
  {
    TL::Context k4;

    //as before, we need d
    TL::Constant r = cache.get(k4);
    REQUIRE(r.index() == TL::TYPE_INDEX_DEMAND);
    CHECK(TL::Types::Demand::get(r).dims() ==
      std::set<TL::dimension_index>{d});

    //set dimension d
    TL::ContextPerturber p{k4};
    p.perturb(d, TL::Types::Dimension::create(zero));

    //get the value now
    r = cache.get(k4);
    REQUIRE(r.index() == TL::TYPE_INDEX_DEMAND);
    CHECK(TL::Types::Demand::get(r).dims() == 
      std::set<TL::dimension_index>({zero}));

    //set dimension zero to 2
    p.perturb(zero, TL::Types::Intmp::create(3));

    r = cache.get(k4);

    //now calc
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //we need B
    cache.set(k4, TL::Types::Demand::create({B}));

    //for #!0 > 0 we need B
    p.perturb(B, TL::Types::Intension::create(&bws));

    //again, calc
    r = cache.get(k4);
    CHECK(r.index() == TL::TYPE_INDEX_CALC);

    //now we want the variable at 2
    {
      TL::ContextPerturber p3{k4, {{zero, TL::Types::Intmp::create(2)}}};

      TL::Constant at1 = cache.get(k4);
      REQUIRE(at1.index() == TL::TYPE_INDEX_INTMP);
      CHECK(cmp(TL::Types::Intmp::get(at1), 2) == 0);
    }

    //now we are ready to set its value
    cache.set(k4, TL::Types::Intmp::create(3));

    r = cache.get(k4);
    REQUIRE(r.index() == TL::TYPE_INDEX_INTMP);
    CHECK(cmp(TL::Types::Intmp::get(r), 3) == 0);
  }
#endif
}
