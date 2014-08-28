//in-place quick sort

fun swap.a.m.n X = if m == n then X else Y fi
where
  var Y [a is m] = X @ [a <- n] ;;
  var Y [a is n] = X @ [a <- m] ;;
  var Y = X ;;
end ;;

fun quick_sort.d.m.n X = //partition
if #.d < m || #.d >= n then X else result fi
where
  dim a <- n - m - 2 ;;
  var pivot = X @ [d <- pivoti] ;;
  var pivoti = (n - m) / 2 + m ;;
  var doswap = (P @ [d <- current]) < pivot ;;
  var swapindex = upon.a (#.a + m) doswap ;;
  var current = (#.a + m)  ;;

  var P = fby.a (swap.d.pivoti.(n-1) X) 
    (if doswap then swap.d.current.swapindex P else P fi) ;;
  
  var result = if n - m < 2 then X else glued fi ;;

  var partition = swap.d.(n-1).swapindex P ;;

  var glued = if #.d < swapindex 
    then 
      (quick_sort.d.m.swapindex!(↑ {a} partition))
    else
      (quick_sort.d.(swapindex + 1).n!(↑ {a} partition)) 
    fi ;;
end ;;

fun random_lcg.d!seed = Rand 
where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

var R = (random_lcg.0!89) % 51 ;;

var sorted = quick_sort.0.0.6 R ;;

var small = quick_sort.0.0.3 R ;;

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
sorted @ [0 <- 4] ;;
sorted @ [0 <- 5] ;;
//sorted @ [0 <- 6] ;;
//sorted @ [0 <- 7] ;;
//sorted @ [0 <- 8] ;;
//sorted @ [0 <- 9] ;;
