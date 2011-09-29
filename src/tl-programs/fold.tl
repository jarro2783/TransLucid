var fby = \d -> \\X -> \\Y -> if #d <= 0 then X else prev.d Y fi;;
var prev = \d -> \\X -> X @ [d <- #d - 1];;
var next = \d -> \\X -> X @ [d <- #d + 1];;

var foldlmd = \d -> \f -> \z -> \\x -> folds where
  var vals = fby.d z x;;
  var folds = fby.d (f.vals.(next.d vals)) (f.folds.(next.d (next.d vals)));;
end;;

var add = \x -> \y -> x + y;;

%%

foldlmd.0.add.0 (#0) @ [0 <- 1];;
