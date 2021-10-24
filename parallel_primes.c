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
    //printf("Start sieve(%ld): %d, Left index: %d, Prime: %d \n", s, start_index , left_index, prime);
    if(start_index != -1){
        for (int i = start_index-left_index; i < size_local_sieve; i += prime){
            if((((i + left_index + 1) * 2) + 1) == prime){
                continue;
            }
            local_array[i] = false;
        }  
    }
    //printf("End sieve %ld \n", s);
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

    //printf("P = %ld, Left: %d , Right: %d, Size: %d \n", s, left_part, right_part, local_sieve_size);



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
    // We have to skip the first sqrt n in array of processor 0, this causes slight inbalance in workload
    // int sqrt_is_even;
    // if (s==0){
    //     sqrt_is_even = (int)ceil(sqrt(n)) % 2 == 0;
    //     printf("Even square? %d", sqrt_is_even);
    // }
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
    //printf("processor %ld: \n", s);

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

    int total_primes = 1;
    for (int m = 0; m < P; ++m){
        total_primes += num_primes_result[m];
    }
    if (s==0){
        printf("Total Primes found: %d \n", total_primes);
    }

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
int find_primes_sieve(int limit, long num_procs, int argc, char **argv){
    n = limit;
    P = num_procs;

    if(n < 2){
        printf("There are no primes below 2 \n");
        return EXIT_FAILURE;
    }
    bsp_init(&bsp_sieve, argc, argv);

    if (P > ceil(floor((n-1)/2))/floor(sqrt(n))){

        P = ceil(floor((n-1)/2))/floor(sqrt(n));
        if(P==0){
            P=1;
        }
    }
    bsp_sieve();
    return EXIT_SUCCESS;

}

void run_experiment_sieve(int num_runs, int limit, long num_procs, int argc, char**argv){
    struct timespec begin, end;
    double timings[num_runs];

    for (int i = 0; i < num_runs; ++i){
        clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
        find_primes_sieve(limit, num_procs, argc, argv);
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
         
        timings[i] = (end.tv_nsec - begin.tv_nsec) / 1000000000.0 + (end.tv_sec  - begin.tv_sec);
    }

    double average_timing = 0;
    for (int j = 0; j < num_runs; ++j){
        average_timing += timings[j];
    }
    free(timings);
    average_timing /= num_runs;
    printf("Average Time: %f", average_timing);
}

int main( int argc, char ** argv ) {
    long num_procs = bsp_nprocs();
    int limit = 10000;
    run_experiment_sieve(100, limit, num_procs, argc, argv);


    return EXIT_SUCCESS;
}