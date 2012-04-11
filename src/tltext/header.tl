//some necessary dimensions
dim slot;;
dim envvar;;

data Infinite = infty | neginfty;;

var sperror = special"sperror";;
var spaccess = special"spaccess";;
var sptypeerror = special"sptypeerror";;
var spdim = special"spdim";;
var sparith = special"sparith";;
var spundef = special"spundef";;
var spconst = special"spconst";;
var spmultidef = special"spmultidef";;
var sploop = special"sploop";;

fun bestselect__.a.b = spmultidef;;

data Assoc = AssocNon | AssocLeft | AssocRight;;
data HostDecl = HostDim intmp | HostType intmp | HostFunc intmp;;
data OpType = OpPostfix ustring bool
            | OpPrefix ustring bool
            | OpInfix ustring bool Assoc intmp
            ;;
data Hyperdaton = HdIn ustring | HdOut ustring;;

op + = OpInfix."plus".false.AssocLeft.100;;
op - = OpInfix."minus".false.AssocLeft.100;;
op * = OpInfix."times".false.AssocLeft.200;;
op / = OpInfix."divide".false.AssocLeft.200;;
op % = OpInfix."modulus".false.AssocLeft.200;;
op < = OpInfix."lt".false.AssocNon.50;;
op <= = OpInfix."lte".false.AssocNon.50;;
op > = OpInfix."gt".false.AssocNon.50;;
op >= = OpInfix."gte".false.AssocNon.50;;
op == = OpInfix."eq".false.AssocNon.25;;
op != = OpInfix."ne".false.AssocNon.25;;
op && = OpInfix."bool_and".false.AssocLeft.20;;
op || = OpInfix."bool_and".false.AssocLeft.20;;

op .. = OpInfix."range_construct".false.AssocLeft.0;;

var pos = 1..infty;;
var nat = 0..infty;;
var neg = neginfty.. ~1;;
var nonpos = neginfty..0;;
var int = neginfty..infty;;

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

fun eq.a.b [a : bool, b : bool] = bool_eq!(a,b);;

fun bool_and.a.b = if a then b else false fi;;
fun bool_or.a.b = if a then true else b fi;;

fun plus.a.b [a : ustring, b : ustring] = ustring_plus!(a,b);;

fun range_construct.a.b [a : intmp, b : intmp] = make_range!(a,b);;
fun range_construct.a.b [a : intmp, b : infty] = make_range_infty!a;;
fun range_construct.a.b [a : neginfty, b : intmp] = make_range_neginfty!b;;
fun range_construct.a.b [a : neginfty, b : infty] = make_range_infinite;;

