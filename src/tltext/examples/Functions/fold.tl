fun foldl.d.f.z X = F
where
  var F = fby.d z (f ! F ! X) ;;
end ;;

fun reflect.d X = X @ [d <- uminus ! (#.d)] ;;

fun foldr.d.f.z X = 
  foldl.a.f.z 
    (fby.a z ((reflect.a Xa) @ [a <- #.a - #.d + 1]))
where
  dim a <- #.d ;;
  var Xa = rotate.d.a X ;;
end ;;

var sum = foldr.0.plus.0 (#.0) ;;

var reversed = reflect.0 (#.0) ;;

%%

reversed @ [0 <- 0] ;;
reversed @ [0 <- 1] ;;
reversed @ [0 <- 2] ;;
reversed @ [0 <- 3] ;;

sum @ [0 <- 0] ;;
sum @ [0 <- 1] ;;
sum @ [0 <- 2] ;;
sum @ [0 <- 3] ;;
sum @ [0 <- 4] ;;
sum @ [0 <- 5] ;;
sum @ [0 <- 6] ;;
sum @ [0 <- 7] ;;
