//tltext -i reactive.tl
// This should work, but ... it does not.

var x = 1;;
%%
#time;;
x;;
$$
var x = 2;;
$$
%%
#time;;
x;;
x @ [time <- 0];;

