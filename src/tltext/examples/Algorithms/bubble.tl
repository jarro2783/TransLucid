fun random_lcg.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

fun at.d.n X = X @ [d <- n] ;;

fun swap.d.m.n X = 
  if #.d == m 
  then 
    X @ [d <- n] 
  elsif #.d == n 
  then 
    X @ [d <- m]
  else
    X
  fi
;;

fun bubble_sort.d.n X = A where
  dim a <- n - 1 ;;
  dim b <- n - 1 ;;

  var A = fby.b X 
    (fby.a (A @ [a <- n - 1]) (next.b 
      if at.d.(#.a) A > at.d.(#.a + 1) A then swap.d.(#.a).(#.a + 1) A
      else A
      fi
      ) ) ;;
end ;;

fun bubble_sort_faster.d.n X = A where
  dim a <- 0 ;;
  dim b <- n - 1 ;;

  var A = fby.b X 
    (fby.a (A @ [a <- n - #.b]) (next.b (sort_swap.d.(#.a).(#.a + 1) A)) ) 
    ;;
end ;;

var R = (random_lcg.0!45) % 26 ;;

var S = bubble_sort.0.10 R ;;

var S2 = bubble_sort_faster.0.4 R ;;

%%

$$ %%

R @ [0 <- 0] ;;
R @ [0 <- 1] ;;
R @ [0 <- 2] ;;
R @ [0 <- 3] ;;

$$ %%

S @ [0 <- 0] ;;
S @ [0 <- 1] ;;
S @ [0 <- 2] ;;
S @ [0 <- 3] ;;
S @ [0 <- 4] ;;
S @ [0 <- 5] ;;
S @ [0 <- 6] ;;
S @ [0 <- 7] ;;
S @ [0 <- 8] ;;
S @ [0 <- 9] ;;

$$ %%

S2 @ [0 <- 0] ;;
S2 @ [0 <- 1] ;;
S2 @ [0 <- 2] ;;
S2 @ [0 <- 3] ;;
