// this computes powers by the squaring method
// there are three arrays used:
//
// P      = <x, x², x⁴, x⁸, ...>
// divs   = <n, n/2, n/4, n/8, ...>
// result = fold P whenever divs % 2 != 0
//
// the answer is just result as soon as divs is zero

fun fold_wvr.d.f.z X B = F
where
  var F = fby.d z if B then f!F!X else F fi ;;
end ;;

fun pow_by_square.x.n = //inresult @ [d <- 0]
  asa.d result (divs == 0)
where
  dim d <- 0 ;;
  var P = fby.d x (P*P) ;;
  var divs = fby.d n (divs/2) ;;
  var result = fold_wvr.d.times.1 P (divs % 2 != 0) ;;
end ;;

%%

pow_by_square.5.0 ;;
pow_by_square.5.1 ;;
pow_by_square.5.2 ;;
pow_by_square.5.3 ;;
pow_by_square.5.4 ;;
pow_by_square.5.5 ;;
pow_by_square.5.6 ;;
pow_by_square.5.10 ;;
