dim n;;
var fact [n:0] = 1 ;;
var fact [n:1..infty] = #!n * (fact @ [n <- #!n -1]) ;;
%%
fact @ [n <- 10] ;;

