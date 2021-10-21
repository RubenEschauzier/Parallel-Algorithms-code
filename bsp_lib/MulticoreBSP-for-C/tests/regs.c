/*
 * Copyright (c) 2018
 *
 * File created September 30th 2018 by Albert-Jan N. Yzelman
 *
 * This file is part of MulticoreBSP in C --
 *        a port of the original Java-based MulticoreBSP.
 *
 * MulticoreBSP for C is distributed as part of the original
 * MulticoreBSP and is free software: you can redistribute
 * it and/or modify it under the terms of the GNU Lesser 
 * General Public License as published by the Free Software 
 * Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 * MulticoreBSP is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A 
 * PARTICULAR PURPOSE. See the GNU Lesser General Public 
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General 
 * Public License along with MulticoreBSP. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <bsp.h>
#include <mcbsp.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 31
#define M 33

#define PRE_PUSH_SYNC 0
#define POST_PUSH_SYNC 1

static void check( const int stage, const size_t addresses, const size_t versions ) {
	const struct mcbsp_thread_data * const thread_data = pthread_getspecific( mcbsp_internal_thread_data );
	if( stage == PRE_PUSH_SYNC ) {
		const size_t expect = addresses * versions;
		if( thread_data->localsToPush.top != expect ) {
			(void) fprintf( stderr, "%zu: expected %zd items in localsToPush, found %zd instead.\n", thread_data->bsp_id, expect, thread_data->localsToPush.top );
		}
	} else if( stage == POST_PUSH_SYNC ) {
		if( thread_data->local2global.size != addresses ) {
			(void) fprintf( stderr, "%zu: expected %zd items in local2global, found %zd instead.\n", thread_data->bsp_id, addresses, thread_data->local2global.size );
		}
		if( thread_data->bsp_id == 0 ) {
			if( thread_data->local2global.size >= thread_data->init->global2local.cap ) {
				(void) fprintf( stderr, "global2local table has smaller capacity (%zd) than the number of global addresses registered (%zd).\n", thread_data->init->global2local.cap, thread_data->local2global.size );
			}
			for( size_t global = 0; global < thread_data->local2global.size; ++global ) {
				for( size_t version = 0; version < thread_data->init->global2local.P; ++version ) {
					const size_t ss = thread_data->init->global2local.table[ global ][ version ].top;
					if( ss != versions ) {
						(void) fprintf( stderr, "global2local table has stack size %zd instead of %zd at %zd, %zd\n", ss, versions, global, version );
					}
				}
			}
		}
	} else {
		(void) fprintf( stderr, "Control flow should never reach this statement!\n" );
		fflush( stderr );
		abort();
	}
}

int main( int argc, char ** argv ) {
	(void)argc;
	(void)argv;

	bsp_begin( bsp_nprocs() );

	const unsigned int s = bsp_pid();

	//initialise random addresses; just to make sure they do not point to the same
	//address, unintentionally
	void * addresses[N];
	for( size_t i = 0; i < N; ++i ) {
		addresses[i] = (void*)(N + i);
	}

	if( s == 0 ) {
		(void) printf( "Testing multiple calls to bsp_push_reg and bsp_pop_reg: both multiple addresses and multiple registrations of the same address; initialization is done, will now execute various variants...\n" ); fflush(stdout);
	}

	if( s == 0 ) {
		(void) printf( "Single test: " ); fflush(stdout);
	}

	//straight single register & deregister test
	for( size_t i = 0; i < N; ++i ) {
		bsp_push_reg( addresses[i], i );
	}

	if( s == 0 ) {
		(void) printf( "registered " );
	}

	check( PRE_PUSH_SYNC, N, 1 );
	bsp_sync();
	check( POST_PUSH_SYNC, N, 1 );

	if( s == 0 ) {
		(void) printf( "synced " ); fflush(stdout);
	}

	for( size_t i = 0; i < N; ++i ) {
		bsp_pop_reg( addresses[i] );
	}

	if( s == 0 ) {
		(void) printf( "deregistered " ); fflush(stdout);
	}

	bsp_sync();

	if( s == 0 ) {
		(void) printf( "done.\nHorizontal test: " ); fflush(stdout);
	}

	//horizontal multiple register & deregister test
	for( size_t j = 0; j < M; ++j ) {
		for( size_t i = 0; i < N; ++i ) {
			bsp_push_reg( addresses[i], j*N + i );
		}
	}

	if( s == 0 ) {
		(void) printf( "registered " ); fflush(stdout);
	}

	check( PRE_PUSH_SYNC, N, M );
	bsp_sync();
	check( POST_PUSH_SYNC, N, M );

	if( s == 0 ) {
		(void) printf( "synced " ); fflush(stdout);
	}

	for( size_t j = 0; j < M; ++j ) {
		for( size_t i = 0; i < N; ++i ) {
			bsp_pop_reg( addresses[i] );
		}
	}

	if( s == 0 ) {
		(void) printf( "deregistered " ); fflush(stdout);
	}

	bsp_sync();

	if( s == 0 ) {
		(void) printf( "done.\nVertical test: " ); fflush(stdout);
	}

	//vertical multiple register & deregister test
	for( size_t i = 0; i < N; ++i ) {
		for( size_t j = 0; j < M; ++j ) {
			bsp_push_reg( addresses[i], j*N + i );
		}
	}

	if( s == 0 ) {
		(void) printf( "registered " ); fflush(stdout);
	}

	check( PRE_PUSH_SYNC, N, M );
	bsp_sync();
	check( POST_PUSH_SYNC, N, M );

	if( s == 0 ) {
		(void) printf( "synced " ); fflush(stdout);
	}

	for( size_t i = 0; i < N; ++i ) {
		for( size_t j = 0; j < M; ++j ) {
			bsp_pop_reg( addresses[i] );
		}
	}

	if( s == 0 ) {
		(void) printf( "deregistered " ); fflush(stdout);
	}

	bsp_sync();
	
	if( s == 0 ) {
		(void) printf( "done.\nMixed test 1: " ); fflush(stdout);
	}

	//mixed-orientation multiple register & deregister test #1
	for( size_t i = 0; i < N; ++i ) {
		for( size_t j = 0; j < M; ++j ) {
			bsp_push_reg( addresses[i], j*N + i );
		}
	}

	if( s == 0 ) {
		(void) printf( "registered " ); fflush(stdout);
	}

	check( PRE_PUSH_SYNC, N, M );
	bsp_sync();
	check( POST_PUSH_SYNC, N, M );

	if( s == 0 ) {
		(void) printf( "synced " ); fflush(stdout);
	}

	for( size_t j = 0; j < M; ++j ) {
		for( size_t i = 0; i < N; ++i ) {
			bsp_pop_reg( addresses[i] );
		}
	}

	if( s == 0 ) {
		(void) printf( "deregistered " ); fflush(stdout);
	}

	bsp_sync();

	if( s == 0 ) {
		(void) printf( "done.\nMixed test 2: " ); fflush(stdout);
	}

	//mixed-orientation multiple register & deregister test #2
	for( size_t j = 0; j < M; ++j ) {
		for( size_t i = 0; i < N; ++i ) {
			bsp_push_reg( addresses[i], j*N + i );
		}
	}

	if( s == 0 ) {
		(void) printf( "registered " ); fflush(stdout);
	}

	check( PRE_PUSH_SYNC, N, M );
	bsp_sync();
	check( POST_PUSH_SYNC, N, M );

	if( s == 0 ) {
		(void) printf( "synced " ); fflush(stdout);
	}

	for( size_t i = 0; i < N; ++i ) {
		for( size_t j = 0; j < M; ++j ) {
			bsp_pop_reg( addresses[i] );
		}
	}

	if( s == 0 ) {
		(void) printf( "deregistered " ); fflush(stdout);
	}

	bsp_sync();

	if( s == 0 ) {
		(void) printf( "done.\nTests complete!\n" ); fflush(stdout);
	}

	bsp_end();
	return 0;
}

