fun swap.a.m.n X = if m == n then X else Y fi
where
  var Y [a is m] = X @ [a <- n] ;;
  var Y [a is n] = X @ [a <- m] ;;
  var Y = X ;;
end ;;

fun glue.d X Y = if #.d < n then X else Y @ [d <- #.d - n] fi
where
  var n = asa.d (#.d) (is_eod.X) ;;
end ;;

fun quick_sort.d.n X = result //asa.a partition (current == upper) //result
where
  dim a <- 0 ;;
  var pivot = X @ [d <- pivoti] ;;
  var pivoti = (n - 1) / 2 ;;
  var swaplower = (P @ [d <- current]) < pivot ;;
  var upper = upon.a (n - #.a - 2) (not!swaplower) ;;
  var current = upon.a (#.a) swaplower ;;

  var P = fby.a (swap.d.pivoti.(n-1) X) (if swaplower then P else 
    swap.d.current.upper P fi) ;;
  
  var result = default₁.d.0.(n-1).EOD 
    if n < 2 then X 
    elsif n == 2 then
      if X @ [d <- 0] > X @ [d <- 1] then swap.d.0.1 X else X fi
    else
      asa.a glued (current == upper)
    fi ;;

  //var partition = swap.d.pivoti.current (asa.a P (current == upper)) ;;
  var partition = swap.d.(n-1).(current) P ;;

  var glued = glue.d 
    (quick_sort.d.(upper) partition) 
    (quick_sort.d.(n - upper) (translate.d.(upper) partition)) 
  ;;
end ;;

fun random_lcg.d!seed = Rand 
where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

var R = (random_lcg.0!89) % 51 ;;

var sorted = quick_sort.0.4 R ;;

var small = quick_sort.0.3 R ;;

%%

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
R @ [0 <- 11] ;;
R @ [0 <- 12] ;;
R @ [0 <- 13] ;;
R @ [0 <- 14] ;;

$$%%

small @ [0 <- 0] ;;
small @ [0 <- 1] ;;
small @ [0 <- 2] ;;

$$%%

sorted @ [0 <- 0] ;;
sorted @ [0 <- 1] ;;
sorted @ [0 <- 2] ;;
sorted @ [0 <- 3] ;;
//sorted @ [0 <- 4] ;;
//sorted @ [0 <- 5] ;;
//sorted @ [0 <- 6] ;;
//sorted @ [0 <- 7] ;;
//sorted @ [0 <- 8] ;;
//sorted @ [0 <- 9] ;;
