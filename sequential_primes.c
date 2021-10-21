#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
// Returning the array makes program significantly slower! Consider returning only primes instead of full array
// Bitwise sieve here: https://bcu.copsewood.net/dsalg/bitwise/bitwise.html, including way to exclude even numbers

// bool* sieve_non_even(int N){
//     bool *isPrime = malloc(sizeof(bool)*N);
//     memset(isPrime, true, N*sizeof(bool));

//     isPrime[0] = false; isPrime[1] = false;
//     for (int i=1; i < ceil(sqrt(N)); i+=2){
//         printf("Even number: {%d} \n", i+1);
//         isPrime[i+1] = false;
//         if(isPrime[i] == true){
//             for (int j=i*i; j < N; j+=i){
//                 isPrime[j] = false;
//             }
//         }
//     }
//     for (int z = ceil(sqrt(N)))
//     // for (int k=0; k<N;++k){
//     //     if(isPrime[k]){
//     //         printf("%d ", k);
//     //     }
//     // }
//     return isPrime;
// }
bool* sieve_non_even(int N){
    // Error in this code!!!! Unneccesairy computations due to ordering of division and sqrt
    int size_non_even = ceil(N/2.0);
    bool *isPrime = malloc(sizeof(bool)*size_non_even);

    memset(isPrime, true, size_non_even*sizeof(bool));

    isPrime[0] = false;
    for (int i=1; i < ceil(sqrt(size_non_even)); i+=1){
        if(isPrime[i] == true){
            for (int j=i+(i*2)+1; j < size_non_even; j+=(i*2)+1){
                isPrime[j] = false;
            }
        }
    }
    return isPrime;
}

bool* sieve(int N){
    bool *isPrime = malloc(sizeof(bool)*N);
    memset(isPrime, true, N*sizeof(bool));

    isPrime[0] = false; isPrime[1] = false;
    for (int i=2; i < ceil(sqrt(N)); ++i){
        if(isPrime[i] == true){
            for (int j=i*i; j < N; j+=i){
                isPrime[j] = false;
            }
        }
    }
    return isPrime;
}

void time_sieve(int num_runs, int N){
    clock_t begin_run_1 = clock();
    for(int i = 0; i < num_runs; ++i){
        sieve(N);
    }
    clock_t end_run_1 = clock();

    clock_t begin_run_2 = clock();
    for(int j=0; j<num_runs;++j){
        sieve_non_even(N);
    }
    clock_t end_run_2 = clock();

    double time_spent_1 = (double)(end_run_1 - begin_run_1) / CLOCKS_PER_SEC;
    double time_spent_2 = (double)(end_run_2 - begin_run_2) / CLOCKS_PER_SEC;

    printf("Experiment 1: Include even numbers, Average time spent: %f \n", time_spent_1/num_runs);
    printf("Experiment 2: Exclude even numbers, Average time spent: %f \n", time_spent_2/num_runs);


}
int test_validity( bool* (*passed_sieve) (int)){
    #define test_range 101
    #define num_real_primes 26

    int realPrimes[num_real_primes] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101};
    int found_primes[num_real_primes];

    // Add 2 always, because we exclude even numbers in our sieve
    found_primes[0] = 2;
    // Apply sieve over test range
    bool *sieve_result = passed_sieve(test_range);

    // Test if number of primes found is correct, if not return failure
    int num_primes_found = 1;
    for (int k=0; k < ceil(test_range/2.0); ++k){
        num_primes_found += sieve_result[k];
    }

    if (num_primes_found != num_real_primes){
        printf("Primes found: %d, Real Number Primes: %d \n", num_primes_found, num_real_primes);
        return EXIT_FAILURE;
    }

    // Extract prime numbers from sieve array
    int index_prime = 1;
    for (int i=0; i < ceil(test_range/2.0); ++i){
        if(sieve_result[i]){
            found_primes[index_prime] = (i*2)+1;
            index_prime += 1;
        }
    }

    // Test if found primes are equal to real primes
    for (int j=0; j<num_real_primes;++j){
        if (found_primes[j] != realPrimes[j]){
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

void main(int argc, char*argv[]){
    int limit = 1000;
    bool pattern[2] = {false, true};
    // sieve_non_even(limit, pattern);
    time_sieve(10000, 100000);    
    int outcome = test_validity(sieve_non_even);
    if(outcome==0){
        printf("Find Primes(N=%d): PASSED. \n", test_range);
    }
    else{
        printf("Find Primes(N=%d): FAILED. \n", test_range);
    }
}