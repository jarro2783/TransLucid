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
op && = OpInfix."bool_and".true.AssocLeft.20;;
op || = OpInfix."bool_or".true.AssocLeft.15;;

op .. = OpInfix."range_construct".false.AssocLeft.0;;

op ∪ = OpInfix."union_construct".false.AssocLeft. ~100;;
//op \/ = OpInfix."union_construct".false.AssocLeft.~100;;

op √ = OpPrefix."sqrt".false;;

var sint = sint8 ∪ sint16 ∪ sint32 ∪ sint64;;
var uint = uint8 ∪ uint16 ∪ uint32 ∪ uint64;;
var fint = sint ∪ uint;;
var float = float32 ∪ float64;;
var numeric = float ∪ int;;

var pos = 1..infty;;
var nat = 0..infty;;
var neg = neginfty .. ~1;;
var nonpos = neginfty..0;;
//var int = neginfty..infty;;

//fun plus.a.b [a : intmp, b : intmp] = int_plus!(a,b);;
fun plus.a.b [a : intmp, b : intmp] = intmp_plus!(a,b);;
fun plus.a.b [a : floatmp, b : floatmp] = floatmp_plus!(a,b);;
fun plus.a.b [a : fint, b : fint] 
  | a!prec == b!prec && a!is_signed == b!is_signed
  = (fint_plus.(a!prec).(a!is_signed))!(a,b);;

fun minus.a.b [a : intmp, b : intmp] = intmp_minus!(a,b);;
fun minus.a.b [a : floatmp, b : floatmp] = floatmp_minus!(a,b);;
fun minus.a.b [a : fint, b : fint] 
  | a!prec == b!prec && a!is_signed == b!is_signed
  = (fint_minus.(a!prec).(a!is_signed))!(a,b);;

fun times.a.b [a : intmp, b : intmp] = intmp_times!(a,b);;
fun times.a.b [a : floatmp, b : floatmp] = floatmp_times!(a,b);;
fun times.a.b [a : fint, b : fint] 
  | a!prec == b!prec && a!is_signed == b!is_signed
  = (fint_times.(a!prec).(a!is_signed))!(a,b);;

fun divide.a.b [a : intmp, b : intmp] = intmp_divide!(a,b);;
fun divide.a.b [a : floatmp, b : floatmp] = floatmp_divide!(a,b);;
fun divide.a.b [a : fint, b : fint] 
  | a!prec == b!prec && a!is_signed == b!is_signed
  = (fint_divide.(a!prec).(a!is_signed))!(a,b);;

fun modulus.a.b [a : intmp, b : intmp] = intmp_modulus!(a,b);;
fun modulus.a.b [a : fint, b : fint] 
  | a!prec == b!prec && a!is_signed == b!is_signed
  = (fint_modulus.(a!prec).(a!is_signed))!(a,b);;

fun sqrt.a [a : floatmp] = floatmp_sqrt!a;;

fun plus.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_plus.(a!prec))!(a,b);;

fun minus.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_minus.(a!prec))!(a,b);;

fun times.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_times.(a!prec))!(a,b);;

fun divide.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_divide.(a!prec))!(a,b);;

fun uminus.a [a : floatmp] = floatmp_uminus!a;;
fun uminus.a [a : intmp] = intmp_uminus!a;;

fun constant_bang.a.b [a : sint8] = sint8_bang.b;;
fun constant_bang.a.b [a : uint8] = uint8_bang.b;;
fun constant_bang.a.b [a : sint16] = sint16_bang.b;;
fun constant_bang.a.b [a : uint16] = uint16_bang.b;;
fun constant_bang.a.b [a : sint32] = sint32_bang.b;;
fun constant_bang.a.b [a : uint32] = uint32_bang.b;;
fun constant_bang.a.b [a : sint64] = sint64_bang.b;;
fun constant_bang.a.b [a : uint64] = uint64_bang.b;;
fun constant_bang.a.b [a : float32] = float32_bang.b;;
fun constant_bang.a.b [a : float64] = float64_bang.b;;

fun float32_bang.a [a : prec] = 32;;
fun float64_bang.a [a : prec] = 64;;

fun sint8_bang.a [a : prec] = 8;;
fun sint8_bang.a [a : is_signed] = true;;

fun uint8_bang.a [a : prec] = 8;;
fun uint8_bang.a [a : is_signed] = false;;

