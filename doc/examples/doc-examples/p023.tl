data BinaryTree a = BinNode a BinaryTree BinaryTree | BinLeaf a ;;

//the below two declarations are added for the above data declaration
//fun BinNode.a.b.c =
//[type <- "BinaryTree", cons <- "BinNode", arg0 <- a, arg1 <- b, arg2 <- c] ;;

//fun BinLeaf.a =
//[type <- "BinaryTree", cons <- "BinLeaf", arg0 <- a] ;;

%%

//the result of this is
//  [I don't know how to print a tuple]
//because printing tuples has not yet been implemented
BinNode.10.(BinLeaf.4).(BinLeaf.15);;

$$
