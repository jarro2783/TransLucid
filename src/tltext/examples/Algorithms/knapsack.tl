fun random_lrc.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

//maximum of X from 0..n in direction d
fun max_wvr.d.n X B = M @ [d <- n + 1] where
  var M = fby.d 0 (if B && (X > M) then X else M fi) ;;
end ;;

var weights = (random_lrc.0!16) % 25 + 1 ;;
var values = (random_lrc.0!1) % 40 ;;

fun knapsack_unbounded.d.n W V = K where
  dim a <- 0 ;;
  var K = if #.d <= 0 then 0 else
    max_wvr.a.n 
    ((V @ [d <- #.a]) + (K @ [d <- #.d - (W @ [d <- #.a]) ]) )
    ((W @ [d <- #.a]) <= #.d)
    fi ;;
end ;;

var ws = #.0 + 1 ;;
var vs = #.0 + 2 ;;

var K = if #.0 <= 0 then 0 else
  max_wvr.2.1 
  ((vs @ [0 <- #.2]) + (K @ [0 <- (#.0 - (ws @ [0 <- #.2])) ]) )
  ((ws @ [0 <- #.2]) <= #.0)
  fi ;;

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
