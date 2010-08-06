//this example doesn't work yet

dimension "arg0";;
dimension "arg1";;

dimension "begin";;
dimension "end";;
library "int";;
infixl "+" "operator+" 1;;
%%
member = (#arg1) @ #arg0;;
addlist | [begin : #end] = 0;;
addlist = (member @ [2 : #begin]) + (addlist @ [begin : (#begin) + 1]);;
%%
addlist @ [arg0 : [1 : 1, 2 : 2, 3 : 3, 4 : 4], begin : 1, end : 5];;
