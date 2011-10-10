dim "type";;
dim "cons";;

//we would like syntactic sugar for the below where clause
var print_tree [0 : [type <- "tree", cons <- "at"]] =
  print_tree @ [0 <- lhs] + " @ " + print_tree @ [0 <- rhs]
where
  var lhs = #0!arg0;;
  var rhs = #0!arg1;;
end  
;;

var print_tree [0 : [type <- "tree", cons <- "lambdaApp"]] =
  print_tree @ [0 <- #0!arg0] + "." + print_tree @ [0 <- #0!arg1];;

var print_tree [0 : intmp] = PRINT @ [arg0 <- #0];;
var print_tree [0 : ustring] = `"` + #0 + `"`;;
%%
print_tree @ [0 <- [type <- "tree", cons <- "at", 
  arg0 <- [type <- "tree", cons <- "lambdaApp", arg0 <- "f", arg1 <- 42],
  arg1 <- "some tuple"
]];;
