//some necessary dimensions
dim "slot";;
dim "envvar";;

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

//the print equations
var PRINT_TLTEXT [arg0 : intmp]   = PRINT;;
var PRINT_TLTEXT [arg0 : ustring] = "`" + PRINT + "`";;
var PRINT_TLTEXT [arg0 : uchar]   = "'" + PRINT + "'";;
var PRINT_TLTEXT [arg0 : bool]    = PRINT;;
var PRINT_TLTEXT [arg0 : range]   = PRINT;;
var PRINT_TLTEXT                  = TYPENAME + "`" + PRINT + "`";;

var CLARGS = "";;
