fun random_lcg.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

fun pow.n = g where
  dim d <- n;;
  var g = fby.d (\_ s -> 1) (\_ {d} s -> s * g.s);;
end;;

//shift X by n in direction d
fun shift.d.n X = X @ [d <- #.d + n] ;;

// The merge function, X and Y are assumed to be comparable.
fun merge.d X Y = if Xʹ <= Yʹ then Xʹ else Yʹ fi
where
  var Xʹ = upon.d X (Xʹ < Yʹ) ;;
  var Yʹ = upon.d Y (Yʹ <= Xʹ) ;;
end ;;

fun merge2.d X Y = if (X @ [d <- iX]) <= (Y @ [d <- iY]) 
  then X @ [d <- iX] 
  else Y @ [d <- iY] 
  fi
where
  var iX = upon.d (#.d) ((X @ [d <- iX]) <= (Y @ [d <- iY])) ;;
  var iY = upon.d (#.d) ((Y @ [d <- iY]) < (X @ [d <- iX])) ;;
end ;;

//merge sort n items of X in direction d
fun merge_sort.d.n X = Y
where
  dim a <- ilog.n ;;
  dim b <- 0 ;;
  var F = pow.(#.a).2 ;;
  var L = (default₁.d.0.(F-1).infty (add_dim.d.b.(F*2) Y)) ;;
  var R = (default₁.d.0.(F-1).infty (add_dim.d.b.(F*2) (shift.d.F Y))) ;;
  var Y = fby.a X (remove_dim.d.b.(F*2) (merge.d L R)) ;;
end;;

fun merge_sort_2.d.n X = Z
where
  dim a <- 0 ;;
  dim b <- ilog.n ;;

  var Z = fby.b 
    (default₂.d.0.0.a.0.(n-1).infty (rotate.d.a X))
    (merge.d (Z @ [a <- #.a * 2]) (Z @ [a <- #.a * 2 + 1])) ;;
end ;;

var A = #.0 ;;
var B = #.0 * 2 ;;

//var M2 = if (A @ [0 <- iX]) <= (B @ [0 <- iY]) 
//  then A @ [0 <- iX] 
//  else B @ [0 <- iY] 
//  fi ;;

var R = random_lcg.0!43 ;;

// X varies in dimension d₁, treat it as an array with an extra dimension
// in direction d₂, with length n in its original dimension
fun add_dim.d₁.d₂.n X = X @ [d₁ <- n * #.d₂ + #.d₁] ;;

// X varies in dimensions d₁ and d₂, treat it as an array of length n in
// direction d₁ and make it vary in only d₂
fun remove_dim.d₁.d₂.n X = X @ [d₁ <- #.d₁ % n, d₂ <- #.d₁ / n] ;;

var M = R % 50 ;;

var M0 = default₁.0.0.0.infty 
  (rotate.0.1 (default₁.0.0.9.infty (R % 50)))  ;;

var M1 = merge.0 (M0 @ [1 <- #.1 * 2]) (M0 @ [1 <- #.1 * 2 + 1]) ;;

var M2 = merge.0 (M1 @ [1 <- #.1 * 2]) (M1 @ [1 <- #.1 * 2 + 1]) ;;

var M3 = merge.0 (M2 @ [1 <- #.1 * 2]) (M2 @ [1 <- #.1 * 2 + 1]) ;;

var F = pow.(#.2).2;;
var Left = default₁.0.0.(F-1).infty (add_dim.0.1.(F*2) Y) ;;
var Right = default₁.0.0.(F-1).infty (add_dim.0.1.(F*2) (shift.0.F Y)) ;;
var Y = fby.2 M0 (remove_dim.0.1.(F*2) (merge.0 Left Right)) ;;

//var M1 = merge.0 (default₁.0.0.0.infty M0) 
//  (shift.0.1 (default₁.0.1.1.infty M0)) ;;

var D0 = (default₁.0.0.0.infty M0) ;;
var D1 = shift.0.1 (default₁.0.1.1.infty M0) ;;

var l1 = (default₁.0.0.0.infty (add_dim.0.1.2 M0)) ;;
var l2 = (default₁.0.0.0.infty (add_dim.0.1.2 (shift.0.1 M0))) ;;

%%

$$ %%

M @ [0 <- 0] ;;
M @ [0 <- 1] ;;
M @ [0 <- 2] ;;
M @ [0 <- 3] ;;
M @ [0 <- 4] ;;
M @ [0 <- 5] ;;
M @ [0 <- 6] ;;
M @ [0 <- 7] ;;
M @ [0 <- 8] ;;
M @ [0 <- 9] ;;

$$ %%

M0 @ [0 <- 0, 1 <- 0] ;;
M0 @ [0 <- 0, 1 <- 1] ;;
M0 @ [0 <- 0, 1 <- 2] ;;
M0 @ [0 <- 0, 1 <- 3] ;;
M0 @ [0 <- 0, 1 <- 4] ;;
M0 @ [0 <- 0, 1 <- 5] ;;
M0 @ [0 <- 0, 1 <- 6] ;;
M0 @ [0 <- 0, 1 <- 7] ;;

$$ %%

M1 @ [1 <- 0, 0 <- 0] ;;
M1 @ [1 <- 0, 0 <- 1] ;;
M1 @ [1 <- 1, 0 <- 0] ;;
M1 @ [1 <- 1, 0 <- 1] ;;
M1 @ [1 <- 2, 0 <- 0] ;;
M1 @ [1 <- 2, 0 <- 1] ;;
M1 @ [1 <- 3, 0 <- 0] ;;
M1 @ [1 <- 3, 0 <- 1] ;;

$$ %%

M2 @ [1 <- 0, 0 <- 0] ;;
M2 @ [1 <- 0, 0 <- 1] ;;
M2 @ [1 <- 0, 0 <- 2] ;;
M2 @ [1 <- 0, 0 <- 3] ;;
M2 @ [1 <- 1, 0 <- 0] ;;
M2 @ [1 <- 1, 0 <- 1] ;;
M2 @ [1 <- 1, 0 <- 2] ;;
M2 @ [1 <- 1, 0 <- 3] ;;

$$ %%

M3 @ [1 <- 0, 0 <- 0] ;;
M3 @ [1 <- 0, 0 <- 1] ;;
M3 @ [1 <- 0, 0 <- 2] ;;
M3 @ [1 <- 0, 0 <- 3] ;;
M3 @ [1 <- 0, 0 <- 4] ;;
M3 @ [1 <- 0, 0 <- 5] ;;
M3 @ [1 <- 0, 0 <- 6] ;;
M3 @ [1 <- 0, 0 <- 7] ;;

$$ %%

R @ [0 <- 0] ;;
R @ [0 <- 1] ;;
R @ [0 <- 2] ;;
R @ [0 <- 3] ;;

$$ %%

(merge_sort.0.10 (R % 50)) @ [0 <- 0] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 1] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 2] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 3] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 4] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 5] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 6] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 7] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 8] ;;
(merge_sort.0.10 (R % 50)) @ [0 <- 9] ;;

$$ %%

(merge_sort_2.0.10 (R % 50)) @ [0 <- 0] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 1] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 2] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 3] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 4] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 5] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 6] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 7] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 8] ;;
(merge_sort_2.0.10 (R % 50)) @ [0 <- 9] ;;

