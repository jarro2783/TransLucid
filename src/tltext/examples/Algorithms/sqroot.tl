fun sqroot.x = asa.d approx (err < floatmp"0.00001")
where
  dim d <- 0 ;; 
  var approx = fby.d (X / floatmp"2") ((approx + X/approx) / floatmp"2") ;;
  var err = abs ! (approx * approx - X) ;;
end ;;

fun avg.d X = mean
where
  var n = convert ! floatmp ! (#.d + 1) ;;
  var mean = fby.d X (mean + a) ;;
  var a = (next.d X - mean) / (n + floatmp"1") ;;
end ;;

var S = sqroot.(#.0) ;;

fun rms.d X = sqroot.(avg.d (X*X)) ;;

%%
S @ [0 <- floatmp"100"] ;;
S @ [0 <- floatmp"90"] ;;
S @ [0 <- floatmp"2"] ;;
avg.0 (convert!floatmp!(#.0)) @ [0 <- 5] ;;
rms.0 (convert!floatmp!(#.0)) @ [0 <- 5] ;;
