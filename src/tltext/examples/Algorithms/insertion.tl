fun random_lcg.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

//find the first position for which X is not less than v between l and r
fun lower_bound.d.l.r.v X = asa.a left (right == left)
where
  dim a <- #.d ;;
  var left = fby.a l (if (X @ [d <- middle]) < v then middle + 1 
    else left fi) ;;
  var right = fby.a r (if (X @ [d <- middle]) < v then right else middle fi) ;;
  var middle = (left + right) / 2 ;;
end ;;

//find the position to insert and shift everything else
fun insert.d.v.p X = if #.d < p then X elsif #.d == p then v 
  else prev.d X fi ;;

fun insertion_sort.d.n X = sorted
where
  // #.a items are sorted at each row, then we put the next thing into the
  // sorted portion of the array
  dim a <- n ;;
  var sorted = fby.a infty (insert.d.v.insertPos sorted) ;;

  var v = X @ [d <- #.a] ;;

  var insertPos = lower_bound.d.0.(#.a + 1).v sorted ;;
end ;;

var R = (random_lcg.0!45) % 60 ;;

var s = insertion_sort.0.10 R ;;

var bound = lower_bound.0.0.20.2 (#.0 + 5) ;;

var inserted = insert.0.42.2 (#.0) ;;

%%

bound @ [0 <- 0] ;;

$$ %%

R @ [0 <- 0] ;;
R @ [0 <- 1] ;;
R @ [0 <- 2] ;;
R @ [0 <- 3] ;;
R @ [0 <- 4] ;;
R @ [0 <- 5] ;;
R @ [0 <- 6] ;;
R @ [0 <- 7] ;;
R @ [0 <- 8] ;;
R @ [0 <- 9] ;;

$$ %%

s @ [0 <- 0] ;;
s @ [0 <- 1] ;;
s @ [0 <- 2] ;;
s @ [0 <- 3] ;;
s @ [0 <- 4] ;;
s @ [0 <- 5] ;;
s @ [0 <- 6] ;;
s @ [0 <- 7] ;;
s @ [0 <- 8] ;;
