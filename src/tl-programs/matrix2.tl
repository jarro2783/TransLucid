in A = file_array_in_hd ! "inlhs";;
in B = file_array_in_hd ! "inrhs";;
out C = file_array_out_hd ! ("C", 20, 20);;

var multiply = \d_r -> \d_c -> \k -> \\X -> \\Y -> W where
  dim d <- 0;;
  var Xr = rotate.d_c.d X;;
  var Yr = rotate.d_r.d Y;;
  var Z = Xr * Yr;;
  var W = sum.d.k Z;;

  var rotate = \d1 -> \d2 -> \\X -> X @ [d1 <- #!d2];;
  var sum = \dx -> \n -> \\X -> Y @ [dx <- n - 1] where
    var Y = fby.dx X (Y + next.dx X);;
  end;;
end;;

assign C [arg0 : 0..19, arg1 : 0..19, time : 0] := 
  multiply.arg0.arg1.19 A B;;
