fun random_lrc.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

//maximum of X from 0..n in direction d
fun max_array.d.n X = M @ [d <- n] where
  var M = fby.d X (if X > M then X else M fi) ;;
end ;;

var weights = (random_lrc.0!0) % 25 ;;
var values = (random_lrc.0!1) % 40 ;;

fun knapsack_unbounded.d.n W V = K where
  dim a ;;
  var K = fby.d 0 (max_array.a.n I) ;;
  var I = upon.a ((V @ [d <- #.a]) + K @ [d <- W - (W @ [d <- #.a])]) 
    ((W @ [d <- #.a]) <= W) ;;
end ;;

%%

weights @ [0 <- 0] ;;
weights @ [0 <- 1] ;;
weights @ [0 <- 2] ;;
weights @ [0 <- 3] ;;

$$ %%

values @ [0 <- 0] ;;
values @ [0 <- 1] ;;
values @ [0 <- 2] ;;
values @ [0 <- 3] ;;

$$ %%

max_array.0.5 weights ;;

$$ %%

(knapsack_unbounded.0.2 weights values) @ [0 <- 10] ;;
(knapsack_unbounded.0.2 weights values) @ [0 <- 20] ;;
(knapsack_unbounded.0.2 weights values) @ [0 <- 30] ;;
