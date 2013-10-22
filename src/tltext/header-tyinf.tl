//a modified header for type inference, there is no ad-hoc polymorphism

//some necessary dimensions
dim slot;;
dim envvar;;

data Infinite;;
constructor infty = Infinite;;
constructor neginfty = Infinite;;

var sperror = construct_special."sperror";;
var spaccess = construct_special."spaccess";;
var sptypeerror = construct_special."sptypeerror";;
var spdim = construct_special."spdim";;
var sparith = construct_special."sparith";;
var spundef = construct_special."spundef";;
var spconst = construct_special."spconst";;
var spmultidef = construct_special."spmultidef";;
var sploop = construct_special."sploop";;

//fun bestselect__.a.b = spmultidef;;
fun special_combine.a.b = a;;

data Assoc;;
constructor AssocNon = Assoc;;
constructor AssocLeft = Assoc;;
constructor AssocRight = Assoc;;

data HostDecl;;
constructor HostDim a [a imp intmp] = HostDecl;;
constructor HostType a [a imp intmp] = HostDecl;;
constructor HostFunc a [a imp intmp] = HostDecl;;

data OpType;;
constructor OpPostfix a b [a imp ustring, b imp bool] = OpType;;
constructor OpPrefix a b [a imp ustring, b imp bool] = OpType;;
constructor OpInfix a b c d //[a : ustring, b : bool, c : Assoc, d : intmp]
  = OpType;;

data Hyperdaton;;
constructor HdIn a [a imp ustring] = Hyperdaton;;
constructor HdOut a [a imp ustring] = Hyperdaton;;

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
op && = OpInfix."bool_and".true.AssocLeft.20;;
op || = OpInfix."bool_or".true.AssocLeft.15;;
op ¬ = OpPrefix."negate".false ;;

op >> = OpInfix."concatenate".false.AssocLeft.100 ;;

op .. = OpInfix."range_construct".false.AssocLeft.0;;

op ∪ = OpInfix."union_construct".false.AssocLeft. ~100;;
//op \/ = OpInfix."union_construct".false.AssocLeft.~100;;

op √ = OpPrefix."sqrt".false;;

//var sint = sint8 ∪ sint16 ∪ sint32 ∪ sint64;;
//var uint = uint8 ∪ uint16 ∪ uint32 ∪ uint64;;
//var fint = sint ∪ uint;;
//var float = float32 ∪ float64;;
//var numeric = float ∪ int;;

var pos = 1..infty;;
var nat = 0..infty;;
var neg = neginfty .. ~1;;
var nonpos = neginfty..0;;
//var int = neginfty..infty;;

fun negate!a = if a then false else true fi;;

//fun plus.a.b [a : intmp, b : intmp] = int_plus!(a,b);;
fun plus!a!b [a imp intmp, b imp intmp] = intmp_plus.(a,b);;
//fun plus!a!b [a imp floatmp, b imp floatmp] = floatmp_plus.(a,b);;
//fun plus!a!b [a : fint, b : fint] 
//  | a.prec == b.prec && a.is_signed == b.is_signed
//  = (fint_plus!(a.prec)!(a.is_signed)).(a,b);;

fun minus!a!b [a imp intmp, b imp intmp] = intmp_minus.(a,b);;
//fun minus!a!b [a imp floatmp, b imp floatmp] = floatmp_minus.(a,b);;
//fun minus!a!b [a : fint, b : fint] 
//  | a.prec == b.prec && a.is_signed == b.is_signed
//  = (fint_minus!(a.prec)!(a.is_signed)).(a,b);;

fun times!a!b [a imp intmp, b imp intmp] = intmp_times.(a,b);;
//fun times!a!b [a imp floatmp, b imp floatmp] = floatmp_times.(a,b);;
//fun times!a!b [a : fint, b : fint] 
//  | a.prec == b.prec && a.is_signed == b.is_signed
//  = (fint_times!(a.prec)!(a.is_signed)).(a,b);;

