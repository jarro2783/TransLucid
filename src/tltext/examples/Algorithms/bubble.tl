fun random_lrc.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

fun sort_swap.d.m.n X = A where
  var A [d is m] = if X > (X @ [d <- n]) then X @ [d <- n] else X fi ;;
  var A [d is n] = if X < (X @ [d <- m]) then X @ [d <- m] else X fi ;;
  var A = X ;;
end ;;

fun bubble_sort.d.n X = A where
  dim a <- n - 1 ;;
  dim b <- n - 1 ;;

  var A = fby.b X 
    (fby.a (A @ [a <- n - 1]) (next.b (sort_swap.d.(#.a).(#.a + 1) A)) ) ;;
end ;;

fun bubble_sort_faster.d.n X = A where
  dim a <- 0 ;;
  dim b <- n - 1 ;;

  var A = fby.b X 
    (fby.a (A @ [a <- n - #.a - 1]) (next.b (sort_swap.d.(#.a).(#.a + 1) A)) ) 
    ;;
end ;;

var R = (random_lrc.0!45) % 26 ;;

var S = bubble_sort.0.4 R ;;

var S2 = bubble_sort_faster.0.4 R ;;

var swapped = sort_swap.0.1.2 R ;;

%%

swapped @ [0 <- 0] ;;
swapped @ [0 <- 1] ;;
swapped @ [0 <- 2] ;;
swapped @ [0 <- 3] ;;

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

$$ %%

S2 @ [0 <- 0] ;;
S2 @ [0 <- 1] ;;
S2 @ [0 <- 2] ;;
S2 @ [0 <- 3] ;;
