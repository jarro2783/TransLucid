var ack = if #!0 == 0 then #!1 + 1
elsif #!1 == 0 then ack @ [0 <- #!0 - 1, 1 <- 1]
else ack @ [0 <- #!0 - 1, 1 <- ack @ [1 <- #!1 - 1]] fi ;;

%%
ack @ [0<-1,1<-1] ;;
$$