fun divide!a!b [a imp intmp, b imp intmp] = intmp_divide.(a,b);;
//fun divide!a!b [a imp floatmp, b imp floatmp] = floatmp_divide.(a,b);;
//fun divide!a!b [a : fint, b : fint] 
//  | a.prec == b.prec && a.is_signed == b.is_signed
//  = (fint_divide!(a.prec)!(a.is_signed)).(a,b);;

fun modulus!a!b [a imp intmp, b imp intmp] = intmp_modulus.(a,b);;
//fun modulus!a!b [a : fint, b : fint] 
//  | a.prec == b.prec && a.is_signed == b.is_signed
//  = (fint_modulus!(a.prec)!(a.is_signed)).(a,b);;

fun sqrt!a [a imp floatmp] = floatmp_sqrt.a;;

//fun plus!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_plus!(a.prec)).(a,b);;

//fun minus!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_minus!(a.prec)).(a,b);;

//fun times!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_times!(a.prec)).(a,b);;

//fun divide!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_divide!(a.prec)).(a,b);;

//fun uminus!a [a imp floatmp] = floatmp_uminus.a;;
fun uminus!a [a imp intmp] = intmp_uminus.a;;

//fun constant_bang!a!b [a imp sint8] = sint8_bang.b;;
//fun constant_bang!a!b [a imp uint8] = uint8_bang.b;;
//fun constant_bang!a!b [a imp sint16] = sint16_bang.b;;
//fun constant_bang!a!b [a imp uint16] = uint16_bang.b;;
//fun constant_bang!a!b [a imp sint32] = sint32_bang.b;;
//fun constant_bang!a!b [a imp uint32] = uint32_bang.b;;
//fun constant_bang!a!b [a imp sint64] = sint64_bang.b;;
//fun constant_bang!a!b [a imp uint64] = uint64_bang.b;;
//fun constant_bang!a!b [a imp float32] = float32_bang.b;;
//fun constant_bang!a!b [a imp float64] = float64_bang.b;;

//fun float32_bang.a [a is prec] = 32;;
//fun float64_bang.a [a is prec] = 64;;

//fun sint8_bang.a [a is prec] = 8;;
//fun sint8_bang.a [a is is_signed] = true;;

//fun uint8_bang.a [a is prec] = 8;;
//fun uint8_bang.a [a is is_signed] = false;;

//fun sint16_bang.a [a is prec] = 16;;
//fun sint16_bang.a [a is is_signed] = true;;

//fun uint16_bang.a [a is prec] = 16;;
//fun uint16_bang.a [a is is_signed] = false;;

//fun sint32_bang.a [a is prec] = 32;;
//fun sint32_bang.a [a is is_signed] = true;;

//fun uint32_bang.a [a is prec] = 32;;
//fun uint32_bang.a [a is is_signed] = false;;

//fun sint64_bang.a [a is prec] = 64;;
//fun sint64_bang.a [a is is_signed] = true;;

//fun uint64_bang.a [a is prec] = 64;;
//fun uint64_bang.a [a is is_signed] = false;;

//fun uint8_bang.a.b [a : prec] = uint8_precision!b;;
//fun uint8_bang.a.b [a : signed] = false;;

//fun sint8_bang.a.b [a : prec] = sint8_precision!b;;
//fun sint8_bang.a.b [a : signed] = true;;

fun lte!a!b [a imp intmp, b imp intmp] = intmp_lte.(a,b);;
fun lte!a!b [a imp intmp, b is infty] = true;;
fun lte!a!b [a is infty, b imp intmp] = false;;
fun lte!a!b [a is infty, b is infty] = true;;
fun lte!a!b [a is neginfty, b is neginfty] = true;;
fun lte!a!b [a is neginfty, b imp intmp] = true;;
fun lte!a!b [a imp intmp, b is neginfty] = false;;
//fun lte!a!b [a imp floatmp, b imp floatmp] = floatmp_lte.(a,b);;
//fun lte!a!b [a : fint, b : fint] 
//  | a.prec == b.prec && a.is_signed == b.is_signed 
//  = fint_lte!(a.prec)!(a.is_signed).(a,b);;

