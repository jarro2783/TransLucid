fun fact.n = F
where
  dim d <- n ;;
  var F [d : 0] = 1 ;;
  var F [d : nat] = #.d * prev.d F ;;
end ;;

fun inf!a!b [a is neginfty, b imp intmp] = true ;;
fun inf!a!b [a imp intmp, b imp intmp] = false ;;

var Hashzero = #.0 ;;

%%
Hashzero ;;
Hashzero @ [0 <- 0] ;;
\a -> \b -> \c -> a + b + c ;;
fby ;;
wvr ;;
upon ;;
upon.0 Hashzero (#.0 % 2 == 0) @ [0 <- 5] ;;
fby.0 ;;
fby.0 0 1;;
fby.0 0 Hashzero ;;
\\f -> (\\x -> f (x x)) (\\x -> f (x x)) ;;
default₁ ;;
fact ;;
inf ;;
