#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <bsp.h>

long P;
long n;


void bsp_sieve() {
    bsp_begin(P);

    long s = bsp_pid();
    int sqrt_n = ceil(sqrt(n));
     
    // For parallel part
    int left_part = floor(s * (n/P)) + 3;
    int right_part = floor((s+1) * (n/P)) + 1;
    int local_sieve_size = floor((s+1) * (n/P)) + 1 - floor(s * (n/P)) + 3;

    if(s==0){
        int size_non_even = ceil(sqrt(sqrt_n)/2.0);

        // Create boolean uneven number array from 1 to sqrt(n) 
        bool *is_prime_small = malloc(sizeof(bool)*ceil(sqrt_n / 2.0));
        memset(is_prime_small, true, sizeof(bool)*ceil(sqrt_n / 2.0));

        // We skip uneven number 1, but this is not prime so set to false
        is_prime_small[0] = false;

        // Find primes below sqrt (sqrt (n)) in sequential way to minimize communication. This is faster than just doing sqrt(n)
        clock_t begin_run_1 = clock();
        for (int i=1; i < ceil(sqrt(sqrt(n)) / 2.0); i+=1){
            if(is_prime_small[i] == true){
                for (int j=i+(i*2)+1; j < ceil(sqrt_n/2.0); j+=(i*2)+1){
                    is_prime_small[j] = false;
                }
            }
        }
        clock_t end_run_1 = clock();
        double time_spent_1 = (double)(end_run_1 - begin_run_1) / CLOCKS_PER_SEC;
        printf("Time spent: %f \n", time_spent_1);



        // Print number of primes found for testing
        int num_primes_found = 1;
        for (int k=0; k < ceil(sqrt_n/2.0); ++k){
            num_primes_found += is_prime_small[k];
        }
        printf("Num primes found: %d", num_primes_found);
    }
    // This part we communicate the found primes and let each processor finish the sieve using the given primes
    bsp_end();
}

int main( int argc, char ** argv ) {

    bsp_init(&bsp_sieve, argc, argv );

    P = bsp_nprocs();
    n = 1000;



    bsp_sieve();

    return EXIT_SUCCESS;
}