dim m;;
dim n;;
var fortytwo = 42 ;;
var sum = #!m + #!n ;;
var fact = if #!n == 0 then 1
else #!n * (fact @ [n <- #!n -1]) fi ;;
%%
fortytwo ;;
fortytwo @ [n <- 10] ;;
sum ;;
sum @ [m <- 3, n <- 5] ;;
fact @ [n <- 10] ;;

