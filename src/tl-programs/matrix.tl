var fby = \d -> \\X -> \\Y -> if #d <= 0 then X else prev.d Y fi;;
var index = \d -> #d + 1;;
var prev = \d -> \\X -> X @ [d <- #d - 1];;
var next = \d -> \\X -> X @ [d <- #d + 1];;

var rotate = \d1 -> \d2 -> \\X -> X @ [d1 <- #d2];;

var multiply = \d_r -> \d_c -> \k -> \\X -> \\Y -> W where
  dim d <- 0;;
  var Xr = rotate.d_c.d X;;
  var Yr = rotate.d_r.d Y;;
  var Z = Xr * Yr;;
  var W = sum.d.k Z;;

end;;

var sum = \dx -> \n -> \\X -> Y @ [dx <- n - 1] where
  var Y = fby.dx X (Y + next.dx X);;
end;;

var A = #0 + #1;;
var B = 0 - A;;

%%

A @ [0 <- 2, 1 <- 5];;
B @ [0 <- 3, 1 <- 4];;
rotate.0.2 A @ [2 <- 1, 1 <- 1];;
sum.0.4 A @ [1 <- 0];;
multiply.0.1.2 A B @ [0 <- 1, 1 <- 1];;
