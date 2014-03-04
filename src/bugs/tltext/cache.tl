//This is broken with the cache, it appears to be not taking into account
//one of the relevant dimensions ... but which one?

// The upon function, B is assumed to be Boolean.
//fun upon.d X B = X @ [d <- T]
//var upon = \d -> \\X -> \\B -> X @ [d <- T]
//fun upon.d X B = X @ [d <- T]
//where
//  var T = fby.d 0 (if B then T+1 else T fi) ;;
//end ;;

var A = upon.0 (#.0) ((A + #.0) % 2 == 0);;

%%
A @ [0 <- 0];;
A @ [0 <- 1];;
A @ [0 <- 2];;
A @ [0 <- 3];;
A @ [0 <- 4];;
A @ [0 <- 5];;
A @ [0 <- 6];;
A @ [0 <- 7];;
A @ [0 <- 8];;
