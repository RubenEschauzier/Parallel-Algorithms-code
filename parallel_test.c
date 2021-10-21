#include <bsp.h>
#include <stdio.h>
#include <stdlib.h>

void spmd() {
    bsp_begin( 3 );
    printf( "Hello world!\n" );
    bsp_end();
}

int main( int argc, char ** argv ) {
    int num_processors = bsp_nprocs();
    printf("%d", num_processors);
    bsp_init( &spmd, argc, argv );
    spmd();
    return EXIT_SUCCESS;
}