var fby = \d -> \\X -> \\Y -> if #!d <= 0 then X else prev.d Y fi;;
var index = \d -> #!d + 1;;
var prev = \d -> \\X -> X @ [d <- #!d - 1];;
var next = \d -> \\X -> X @ [d <- #!d + 1];;

fun escape_character.c | is_printable!c = print.c;;
fun escape_character.c | is_printable!c == false && code_point!c <= 0GFFFF 
  = `\u` + code_point_4!c;;
fun escape_character.c | is_printable!c == false && code_point!c > 0GFFFF 
  = `\U` + code_point_8!c;;
fun escape_character.c [c : '\n'] = `\n`;;
fun escape_character.c [c : '\t'] = `\t`;;
fun escape_character.c [c : '\r'] = `\r`;;
fun escape_character.c [c : '\\'] = `\\`;;
fun escape_character.c [c : '\"'] = `\"`;;
fun escape_character.c [c : '\''] = `\'`;;

fun string_at.s.pos [s : ustring, pos : intmp] = string_at_base!(s,pos);;
fun substring.s.start.length [s : ustring, start : intmp, length : intmp]
  = substring_base!(s, start, length);;
fun substring.s.start.length [s : ustring, start : intmp, length : infty]
  = substring_toend_base!(s, start);;

fun escape_string.s = escape_character.(string_at.s.0)
  + escape_string.(substring.s.1.infty)
;;
fun escape_string.s [s : ""] = "";;

fun print_special.c [c : sperror] = "sperror";;
fun print_special.c [c : spaccess] = "spaccess";;
fun print_special.c [c : sptypeerror] = "typeerror";;
fun print_special.c [c : spdim] = "spdim";;
fun print_special.c [c : sparith] = "sparith";;
fun print_special.c [c : spundef] = "spundef";;
fun print_special.c [c : spconst] = "spconst";;
fun print_special.c [c : spmultidef] = "spmultidef";;
fun print_special.c [c : sploop] = "sploop";;

fun print.c = "I don't know how to print this type";;
fun print.c [c : ustring] = c;;
fun print.c [c : intmp] = print_intmp!c;;
fun print.c [c : bool] = print_bool!c;;
fun print.c [c : special] = print_special.c;;
fun print.c [c : typetype] = print_typetype!c;;
fun print.c [c : uchar] = print_uchar!c;;
fun print.c [c : range] = print_range!c;;
fun print.c [c : tuple] = print_tuple!c;;
fun print.c [c : uuid] = print_uuid!c;;
fun print.c [c : demand] = "Cannot print demands";;
fun print.c [c : calc] = "Cannot print calc";;

fun print_typename.c = "unknown typename";;
fun print_typename.c [c : intmp] = "intmp";;
fun print_typename.c [c : ustring] = "ustring";;
fun print_typename.c [c : uchar] = "uchar";;
fun print_typename.c [c : bool] = "bool";;
fun print_typename.c [c : range] = "range";;
fun print_typename.c [c : tuple] = "tuple";;
fun print_typename.c [c : special] = "special";;
fun print_typename.c [c : typetype] = "typetype";;
fun print_typename.c [c : uuid] = "uuid";;
fun print_typename.c [c : demand] = "demand";;
fun print_typename.c [c : calc] = "calc";;

fun construct_literal.t.v [t : "intmp"] = construct_intmp!v;;
fun construct_literal.t.v [t : "special"] = construct_special!v;;
fun construct_literal.t.v [t : "typetype"] = construct_typetype!v;;
fun construct_literal.t.v [t : "uuid"] = construct_uuid!v;;

//the print equations
fun canonical_print.c [c : intmp]   = print.c;;
fun canonical_print.c [c : ustring] = `"` + escape_string.(print.c) + `"`;;
fun canonical_print.c [c : uchar]   = "'" + escape_string.(print.c) + "'";;
fun canonical_print.c [c : bool]    = print.c;;
fun canonical_print.c [c : range]   = print.c;;
fun canonical_print.c [c : tuple]   = "[I don't know how to print a tuple]";;
//fun canonical_print.c               = print_typename.c + `"` + 
//                                      escape_string.(print.c) + `"`;;
fun canonical_print.c = "cannot print this thing";;
fun canonical_print.c [c : special] = print.c;;

var RETURN = 0;;

// The first function.
fun first.d X = X @ [d <- 0] ;;

// The whenever function, B is assumed to be Boolean.
fun wvr.d X B = X @ [d <- T]
where
  var T = fby.d U (U @ [d <- T + 1]) ;;
  var U = if B then #!d else next.d U fi ;;
end ;;

// The upon function, B is assumed to be Boolean.
fun upon.d X B = X @ [d <- T]
where
  var T = fby.d 0 (if B then T+1 else T fi) ;;
end ;;

// The merge function, X and Y are assumed to be comparable.
fun merge.d X Y = if Xʹ <= Yʹ then Xʹ else Yʹ fi
where
  var Xʹ = upon.d X (Xʹ <= Yʹ) ;;
  var Yʹ = upon.d Y (Yʹ <= Xʹ) ;;
end ;;

// The as-soon-as function, B is assumed to be Boolean.
fun asa.d X B = first.d (wvr.d X B) ;;

// Rotating and transposing.
fun rotate.d.d′ X = X @ [d <- #!d′] ;;
fun transpose.d.d′ X = X @ [d <- #!d′, d′ <- #!d] ;;

fun firstOfPair.d X = X @ [d <- #!d * 2] ;;
fun secondOfPair.d X = X @ [d <- #!d * 2 + 1] ;;
