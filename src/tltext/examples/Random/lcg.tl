fun random_lcg.d!seed = Rand where
  var Rand = fby.d seed ((1664525 * Rand + 1013904223) % 4294967296) ;;
end ;;

var R = (random_lcg.0!45) % 25 ;;

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
