" we care about case
sy case match

" decl keywords
sy keyword      tlDecl             var fun op assign hd dim

" if expressions
sy keyword      tlConditional      if then else elsif fi
sy keyword      tlWhere            where end
sy keyword      tlBool             true false

hi def link     tlDecl             Type
hi def link     tlConditional      Conditional
hi def link     tlWhere            Conditional
hi def link     tlBool             Constant

sy match        Number             "\~\?\<\([1-9][0-9]*\)\|\(0\([2-9a-zA-Z][0-9a-zA-Z]\+\)\?\)\>"

sy region       tlRawString        start=+`+ end=+`+
sy region       String             start=+"+ skip=+\\.+ end=+"+
sy region       Character          start=+'+ skip=+\\.+ end=+'+
sy match        Error              /;/
sy match        cleared            /;;/
sy match        Comment            "//.*$"

hi def link     tlRawString        String
