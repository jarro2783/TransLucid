fun random_lrc.d!seed = Rand where
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

fun rotate.a.b X = X @ [a <- #.b] ;;

fun merge_sort_2.d.n X = Y
where
  dim a <- 0 ;;
  dim b <- ilog.n ;;

  var Y = fby.b 
    (default₁.a.0.0.infty (rotate.d.a (default₁.d.0.(n-1).infty X)))
    (merge.d (Y @ [a <- #.a * 2]) (Y @ [a <- #.a * 2 + 1]))
end ;;

var A = #.0 ;;
var B = #.0 * 2 ;;

var M = merge.0 A B ;;
//var M2 = if (A @ [0 <- iX]) <= (B @ [0 <- iY]) 
//  then A @ [0 <- iX] 
//  else B @ [0 <- iY] 
//  fi ;;

var R = random_lrc.0!43 ;;

// X varies in dimension d₁, treat it as an array with an extra dimension
// in direction d₂, with length n in its original dimension
fun add_dim.d₁.d₂.n X = X @ [d₁ <- n * #.d₂ + #.d₁] ;;

// X varies in dimensions d₁ and d₂, treat it as an array of length n in
// direction d₁ and make it vary in only d₂
fun remove_dim.d₁.d₂.n X = X @ [d₁ <- #.d₁ % n, d₂ <- #.d₁ / n] ;;

var M0 = (random_lrc.0!43) % 25 ;;
var M1 = remove_dim.0.1.2 (merge.0 
  (default₁.0.0.0.infty (add_dim.0.1.2 M0))
  (default₁.0.0.0.infty (add_dim.0.1.2 (shift.0.1 M0)))
  ) ;;

var M2 = remove_dim.0.1.4 (merge.0 
  (default₁.0.0.1.infty (add_dim.0.1.4 M1))
  (default₁.0.0.1.infty (add_dim.0.1.4 (shift.0.2 M1)))
  ) ;;

var M3 = remove_dim.0.1.8 (merge.0 
  (default₁.0.0.3.infty (add_dim.0.1.8 M2))
  (default₁.0.0.3.infty (add_dim.0.1.8 (shift.0.4 M2)))
  ) ;;

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

Y @ [0 <- 0, 2 <- 0] ;;
Y @ [0 <- 1, 2 <- 0] ;;
Y @ [0 <- 2, 2 <- 0] ;;
Y @ [0 <- 3, 2 <- 0] ;;
Y @ [0 <- 4, 2 <- 0] ;;
Y @ [0 <- 5, 2 <- 0] ;;
Y @ [0 <- 6, 2 <- 0] ;;
Y @ [0 <- 7, 2 <- 0] ;;

Y @ [0 <- 0, 2 <- 1] ;;
Y @ [0 <- 1, 2 <- 1] ;;
Y @ [0 <- 2, 2 <- 1] ;;
Y @ [0 <- 3, 2 <- 1] ;;
Y @ [0 <- 4, 2 <- 1] ;;
Y @ [0 <- 5, 2 <- 1] ;;
Y @ [0 <- 6, 2 <- 1] ;;
Y @ [0 <- 7, 2 <- 1] ;;

Y @ [0 <- 0, 2 <- 2] ;;
Y @ [0 <- 1, 2 <- 2] ;;
Y @ [0 <- 2, 2 <- 2] ;;
Y @ [0 <- 3, 2 <- 2] ;;
Y @ [0 <- 4, 2 <- 2] ;;
Y @ [0 <- 5, 2 <- 2] ;;
Y @ [0 <- 6, 2 <- 2] ;;
Y @ [0 <- 7, 2 <- 2] ;;

Y @ [0 <- 0, 2 <- 3] ;;
Y @ [0 <- 1, 2 <- 3] ;;
Y @ [0 <- 2, 2 <- 3] ;;
Y @ [0 <- 3, 2 <- 3] ;;
Y @ [0 <- 4, 2 <- 3] ;;
Y @ [0 <- 5, 2 <- 3] ;;
Y @ [0 <- 6, 2 <- 3] ;;
Y @ [0 <- 7, 2 <- 3] ;;

$$ %%

M0 @ [0 <- 0] ;;
M0 @ [0 <- 1] ;;
M0 @ [0 <- 2] ;;
M0 @ [0 <- 3] ;;
M0 @ [0 <- 4] ;;
M0 @ [0 <- 5] ;;
M0 @ [0 <- 6] ;;
M0 @ [0 <- 7] ;;

$$ %%

l1 @ [0 <- 0, 1 <- 0] ;;
l2 @ [0 <- 0, 1 <- 0] ;;
l1 @ [0 <- 0, 1 <- 1] ;;
l2 @ [0 <- 0, 1 <- 1] ;;
l1 @ [0 <- 0, 1 <- 2] ;;
l2 @ [0 <- 0, 1 <- 2] ;;

$$ %%

M1 @ [0 <- 0] ;;
M1 @ [0 <- 1] ;;
M1 @ [0 <- 2] ;;
M1 @ [0 <- 3] ;;
M1 @ [0 <- 4] ;;
M1 @ [0 <- 5] ;;
M1 @ [0 <- 6] ;;
M1 @ [0 <- 7] ;;

$$ %%

M2 @ [0 <- 0] ;;
M2 @ [0 <- 1] ;;
M2 @ [0 <- 2] ;;
M2 @ [0 <- 3] ;;
M2 @ [0 <- 4] ;;
M2 @ [0 <- 5] ;;
M2 @ [0 <- 6] ;;
M2 @ [0 <- 7] ;;

$$ %%

M3 @ [0 <- 0] ;;
M3 @ [0 <- 1] ;;
M3 @ [0 <- 2] ;;
M3 @ [0 <- 3] ;;
M3 @ [0 <- 4] ;;
M3 @ [0 <- 5] ;;
M3 @ [0 <- 6] ;;
M3 @ [0 <- 7] ;;

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
(merge_sort.0.10 (R % 50)) @ [0 <- 10] ;;

