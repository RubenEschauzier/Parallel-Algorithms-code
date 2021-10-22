#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <bsp.h>

// REMEMBER, I REMOVE ALL PRIMES BEFORE sqrt N!

long P;
long n;
int get_start(int left_index, int right_index, int prime){
    // Function to calculate from what number we should sieve the local array, input is the left and right index that make the partition of the global array
    // we return the local index to start sieve from
    
    // Left start is the first number of the local array, while right end is the last number of local array
    // We do not add 1 to right_index, due to the last index never being reached
    int left_start = ((left_index + 1) * 2) + 1; int right_end = (right_index) * 2 + 1;
    if (left_start % prime == 0){
        return left_index;
    }
    int start_number = left_start - (left_start % prime) + prime;

    // Skip if the interval is too small to contain a division
    if (start_number > right_end){
        return -1;
    }
    // If even we can make uneven by adding our prime
    if (start_number % 2 == 0){
        // Map start number to index by: i = ((start - 1) / 2) - 1, `inverse' of our index to prime mapping
        return ((start_number + prime - 1) / 2) - 1;
    }
    else{
        return ((start_number - 1) / 2) - 1;
    }
 
}
void sieve(long s, int prime, int size_local_sieve, int left_index, int right_index, bool local_array[] ){
    int start_index = get_start(left_index, right_index, prime);
    printf("Start sieve(%ld): %d, Left index: %d, Prime: %d \n", s, start_index , left_index, prime);
    if(start_index != -1){
        for (int i = start_index-left_index; i < size_local_sieve; i += prime){
            local_array[i] = false;
        }  
    }
    printf("End sieve %ld \n", s);
}

void bsp_sieve() {
    bsp_begin(P);

    long s = bsp_pid();
    int sqrt_n = ceil(sqrt(n));
     
    // Calculate the division of the sieve array, we only include non-even numbers and sieve from 3
    int left_part = floor(s*floor((n-1)/2)/P);
    int right_part = floor((s+1)*floor((n-1)/2)/P);
    int local_sieve_size = right_part - left_part;

    int num_primes_result[P];
    memset(num_primes_result, 0, sizeof(int)*P);


    bool *local_sieve_array = malloc(sizeof(bool)*local_sieve_size);
    memset(local_sieve_array, true, sizeof(bool)*local_sieve_size);

    int local_num_primes = 0;

    printf("P = %ld, Left: %d , Right: %d, Size: %d \n", s, left_part, right_part, local_sieve_size);



    // Allocate space for array that will be used by s=0 to find all primes below sqrt(n) 
    bool *is_prime_small = malloc(sizeof(bool)*ceil(sqrt_n / 2.0));
    memset(is_prime_small, true, sizeof(bool)*ceil(sqrt_n / 2.0));

    // Register the array to all other processors to recieve what primes to sieve with
    bsp_push_reg(is_prime_small, sizeof(bool)*ceil(sqrt_n / 2.0));
    bsp_push_reg(num_primes_result, sizeof(int)*P);
    bsp_sync();


    if(s==0){

        // Create boolean uneven number array from 1 to sqrt(n) 
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

        for(long p_num = 1; p_num < P; ++p_num){
            for(int elem = 0; elem < ceil(sqrt_n / 2.0); ++elem){
                bsp_put(p_num, &is_prime_small[elem], is_prime_small, elem*sizeof(bool), sizeof(bool));
            }
        }
    }

    bsp_sync();
    for (int z = 0; z < ceil(sqrt_n / 2.0); ++z){
        if(is_prime_small[z]){
            int prime = (z*2) + 1;
            sieve(s, prime, local_sieve_size, left_part, right_part, local_sieve_array);
            //printf("Processor %ld, found %d \n", s, prime);
        }
    }



    // for (int i = 0; i < ceil(sqrt_n/2.0); ++i){
    //     if(is_prime_small[i] == true){
    //         int prime = (i*2) + 1;
    //         printf("Prime: %d \n", prime);
    //         //printf("Left: %d, Right: %d \n", left_part, right_part);
    //         sieve(s, prime, local_sieve_size, left_part, right_part, &local_sieve_array);
    //     }
    // }
    printf("processor %ld: \n", s);

    for (int j = 0; j < local_sieve_size; j++){
        if(local_sieve_array[j]){
            local_num_primes += 1;
            //printf("P % ld, Index %d, Prime: %d \n", s, j, 1 + (left_part+j+1)*2);
        }
    }
    for (int l = 0; l < P; ++l){
        bsp_put(l, &local_num_primes, num_primes_result, s*sizeof(int), sizeof(int));
    }
    bsp_sync();

    if (s==0){
        // Start count primes at 1, since 2 is an even prime and not counted
        int total_primes = 1;
        for (int m = 0; m < P; ++m){
            total_primes += num_primes_result[m];
        }
        printf("Total Primes found: %d \n", total_primes);

    }

    // // Start at 1, because we exlcude uneven numbers
    // int num_primes_small = 1;


    // int primes_small[num_primes_small];
    // printf("Processor %ld, found %d primes \n", s, num_primes_small);

    // for (int k=0; k<ceil(sqrt_n / 2.0); ++k){
    //     if(is_prime_small[k]){
    //         primes_small[k] =(k*2)+1;
    //     }
    // }

    

    bsp_end();

    
    // This part we communicate the found primes and let each processor finish the sieve using the given primes
}

int main( int argc, char ** argv ) {

    bsp_init(&bsp_sieve, argc, argv );

    P = bsp_nprocs();
    n = 1011;



    bsp_sieve();

    return EXIT_SUCCESS;
}