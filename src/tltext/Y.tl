var almostI = \f -> fby.0 0 (↓f + 1) ;;
var Y = \\f -> f (Y f) ;;
var Y2 = \\f -> (\\x -> (f (x x))) (\\x -> (f (x x))) ;;
var I = Y almostI;;
var I2 = Y2 almostI;;

var Y_2 = \\f -> 0 ;;

var odd = 0;;

%%
Y2 ;;