fun lt!a!b [a imp intmp, b imp intmp] = intmp_lt.(a,b);;
fun lt!a!b [a imp intmp, b is infty] = true;;
fun lt!a!b [a is infty, b imp intmp] = false;;
fun lt!a!b [a is infty, b is infty] = false;;
fun lt!a!b [a is neginfty, b is neginfty] = false;;
fun lt!a!b [a is neginfty, b imp intmp] = true;;
fun lt!a!b [a imp intmp, b is neginfty] = false;;
//fun lt!a!b [a imp floatmp, b imp floatmp] = floatmp_lt.(a,b);;
//fun lt!a!b [a : fint, b : fint]
//  | a.prec == b.prec && a.is_signed == b.is_signed 
//  = fint_lt!(a.prec)!(a.is_signed).(a,b);;

fun gte!a!b [a imp intmp, b imp intmp] = intmp_gte.(a,b);;
//fun gte!a!b [a imp floatmp, b imp floatmp] = floatmp_gte.(a,b);;
//fun gte!a!b [a : fint, b : fint]
//  | a.prec == b.prec && a.is_signed == b.is_signed 
//  = fint_gte!(a.prec)!(a.is_signed).(a,b);;

fun gt!a!b [a imp intmp, b imp intmp] = intmp_gt.(a,b);;
//fun gt!a!b [a imp floatmp, b imp floatmp] = floatmp_gt.(a,b);;
//fun gt!a!b [a : fint, b : fint]
//  | a.prec == b.prec && a.is_signed == b.is_signed 
//  = fint_gt!(a.prec)!(a.is_signed).(a,b);;

fun eq!a!b [a imp intmp, b imp intmp] = intmp_eq.(a,b);;
//fun eq!a!b [a imp floatmp, b imp floatmp] = floatmp_eq.(a,b);;
//fun eq!a!b [a : fint, b : fint] 
//  | intmp_eq.(a.prec, b.prec) && bool_eq.(a.is_signed, b.is_signed)
//  = (fint_eq!(a.prec)!(a.is_signed)).(a,b);;

fun ne!a!b [a imp intmp, b imp intmp] = intmp_ne.(a,b);;
//fun ne!a!b [a imp floatmp, b imp floatmp] = floatmp_ne.(a,b);;
//fun ne!a!b [a : fint, b : fint]
//  | a.prec == b.prec && a.is_signed == b.is_signed 
//  = fint_ne!(a.prec)!(a.is_signed).(a,b);;

fun eq!a!b [a imp bool, b imp bool] = bool_eq.(a,b);;

//fun lt!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_lt!(a.prec)).(a,b);;

//fun lte!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_lte!(a.prec)).(a,b);;

//fun gt!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_gt!(a.prec)).(a,b);;

//fun gte!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_gte!(a.prec)).(a,b);;

//fun eq!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_eq!(a.prec)).(a,b);;

//fun ne!a!b [a : float, b : float] | a.prec == b.prec 
//  = (float_ne!(a.prec)).(a,b);;

fun bool_and a b = if a then b else false fi;;
fun bool_or a b = if a then true else b fi;;

fun concatenate!a!b [a imp ustring, b imp ustring] = ustring_plus.(a,b);;

fun abs!a [a imp floatmp] = floatmp_abs.a;;

fun range_construct!a!b [a imp intmp, b imp intmp] = make_range.(a,b);;
fun range_construct!a!b [a imp intmp, b is infty] = make_range_infty.a;;
fun range_construct!a!b [a is neginfty, b imp intmp] = make_range_neginfty.b;;
fun range_construct!a!b [a is neginfty, b is infty] = make_range_infinite;;

fun union_construct!a!b = make_union.(a,b);;

//convert to a from b
fun convert!a!b [a is floatmp, b imp intmp] = floatmp_convert_intmp.b;;

