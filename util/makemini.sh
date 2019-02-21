CC=cc
CFLAGS="-g -I./vm -c -I./include -I./optional --std=c99"

cd src
$CC $CFLAGS vm/*.c 
$CC $CFLAGS optional/*.c 
$CC $CFLAGS mini/*.c
$CC -o mini.out *.o -lm
./mini.out mini/test.wren



