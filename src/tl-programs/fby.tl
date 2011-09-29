var fby = \d -> \\X -> \\Y -> if #d <= 0 then X else prev.d Y fi;;
var index = \d -> #d + 1;;
var prev = \d -> \\X -> X @ [d <- #d - 1];;

var fact = \m -> f where
  dim d <- m;;
  var f = fby.d 1 (index.d * f);;
end;;
%%
fby.n 0 1 @ [n <- 0];;
fby.n 0 1 @ [n <- 1];;
fby.n 0 1 @ [n <- 2];;
fact.10;;
