" we care about case
sy case match

" decl keywords
sy keyword      tlDecl             var fun op

" if expressions
sy keyword      tlConditional      if then else elsif fi

hi def link     tlDecl             Statement
hi def link     tlConditional      Conditional

sy match        Number             "\([1-9]\+\)\|\(0\([2-9a-zA-Z][0-9a-zA-Z]\+\)\?\)"

sy match        String             /"[^\_$]*"/
sy match        String             /`\_.*`/
sy match        Error              /"[^"]*$/
