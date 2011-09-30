var fby = \d -> \\X -> \\Y -> if #d <= 0 then X else prev.d Y fi;;
var prev = \d -> \\X -> X @ [d <- #d - 1];;
var next = \d -> \\X -> X @ [d <- #d + 1];;

var foldL = \d -> \f -> \z -> \\x -> folds where
  var folds = fby.d z (f.folds.(next.d x));;
end;;

var add = \x -> \y -> x + y;;

%%

foldL.0.add.42 (#0) @ [0 <- 8];;
