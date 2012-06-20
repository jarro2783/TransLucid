//Wed Jun 20 14:36:42 EST 2012
//This one doesn't work yet

// Matrix multiplication.  X and Y are (x,y)-dimensional variables,
// with X viewed as an m*p matrix and Y viewed as a p*n matrix.
// The variables Xʹ and Yʹ are the same as X and Y for the matrix
// parts, with 0s addded all around.  Variable Z is the pointwise
// multiplication of Xʹ, with y-variance rotated into z-variance,
// and Yʹ, with x-variance rotated into z-variance.

fun mm.x.y.m.p.n X Y = tournamentOp₁.z.p.plus Z
where
  dim z <- p - 1 ;;
  var Xʹ = default₂.x.0.(m-1).y.0.(p-1).0 X ;;
  var Yʹ = default₂.x.0.(p-1).y.0.(n-1).0 Y ;;
  var Z  = (rotate.y.z Xʹ) * (rotate.x.z Yʹ) ;;
end ;;

// Test data.
dim a ;;
dim b ;;
dim c ;;
var A = 2*#!a + 3*#!b + 1;;
var B = (rotate.b.c A) * (rotate.a.c A) ;;
var C = mm.a.b.2.2.2 A A ;;

%%

A @ [a <- 0, b <- 0] ;;
A @ [a <- 0, b <- 1] ;;
A @ [a <- 1, b <- 0] ;;
A @ [a <- 1, b <- 1] ;;

$$%%
B @ [a <- 0, b <- 0, c <- 0] ;;
B @ [a <- 0, b <- 0, c <- 1] ;;
B @ [a <- 0, b <- 1, c <- 0] ;;
B @ [a <- 0, b <- 1, c <- 1] ;;
B @ [a <- 1, b <- 0, c <- 0] ;;
B @ [a <- 1, b <- 0, c <- 1] ;;
B @ [a <- 1, b <- 1, c <- 0] ;;
B @ [a <- 1, b <- 1, c <- 1] ;;

$$%%

mm;;
default₂;;
rotate;;
tournamentOp₁;;
tournamentOp₁.a.5.plus A @ [b <- 3];;
default₂.a.0.(m-1).b.0.(p-1).0 A @ [a <- 0, b <- 0] ;;
default₂.a.0.(m-1).b.0.(p-1).0 A @ [a <- 0, b <- 0] ;;
default₂.a.0.(p-1).b.0.(n-1).0 B @ [a <- 0, b <- 0, c <- 0] ;;

$$%%
C @ [a <- 0, b <- 0] ;;
C @ [a <- 0, b <- 1] ;;
C @ [a <- 1, b <- 0] ;;
C @ [a <- 1, b <- 1] ;;
C @ [a <- 0, b <- 2] ;;
C @ [a <- 0, b <- 3] ;;
C @ [a <- 1, b <- 2] ;;
C @ [a <- 1, b <- 3] ;;
C @ [a <- 2, b <- 0] ;;
C @ [a <- 2, b <- 1] ;;
C @ [a <- 3, b <- 0] ;;
C @ [a <- 3, b <- 1] ;;
C @ [a <- 2, b <- 2] ;;
C @ [a <- 2, b <- 3] ;;
C @ [a <- 3, b <- 2] ;;
C @ [a <- 3, b <- 3] ;;