fun sint16_bang.a [a : prec] = 16;;
fun sint16_bang.a [a : is_signed] = true;;

fun uint16_bang.a [a : prec] = 16;;
fun uint16_bang.a [a : is_signed] = false;;

fun sint32_bang.a [a : prec] = 32;;
fun sint32_bang.a [a : is_signed] = true;;

fun uint32_bang.a [a : prec] = 32;;
fun uint32_bang.a [a : is_signed] = false;;

fun sint64_bang.a [a : prec] = 64;;
fun sint64_bang.a [a : is_signed] = true;;

fun uint64_bang.a [a : prec] = 64;;
fun uint64_bang.a [a : is_signed] = false;;

//fun uint8_bang.a.b [a : prec] = uint8_precision!b;;
//fun uint8_bang.a.b [a : signed] = false;;

//fun sint8_bang.a.b [a : prec] = sint8_precision!b;;
//fun sint8_bang.a.b [a : signed] = true;;

fun lte.a.b [a : intmp, b : intmp] = intmp_lte!(a,b);;
fun lte.a.b [a : intmp, b : infty] = true;;
fun lte.a.b [a : infty, b : intmp] = false;;
fun lte.a.b [a : infty, b : infty] = true;;
fun lte.a.b [a : neginfty, b : neginfty] = true;;
fun lte.a.b [a : neginfty, b : intmp] = true;;
fun lte.a.b [a : intmp, b : neginfty] = false;;
fun lte.a.b [a : floatmp, b : floatmp] = floatmp_lte!(a,b);;
fun lte.a.b [a : fint, b : fint] 
  | a!prec == b!prec && a!is_signed == b!is_signed 
  = fint_lte.(a!prec).(a!is_signed)!(a,b);;

fun lt.a.b [a : intmp, b : intmp] = intmp_lt!(a,b);;
fun lt.a.b [a : intmp, b : infty] = true;;
fun lt.a.b [a : infty, b : intmp] = false;;
fun lt.a.b [a : infty, b : infty] = false;;
fun lt.a.b [a : neginfty, b : neginfty] = false;;
fun lt.a.b [a : neginfty, b : intmp] = true;;
fun lt.a.b [a : intmp, b : neginfty] = false;;
fun lt.a.b [a : floatmp, b : floatmp] = floatmp_lt!(a,b);;
fun lt.a.b [a : fint, b : fint]
  | a!prec == b!prec && a!is_signed == b!is_signed 
  = fint_lt.(a!prec).(a!is_signed)!(a,b);;

fun gte.a.b [a : intmp, b : intmp] = intmp_gte!(a,b);;
fun gte.a.b [a : floatmp, b : floatmp] = floatmp_gte!(a,b);;
fun gte.a.b [a : fint, b : fint]
  | a!prec == b!prec && a!is_signed == b!is_signed 
  = fint_gte.(a!prec).(a!is_signed)!(a,b);;

fun gt.a.b [a : intmp, b : intmp] = intmp_gt!(a,b);;
fun gt.a.b [a : floatmp, b : floatmp] = floatmp_gt!(a,b);;
fun gt.a.b [a : fint, b : fint]
  | a!prec == b!prec && a!is_signed == b!is_signed 
  = fint_gt.(a!prec).(a!is_signed)!(a,b);;

fun eq.a.b [a : intmp, b : intmp] = intmp_eq!(a,b);;
fun eq.a.b [a : floatmp, b : floatmp] = floatmp_eq!(a,b);;
fun eq.a.b [a : fint, b : fint] 
  | intmp_eq!(a!prec, b!prec) && bool_eq!(a!is_signed, b!is_signed)
  = (fint_eq.(a!prec).(a!is_signed))!(a,b);;

fun ne.a.b [a : intmp, b : intmp] = intmp_ne!(a,b);;
fun ne.a.b [a : floatmp, b : floatmp] = floatmp_ne!(a,b);;
fun ne.a.b [a : fint, b : fint]
  | a!prec == b!prec && a!is_signed == b!is_signed 
  = fint_ne.(a!prec).(a!is_signed)!(a,b);;

fun eq.a.b [a : bool, b : bool] = bool_eq!(a,b);;

fun lt.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_lt.(a!prec))!(a,b);;

fun lte.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_lte.(a!prec))!(a,b);;

