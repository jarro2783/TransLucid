//some necessary dimensions
dim slot;;
dim envvar;;

//the basic operators
infixl "+" "plus" 100;;
infixl "-" "minus" 100;;
infixl "*" "times" 200;;
infixl "/" "divide" 200;;
infixl "%" "modulus" 200;;
infixl "<" "lt" 50;;
infixl "<=" "lte" 50;;
infixl ">" "gt" 50;;
infixl ">=" "gte" 50;;
infixl "==" "eq" 25;;
infixl "!=" "ne" 25;;
infixl "&&" "and" 20;;
infixl "||" "or" 20;;

infixl ".." "range_construct" 0;;

fun plus.a.b [a : intmp, b : intmp] = int_plus!(a,b);;
fun minus.a.b [a : intmp, b : intmp] = int_minus!(a,b);;
fun times.a.b [a : intmp, b : intmp] = int_times!(a,b);;
fun divide.a.b [a : intmp, b : intmp] = int_divide!(a,b);;
fun modulus.a.b [a : intmp, b : intmp] = int_modulus!(a,b);;

fun lte.a.b [a : intmp, b : intmp] = int_lte!(a,b);;
fun lt.a.b [a : intmp, b : intmp] = int_lt!(a,b);;
fun gte.a.b [a : intmp, b : intmp] = int_gte!(a,b);;
fun gt.a.b [a : intmp, b : intmp] = int_gt!(a,b);;
fun eq.a.b [a : intmp, b : intmp] = int_eq!(a,b);;
fun ne.a.b [a : intmp, b : intmp] = int_ne!(a,b);;

fun bool_and a b = if a then b else false fi;;
fun bool_or a b = if a then true else b fi;;

fun plus.a.b [a : ustring, b : ustring] = ustring_plus!(a,b);;

fun range_construct.a.b = make_range!(a,b);;

var fby = \d -> \\X -> \\Y -> if #!d <= 0 then X else prev.d Y fi;;
var index = \d -> #!d + 1;;
var prev = \d -> \\X -> X @ [d <- #!d - 1];;
var next = \d -> \\X -> X @ [d <- #!d + 1];;

//the print equations
var CANONICAL_PRINT [arg0 : intmp]   = PRINT;;
var CANONICAL_PRINT [arg0 : ustring] = "`" + PRINT + "`";;
var CANONICAL_PRINT [arg0 : uchar]   = "'" + PRINT + "'";;
var CANONICAL_PRINT [arg0 : bool]    = PRINT;;
var CANONICAL_PRINT [arg0 : range]   = PRINT;;
var CANONICAL_PRINT [arg0 : tuple]   = "I don't know how to print a tuple";;
var CANONICAL_PRINT                  = TYPENAME + "`" + PRINT + "`";;
