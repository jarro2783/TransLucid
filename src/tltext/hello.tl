dim lang ;;
dim country ;;
var HELLO [ lang : "fr" ] = "Bonjour" ;;
var HELLO [ lang : "fr", country : "qc" ]  = "Ça va tu?" ;;
var HELLO [ lang : "en" ]  = "Hello" ;;
var HELLO [ lang : "en", country : "au" ]  = "G'day, mate" ;;
var HELLO [ lang : "en", country : "ca" ]  = "G'day, eh?" ;;
var HELLO [ lang : "it" ]  = "Buon giorno" ;;
var HELLO [ lang : "es" ]  = "¡Hola!" ;;
var HELLO [ lang : "de" ]  = "Guten tag" ;;
var HELLO [ lang : "de", country : "ch" ]  = "Gruzzi" ;;
%%
HELLO @ [lang <- CLARGS @ [arg0 <- 0],
         country <- CLARGS @ [arg0 <- 1]];;
