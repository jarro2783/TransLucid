data BinaryTree ;;
constructor BinNode a b c [b : BinaryTree, c: BinaryTree] = BinaryTree ;;
constructor BinLeaf a = BinaryTree ;;

//fun BinNode.a.b.c =
//[type <- "BinaryTree", cons <- "BinNode",
//arg0 <- a, arg1 <- b, arg2 <- c] ;;

//fun BinLeaf.a =
//[type <- "BinaryTree", cons <- "BinLeaf", arg0 <- a] ;;

%%

BinNode.10.(BinLeaf.4).(BinLeaf.15);;

$$
