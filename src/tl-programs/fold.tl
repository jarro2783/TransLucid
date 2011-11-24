var foldL = \d -> \f -> \z -> \\x -> folds where
  var folds = fby.d z (f.folds.(next.d x));;
end;;

var add = \x -> \y -> x + y;;

%%

foldL.0.add.42 (#0) @ [0 <- 8];;
