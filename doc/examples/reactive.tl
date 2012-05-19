//tltext -i reactive.tl

var x = 1;;
%%
#!time;;
x;;
$$
var x = 2;;
$$
%%
#!time;;
x;;
x @ [time <- 0];;

