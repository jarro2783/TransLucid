infixl "+" "operator+" 10;;
infixl "-" "operator-" 10;;
library "int";;
%%
fib | [1 : 0] = 0;;
fib | [1 : 1] = 1;;
fib = (fib @ [1 : #1 - 1]) + (fib @ [1 : #1 - 2]);;
%%
fib @ [1 : 0];;
fib @ [1 : 1];;
fib @ [1 : 2];;
fib @ [1 : 3];;
fib @ [1 : 5];;
fib @ [1 : 6];;
fib @ [1 : 7];;
fib @ [1 : 5] + fib @ [1 : 6];;
