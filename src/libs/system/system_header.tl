//read = 1, write = 2, rw = 3

//have to implement bang
//file[arg0 : ustring, arg1 : 1 ..3] = "fileopen"!(#arg0, #arg1);;
ifile [arg0 : ustring] = file @ [arg1 <- 1];;
ofile [arg0 : ustring] = file @ [arg1 <- 2];;
iofile [arg0 : ustring] = file @ [arg1 <- 3];;
