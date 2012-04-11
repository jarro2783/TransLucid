// This file should bomb on the last 7 outputs.

dim a ;;
dim b ;;
var m = 4 ;;
var g [a : 0, b : 0 ] = 1 ;;
var g [a : 0, b : 1..m ] = #!b ;;
var g [a : 0, b : m+1 .. infty ] = 1 ;;
var g [a : pos, b : nat] = prev.a (firstOfPair.d g * secondOfPair.b g) ;;
%%

$$ %%

g @ [a <- 0 , b <- 0] ;;
g @ [a <- 0 , b <- 1] ;;
g @ [a <- 0 , b <- 2] ;;
g @ [a <- 0 , b <- 3] ;;
g @ [a <- 0 , b <- 4] ;;
g @ [a <- 0 , b <- 5] ;;
g @ [a <- 0 , b <- 6] ;;
g @ [a <- 0 , b <- 7] ;;

$$ %%

g @ [a <- 1 , b <- 0] ;;
g @ [a <- 1 , b <- 1] ;;
g @ [a <- 1 , b <- 2] ;;
g @ [a <- 1 , b <- 3] ;;

$$ %%

g @ [a <- 2 , b <- 0] ;;
g @ [a <- 2 , b <- 1] ;;

$$ %%

g @ [a <- 3 , b <- 0] ;;
