// This file produces spundef for all non-1 cells.

dim a ;;
dim b ;;
var m = 4 ;;
var g [a : 0, b : nat ] = 1 ;;
var g [a : 0, b : 1..m ] = #!b ;;
var g [a : pos, b : nat] = prev.a (firstOfPair.b g * secondOfPair.b g) ;;
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