//var fby = \d -> \\X -> \\Y -> if #!d <= 0 then X else prev.d Y fi;;
fun fby.d X Y = if #.d <= 0 then X else prev.d Y fi;;
fun index!d = #.d + 1;;
fun prev.d X = X @ [d <- #.d - 1];;
fun next.d X = X @ [d <- #.d + 1];;

fun escape_character!c | is_printable.c = print!c;;
fun escape_character!c | ¬ (is_printable.c) && code_point.c <= 0GFFFF 
  = `\u` >> code_point_4.c;;
fun escape_character!c | ¬ (is_printable.c) && code_point.c > 0GFFFF 
  = `\U` >> code_point_8.c;;
fun escape_character!c [c is '\n'] = `\n`;;
fun escape_character!c [c is '\t'] = `\t`;;
fun escape_character!c [c is '\r'] = `\r`;;
fun escape_character!c [c is '\\'] = `\\`;;
fun escape_character!c [c is '\"'] = `\"`;;
fun escape_character!c [c is '\''] = `\'`;;

fun string_at!s!pos [s imp ustring, pos imp intmp] = string_at_base.(s,pos);;
fun substring!s!start!length [s imp ustring, start imp intmp, length imp intmp]
  = substring_base.(s, start, length);;
fun substring!s!start!length [s imp ustring, start imp intmp, length is infty]
  = substring_toend_base.(s, start);;

fun escape_string!s = escape_character!(string_at!s!0)
  >> escape_string!(substring!s!1!infty)
;;
fun escape_string!s [s is ""] = "";;

fun print_special!c [c is sperror] = "sperror";;
fun print_special!c [c is spaccess] = "spaccess";;
fun print_special!c [c is sptypeerror] = "typeerror";;
fun print_special!c [c is spdim] = "spdim";;
fun print_special!c [c is sparith] = "sparith";;
fun print_special!c [c is spundef] = "spundef";;
fun print_special!c [c is spconst] = "spconst";;
fun print_special!c [c is spmultidef] = "spmultidef";;
fun print_special!c [c is sploop] = "sploop";;

fun print!c = "I don't know how to print this type";;
fun print!c [c imp ustring] = c;;
fun print!c [c imp intmp] = print_intmp.c;;
fun print!c [c imp floatmp] = print_floatmp.c;;
fun print!c [c imp bool] = print_bool.c;;
fun print!c [c imp special] = print_special!c;;
fun print!c [c imp typetype] = print_typetype.c;;
fun print!c [c imp uchar] = print_uchar.c;;
fun print!c [c imp range] = print_range.c;;
fun print!c [c imp tuple] = print_tuple.c;;
fun print!c [c imp uuid] = print_uuid.c;;
fun print!c [c imp demand] = "Cannot print demands";;
fun print!c [c imp calc] = "Cannot print calc";;

fun print_typename!c = "unknown_typename(" >> (print!(type_index.c)) >> ")" ;;
fun print_typename!c [c imp intmp] = "intmp";;
fun print_typename!c [c imp ustring] = "ustring";;
fun print_typename!c [c imp uchar] = "uchar";;
fun print_typename!c [c imp bool] = "bool";;
fun print_typename!c [c imp range] = "range";;
fun print_typename!c [c imp tuple] = "tuple";;
fun print_typename!c [c imp special] = "special";;
fun print_typename!c [c imp typetype] = "typetype";;
fun print_typename!c [c imp uuid] = "uuid";;
fun print_typename!c [c imp demand] = "demand";;
fun print_typename!c [c imp calc] = "calc";;
fun print_typename!c [c imp basefun] = "basefun";;
fun print_typename!c [c imp lambda] = "lambda";;
fun print_typename!c [c imp phi] = "phi";;
fun print_typename!c [c imp floatmp] = "floatmp";;
fun print_typename!c [c imp intension] = "intension";;

fun construct_literal!t!v [t is "intmp"] = construct_intmp.v;;
fun construct_literal!t!v [t is "special"] = construct_special.v;;
fun construct_literal!t!v [t is "typetype"] = construct_typetype.v;;
fun construct_literal!t!v [t is "uuid"] = construct_uuid.v;;
fun construct_literal!t!v [t is "floatmp"] = construct_floatmp.v;;

//the print equations
fun canonical_print!c [c imp intmp]   = print!c;;
fun canonical_print!c [c imp ustring] = `"` >> escape_string!(print!c) >> `"`;;
fun canonical_print!c [c imp uchar]   = "'" >> escape_string!(print!c) >> "'";;
fun canonical_print!c [c imp bool]    = print!c;;
fun canonical_print!c [c imp range]   = print!c;;
fun canonical_print!c [c imp tuple]   = "[I don't know how to print a tuple]";;
fun canonical_print!c               = print_typename!c >> `"` >> 
                                      escape_string!(print!c) >> `"`;;
fun canonical_print!c [c imp special] = print!c;;
fun canonical_print!c [c is infty] = "infty";;
fun canonical_print!c [c is neginfty] = "neginfty";;

//I don't think that we want this definition
//fun canonical_print!c = "cannot print this thing";;

var RETURN = 0;;

// The first function.
fun first.d X = X @ [d <- 0] ;;

// The whenever function, B is assumed to be Boolean.
fun wvr.d X B = X @ [d <- T]
where
  var T = fby.d U (U @ [d <- T + 1]) ;;
  var U = if B then #.d else next.d U fi ;;
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
fun rotate.d.dʹ X = X @ [d <- #.dʹ] ;;
fun transpose.d.dʹ X = X @ [d <- #.dʹ, dʹ <- #.d] ;;

// For divide-and-conquer computations.
fun LofPair.d X = X @ [d <- #.d * 2] ;;
fun RofPair.d X = X @ [d <- #.d * 2 + 1] ;;

// The integer log function, for depth of divide-and-conquer.
fun ilog.n = asa.d (#.d) (double > n)
where
  dim d <- 0 ;;
  var double = fby.d 1 (double * 2) ;;
end ;;

fun runningOp.d.g X = S
where
  var S = fby.d X (g!(next.d X)!S) ;;
end ;;

fun default₁.d.m.n.val X = Y
where
  var Y [d : m..n] = X ;;
  var Y [d : nat] = val ;;
end ;;

fun default₂.d₁.m₁.n₁.d₂.m₂.n₂.val X = Y
where
  var Y [d₁ : m₁..n₁, d₂ : m₂..n₂] = X ;;
  var Y [d₁ : nat, d₂ : nat] = val ;;
end ;;

fun tournamentOp₁.d.n.g X = Y @ [d <- 0]
where
  dim t <- ilog.n ;;
  var Y = fby.t X (g!(LofPair.d Y)!(RofPair.d Y)) ;;
end ;;

fun tournamentOp₂.d₁.d₂.n.g X = Y @ [d <- 0]
where
  dim t <- ilog.n ;;
  var Y = fby.t X (g!(NWofQuad.d₁.d₂ Y)!(NEofQuad.d₁.d₂ Y)!
                     (SWofQuad.d₁.d₂ Y)!(SEofQuad.d₁.d₂ Y)) ;;
end ;;


// Define lists and functions over lists.

// The List data type and the Cons and Nil constructors.
data List;;
constructor Cons a b [b : List] = List;;
constructor Nil = List;;

// Two infix operators defined over lists.
op :: = OpInfix."cons_list".false.AssocRight.(~10) ;;
op <> = OpInfix."append_list".false.AssocRight.(~10) ;;

// The cons_list function corresponding to operator "::".
fun cons_list!a!b [b : [type is "List"]] = Cons.a.b ;;

// The append_list function corresponding to operator "<>".
fun append_list!l₁!l₂
  [l₁ : Nil, l₂ : [type is "List"]] = l₂ ;;
fun append_list!l₁!l₂
  [l₁ : [type is "List"], l₂ : [type is "List"]]
  = head.l₁ :: append_list!(tail.l₁)!l₂ ;;

// True if l is a List.
fun isList.l [l : [type is "List"]] = true ;;
fun isList.l = false ;;

// True if l is Nil.
fun isNil.l [l : Nil] = true ;;
fun isNil.l = false ;;

// The head of non-empty list l.
fun head.l [l : [type is "List", cons is "Cons"]] = l.arg0;;

// The tail of non-empty list l.
fun tail.l [l : [type is "List", cons is "Cons"]] = l.arg1;;

// The length of a list.
fun length.l [l : Nil] = 0 ;;
fun length.l [l : [type is "List"]] = 1 + length.(tail.l) ;;
