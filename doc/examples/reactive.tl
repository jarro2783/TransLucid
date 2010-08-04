//tlcore --reactive -i reactive.tl

%%
x = 1;;
%%
#time;;
$$
%%
x = 2;;
%%
$$
%%
%%
#time;;
x;;
x @ [time : 0];;

