" we care about case
sy case match

" decl keywords
sy keyword      tlDecl             var fun op assign hd dim data constructor

" if expressions
sy keyword      tlConditional      if then else elsif fi
sy keyword      tlWhere            where end
sy keyword      tlBool             true false

hi def link     tlDecl             Type
hi def link     tlConditional      Conditional
hi def link     tlWhere            Conditional
hi def link     tlBool             Constant

sy match        Error              "\<0[1-9a-zA-Z]\>"
sy match        tlDecimalNumber    "\~\?\<[1-9][0-9]*\>"
sy match        tlBasedNumber      "\~\?\<\(0\([2-9a-zA-Z][0-9a-zA-Z]\+\)\?\)\>"

hi def link     tlDecimalNumber    Number
hi def link     tlBasedNumber      Number

sy region       tlRawString        start=+`+ end=+`+
sy region       String             start=+"+ skip=+\\.+ end=+"+
sy region       Character          start=+'+ skip=+\\.+ end=+'+
sy match        Error              /;/
sy match        cleared            /;;/

hi def link     tlRawString        String

sy match        tlReservedSymbol   ":"
sy match        tlReservedSymbol   "\["
sy match        tlReservedSymbol   "\]"
sy match        tlReservedSymbol   "\<=\>"
sy match        tlReservedSymbol   "@"
sy match        tlReservedSymbol   "<-"
sy match        tlReservedSymbol   "#"
sy match        tlReservedSymbol   "!"
sy match        tlReservedSymbol   "\."
sy match        tlReservedSymbol   "\<,\>"
sy match        tlReservedSymbol   "%%"
sy match        tlReservedSymbol   "\$\$"
sy match        tlReservedSymbol   "("
sy match        tlReservedSymbol   ")"
sy match        tlReservedSymbol   "|"

hi              tlReservedSymbol   ctermfg=DarkGrey

sy match        Comment            "//.*$"
