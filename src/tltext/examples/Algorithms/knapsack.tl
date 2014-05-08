fun random_lcg.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

//maximum of X from 0..n in direction d
fun max_wvr.d.n X B = M @ [d <- n + 1] where
  var M = fby.d 0 (if B && (X > M) then X else M fi) ;;
end ;;

fun fold_wvr.d.f.z X B = F
where
  var F = fby.d z if B then f!F!X else F fi ;;
end ;;

fun Max.d.n X = M @ [d <- n + 1] 
where
  var M = fby.d neginfty (if X > M then X else M fi) ;;
end ;;

var weights = (random_lcg.0!16) % 25 + 1 ;;
var values = (random_lcg.0!1) % 40 ;;

fun knapsack_unbounded.d.n W V = K where
  dim a <- 0 ;;
  var K = fby.d 0 (
      //max_wvr.a.n 
      //(Va + (K @ [d <- #.d - Wa ]) )
      //(Wa <= #.d)
      //Max.a.n (wvr.a (Va + (K @ [d <- #.d - Wa])) (Wa <= #.d))
      (fold_wvr.a.max.0 (Va + (K @ [d <- #.d - Wa ]) ) (Wa <= #.d))
        @ [a <- n + 1]
    ) ;;
  var Va = rotate.d.a V ;;
  var Wa = rotate.d.a W ;;
end ;;

var ws = #.0 + 1 ;;
var vs = #.0 + 2 ;;

//var K = if #.0 <= 0 then 0 else
//  max_wvr.2.1 
//  ((vs @ [0 <- #.2]) + (K @ [0 <- (#.0 - (ws @ [0 <- #.2])) ]) )
//  ((ws @ [0 <- #.2]) <= #.0)
//  fi ;;

var K2 = fby.0 0 (
  wvr.1 ((rotate.d.a weights) + K2 @ [0 <- #.0 - (rotate.d.a weights))
    ((rotate.d.a weights) <= #.d)
    ) ;;

var M = max_wvr.2.25
  (values @ [0 <- #.2])
  ((weights @ [0 <- #.2]) <= #.0) ;;

%%

weights @ [0 <- 0] ;;
weights @ [0 <- 1] ;;
weights @ [0 <- 2] ;;
weights @ [0 <- 3] ;;
weights @ [0 <- 4] ;;
weights @ [0 <- 5] ;;

$$ %%

values @ [0 <- 0] ;;
values @ [0 <- 1] ;;
values @ [0 <- 2] ;;
values @ [0 <- 3] ;;

$$ %%

max_wvr.0.5 weights (#.0 % 1 == 0) ;;
fold_wvr.0.max.neginfty weights (weights < #.0) @ [0 <- 0] ;;
fold_wvr.0.max.neginfty weights (weights < #.0) @ [0 <- 18] ;;
fold_wvr.0.max.neginfty weights (weights < #.0) @ [0 <- 19] ;;
fold_wvr.0.max.neginfty weights (weights < #.0) @ [0 <- 20] ;;

$$ %%

M @ [0 <- 2] ;;

$$ %%

K @ [0 <- 1] ;;
K @ [0 <- 2] ;;
K @ [0 <- 3] ;;

$$ %%

(knapsack_unbounded.0.1 weights values) @ [0 <- 1] ;;
//(knapsack_unbounded.0.1 (#.0 + 1) values) @ [0 <- 1] ;;

$$ %%

(knapsack_unbounded.0.2 weights values) @ [0 <- 20] ;;
(knapsack_unbounded.0.2 weights values) @ [0 <- 30] ;;
(knapsack_unbounded.0.30 weights values) @ [0 <- 50] ;;
