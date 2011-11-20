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
fun plus.a.b [a : ustring, b : ustring] = string_plus!(a,b);;

//the print equations
var PRINT_TLTEXT [arg0 : intmp]   = PRINT;;
var PRINT_TLTEXT [arg0 : ustring] = "`" + PRINT + "`";;
var PRINT_TLTEXT [arg0 : uchar]   = "'" + PRINT + "'";;
var PRINT_TLTEXT [arg0 : bool]    = PRINT;;
var PRINT_TLTEXT [arg0 : range]   = PRINT;;
var PRINT_TLTEXT [arg0 : tuple]   = "I don't know how to print a tuple";;
var PRINT_TLTEXT                  = TYPENAME + "`" + PRINT + "`";;
