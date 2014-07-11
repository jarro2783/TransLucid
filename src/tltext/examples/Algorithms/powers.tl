fun foldl.d.f.z X = next.d F
where
  var F = fby.d z (f ! F ! X) ;;
end ;;

fun drop.d.n X = wvr.d X ((#.d + 1) % n != 0) ;;

fun powers.b!N = seq
where
  dim a <- N - 1 ;;
  var seq = fby.a posint (foldl.b.plus.0 (drop.b.n seq)) ;;
  var posint = #.b + 1 ;;
  var n = N - #.a ;;
end ;;

%%
(drop.0.4 (#.0)) @ [0 <- 3] ;;
(powers.0!4) @ [0 <- 0] ;;
(powers.0!4) @ [0 <- 1] ;;
(powers.0!4) @ [0 <- 2] ;;
(powers.0!4) @ [0 <- 3] ;;
(powers.0!4) @ [0 <- 4] ;;
