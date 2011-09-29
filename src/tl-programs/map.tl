var fby = \d -> \\X -> \\Y -> if #d <= 0 then X else prev.d Y fi;;
var index = \d -> #d + 1;;
var prev = \d -> \\X -> X @ [d <- #d - 1];;

var map2 = \f -> \\X -> \\Y -> f.X.Y;;
var add = \x -> \y -> x + y;;
var mul = \x -> \y -> x * y;;
var squares = #0*#0;;
var cubes = #0*#0*#0;;
%%
map2.add (#0) (#1) @ [0 <- 5, 1 <- 6];;
map2.mul squares cubes @ [0 <- 3];;