fun gt.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_gt.(a!prec))!(a,b);;

fun gte.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_gte.(a!prec))!(a,b);;

fun eq.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_eq.(a!prec))!(a,b);;

fun ne.a.b [a : float, b : float] | a!prec == b!prec 
  = (float_ne.(a!prec))!(a,b);;

fun bool_and a b = if a then b else false fi;;
fun bool_or a b = if a then true else b fi;;

fun plus.a.b [a : ustring, b : ustring] = ustring_plus!(a,b);;

fun abs.a [a : floatmp] = floatmp_abs!a;;

fun range_construct.a.b [a : intmp, b : intmp] = make_range!(a,b);;
fun range_construct.a.b [a : intmp, b : infty] = make_range_infty!a;;
fun range_construct.a.b [a : neginfty, b : intmp] = make_range_neginfty!b;;
fun range_construct.a.b [a : neginfty, b : infty] = make_range_infinite;;

fun union_construct.a.b = make_union!(a,b);;

//convert to a from b
fun convert.a.b [a : floatmp, b : intmp] = floatmp_convert_intmp!b;;

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
fun print.c [c : floatmp] = print_floatmp!c;;
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
fun print_typename.c [c : basefun] = "basefun";;
fun print_typename.c [c : lambda] = "lambda";;
fun print_typename.c [c : phi] = "phi";;
fun print_typename.c [c : floatmp] = "floatmp";;

fun construct_literal.t.v [t : "intmp"] = construct_intmp!v;;
fun construct_literal.t.v [t : "special"] = construct_special!v;;
fun construct_literal.t.v [t : "typetype"] = construct_typetype!v;;
fun construct_literal.t.v [t : "uuid"] = construct_uuid!v;;
fun construct_literal.t.v [t : "floatmp"] = construct_floatmp!v;;

//the print equations
fun canonical_print.c [c : intmp]   = print.c;;
fun canonical_print.c [c : ustring] = `"` + escape_string.(print.c) + `"`;;
fun canonical_print.c [c : uchar]   = "'" + escape_string.(print.c) + "'";;
fun canonical_print.c [c : bool]    = print.c;;
fun canonical_print.c [c : range]   = print.c;;
fun canonical_print.c [c : tuple]   = "[I don't know how to print a tuple]";;
fun canonical_print.c               = print_typename.c + `"` + 
                                      escape_string.(print.c) + `"`;;
//fun canonical_print.c = "cannot print this thing";;
fun canonical_print.c [c : special] = print.c;;
fun canonical_print.c [c : infty] = "infty";;
fun canonical_print.c [c : neginfty] = "neginfty";;

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
fun rotate.d.dʹ X = X @ [d <- #!dʹ] ;;
fun transpose.d.dʹ X = X @ [d <- #!dʹ, dʹ <- #!d] ;;

// For divide-and-conquer computations.
fun firstOfPair.d X = X @ [d <- #!d * 2] ;;
fun secondOfPair.d X = X @ [d <- #!d * 2 + 1] ;;

// The integer log function, for depth of divide-and-conquer.
fun ilog.n = asa.d (#!d) (double > n)
where
  dim d <- 0 ;;
  var double = fby.d 1 (double * 2) ;;
end ;;

fun runningOp.d.g X = S
where
  var S = fby.d X (g.(next.d X).S) ;;
end ;;

fun default₁.d.m.n.val X = f
where
  var f [d : m..n] = X ;;
  var f [d : nat] = val ;;
end ;;

fun default₂.d₁.m₁.n₁.d₂.m₂.n₂.val X = f
where
  var f [d₁ : m₁..n₁, d₂ : m₂..n₂] = X ;;
  var f [d₁ : nat, d₂ : nat] = val ;;
end ;;

fun tournamentOp₁.d.n.g A = B @ [d <- 0]
where
  dim t <- ilog.n ;;
  var B = fby.t A (g.(firstOfPair.d B).(secondOfPair.d B)) ;;
end ;;

fun tournamentOp₂.d₁.d₂.n.g A = B @ [d <- 0]
where
  dim t <- ilog.n ;;
  var B = fby.t A (g.(NWofQuad.d₁.d₂ B).(NEofQuad.d₁.d₂ B).
                     (SWofQuad.d₁.d₂ B).(SEofQuad.d₁.d₂ B)) ;;
end ;;
