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
int get_start(int left_index, int right_index, int prime){
    // Function to calculate from what number we should sieve the local array, input is the left and right index that make the partition of the global array
    // we return the local index to start sieve from
    
    // Left start is the first number of the local array, while right end is the last number of local array
    // We do not add 1 to right_index, due to the last index never being reached
    int left_start = ((left_index + 1) * 2) + 1; int right_end = (right_index) * 2 + 1;
    if (left_start % prime == 0 && left_start != prime){
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

    if(start_index != -1){
        for (int i = start_index-left_index; i < size_local_sieve; i += prime){
            if((((i + left_index + 1) * 2) + 1) == prime){
                continue;
            }
            local_array[i] = false;
        }  
    }
}

void bsp_sieve_iterative(){
    bsp_begin(P);
    // Superstep one, register and communicate all needed variables
    int prime_index = 0;

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

    bsp_push_reg(&prime_index, sizeof(int));
    bsp_push_reg(&num_primes_result, P*sizeof(int));
    bsp_sync();

    while (prime_index < sqrt_n/2.0){
        // Combined superstep: Get next prime, communicate it and sieve the array
        sieve(s, ((prime_index+1)*2) + 1, local_sieve_size, left_part, right_part, local_sieve_array);

        if (s==0){
            do{
                prime_index += 1;
            }
            while(prime_index < ceil(sqrt_n/2.0)-1 && local_sieve_array[prime_index]!=1);

            for (int k = 1; k < P; ++k){
                bsp_put(k, &prime_index, &prime_index, 0, sizeof(int));
            }
        }
        bsp_sync();
    }
    // Superstep: Communicate the number of primes each processor found
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

    // Superstep: Combine number of primes and push total number primes to all processors
    // Start count primes at 1, since 2 is an even prime and not counted
    int total_primes = 1;
    for (int m = 0; m < P; ++m){
        total_primes += num_primes_result[m];
    }
    printf("Total Primes found: %d \n", total_primes);

    // Define prime array and set first element to 2
    long primes[total_primes];
    primes[0] = 2;

    bsp_push_reg(&primes, total_primes*sizeof(long));
    bsp_sync();

    // Final superstep: Create array of all found primes
    int prime_index_put = 0;
    int processor_offset = 0;
    long prime_to_add = 0;

    for (int t = 0; t < s; ++t){
        processor_offset += num_primes_result[t];
    }
    
    for (int p = 0; p < local_sieve_size; ++p){
        if (local_sieve_array[p]){
            prime_to_add = (p+1+left_part)*2 + 1;
            //("Prime to add: %ld", prime_to_add);
            bsp_put(0, &prime_to_add, primes, (processor_offset + prime_index_put + 1)*sizeof(long), sizeof(long));
            prime_index_put += 1;
        }
    }
    bsp_sync();
    // if(s==0){
    //     for(int u = 0; u < total_primes; ++u){
    //         printf("Prime %ld \n", primes[u]);
    //     }
    // }

    
}

int main( int argc, char ** argv ) {

    P = bsp_nprocs();
    //n = pow(2,25);
    n = 100;

    if (P > ceil(floor((n-1)/2))/floor(sqrt(n))){
        P = ceil(floor((n-1)/2))/floor(sqrt(n));
    }
    // Ensure atleast one core is used 
    if (P == 0){
        P = 1;
    }
    printf("P: %ld", P);

    bsp_init(&bsp_sieve_iterative, argc, argv );

    clock_t time0 = clock();
    bsp_sieve_iterative();
    clock_t time1 = clock();
    double elapsed = (double) (time1 - time0) / CLOCKS_PER_SEC;

    printf("Time spent: %f \n", elapsed);

    return EXIT_SUCCESS;
}