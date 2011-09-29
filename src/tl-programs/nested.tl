var f = \x -> outer where
  dim d <- x;;
  var outer [d : 0] = 0;;
  var outer = inner + outer @ [d <- #d - 1] where
    dim e <- x;;
    var inner [e : 0] = 0;;
    var inner = 1 + inner @ [e <- #e - 1];;
  end;;
end;;

%%

f. 9;;
