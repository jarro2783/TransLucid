//tlcore --reactive -i reactive.tl
//this example doesn't work properly yet

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

