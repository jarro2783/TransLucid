fun random_lcg.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

fun lower_bound.d.left.right.v X = asa.a () (X @ [d <- #.a + 1]
where
  dim a <- (right - left) / 2 + left ;;
  var pos = 
end ;;

fun insertion_sort.d.n X = sorted
where
  // #.a items are sorted at each row, then we put the next thing into the
  // sorted portion of the array
  dim a <- n ;;
  var sorted = fby.a infty (insert.d.v.insertPos sorted) ;;

  var v = X @ [d <- #.a] ;;

  var insertPos = fby.a 0 (lower_bound.d.0.(#.a).v sorted) ;;

  //find the position to insert and shift everything else
  fun insert.d.v.p X = shift
  where
    var shift [d : 0..(p-1)] = X ;;
    var shift [d is p] = v ;;
    var shift [d : (p+1) .. infty] = next.d X ;;
  end ;;
end ;;

var R = (random_lcg.0.45) 60 ;;

%%


