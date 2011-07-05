eqn PRINT | [arg0 : intmp] = print_int;;

eqn print_int | [arg0 : 0..9] = print_int_unit;;
eqn print_int | [arg0 : -inf..-1] = "~" + print_int @ [arg0 <- -#arg0];;
eqn print_int = print_int @ [arg0 <- #arg0 / 10] + 
                  print_int_unit @ [arg0 <- #arg0 % 10];;

eqn print_int_unit | [arg0 : 0] = "0";;
eqn print_int_unit | [arg0 : 1] = "1";;
eqn print_int_unit | [arg0 : 2] = "2";;
eqn print_int_unit | [arg0 : 3] = "3";;
eqn print_int_unit | [arg0 : 4] = "4";;
eqn print_int_unit | [arg0 : 5] = "5";;
eqn print_int_unit | [arg0 : 6] = "6";;
eqn print_int_unit | [arg0 : 7] = "7";;
eqn print_int_unit | [arg0 : 8] = "8";;
eqn print_int_unit | [arg0 : 9] = "9";;
