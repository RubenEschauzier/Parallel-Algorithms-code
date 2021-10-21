/*
 * Copyright (c) 2015
 *
 * Created 2nd of April, 2015 by Albert-Jan N. Yzelman
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

#ifndef _H_MCBSP_INTERNAL_MACROS
#define _H_MCBSP_INTERNAL_MACROS

#ifdef __GNUC__
 #ifndef _GNU_SOURCE
  #define _GNU_SOURCE
 #endif
#endif

#ifdef __DOXYGEN__
 /** 
  * Sets bsp_put and bsp_get to substitute as implementations
  * for bsp_hpput and bsp_hpget, respectively.
  * If this flag is not set, bsp_hpput and bsp_hpget will not
  * be defined. Note that according to the standard, the
  * regular bsp_put and bsp_get are indeed valid replacements
  * for bsp_hpput and bsp_hpget-- they just are not more
  * performant.
  *
  * This macro takes precendence over the 
  * #MCBSP_ENABLE_HP_DIRECTIVES macro.
  *
  * This macro does not affect the MulticoreBSP binaries--
  * each compilation unit can define either of
  * #MCBSP_ENABLE_HP_DIRECTIVES and
  * #MCBSP_ENABLE_FAKE_HP_DIRECTIVES
  * and link against the same MulticoreBSP library.
  */
 #define MCBSP_ENABLE_FAKE_HP_DIRECTIVES

 /**
  * Uses standard POSIX mutexes for inter-thread
  * synchronization. If this is undefined, the
  * #MCBSP_USE_SPINLOCK macro will be assumed instead.
  *
  * Switching between this macro and #MCBSP_USE_SPINLOCK
  * requires re-compilation of the MulticoreBSP library.
  * By default, the MulticoreBSP debug mode uses mutex-
  * based synchronisations-- profiling and performance
  * mode define #MCBSP_USE_SPINLOCK.
  */
 #define MCBSP_USE_MUTEXES

 /**
  * Uses spin-locks instead of standard PThreads mutexes
  * and conditions. Less energy-efficient, but faster.
  * The #MCBSP_USE_MUTEXES macro, when set, takes
  * precedence over this macro.
  *
  * Switching between this macro and #MCBSP_USE_MUTEXES
  * requires re-compilation of the MulticoreBSP library.
  * By default, the MulticoreBSP debug mode define
  * #MCBSP_USE_MUTEXES-- profiling and performance use
  * spinlocking instead.
  */
 #define MCBSP_USE_SPINLOCK

 /**
  * A memcpy will always translate to the standard system
  * memcpy instead of a custom implementation as per the
  * #MCBSP_USE_CUSTOM_MEMCPY flag.
  * If this flag is not defined, #MCBSP_USE_CUSTOM_MEMCPY
  * will be assumed instead.
  *
  * Switching between this and #MCBSP_USE_CUSTOM_MEMCPY
  * requires re-compilation of the MulticoreBSP library.
  */
 #define MCBSP_USE_SYSTEM_MEMCPY

 /**
  * Enables a custom implementation of memcpy that can
  * lead to faster codes than standard implementations
  * because the custom implementation can be compiled to
  * run on your target architecture specifically, and may
  * benefit from, for example, architecture-specific
  * vectorisation capabilities that can speed up memory
  * copying.
  * If disabled, MulticoreBSP for C will rely on the
  * standard system memcpy instead.
  *
  * The #MCBSP_USE_SYSTEM_MEMCPY takes precendence over
  * this flag.
  *
  * Switching between this and #MCBSP_USE_SYSTEM_MEMCPY
  * requires re-compilation of the MulticoreBSP library.
  */
 #define MCBSP_USE_CUSTOM_MEMCPY

 /**
  * Allows multiple registrations of the same memory address.
  * 
  * The asymptotic running times of bsp_push_reg, bsp_pop_reg,
  * all communication primitives, and the synchronisation
  * methods, are unaffected by this flag.
  * However, it does introduce run-time overhead as well as
  * memory overhead. If your application(s) do not use
  * multiple registrations of the same variable, you may
  * consider compiling MulticoreBSP for C with this flag
  * turned off for slightly enhanced performance.
  *
  * Note the original BSPlib standard does demand multiple
  * registrations be possible. Thus if 
  * MCBSP_COMPATIBILITY_MODE is set while this flag was not
  * the resulting MulticoreBSP does not behave as per the
  * standard.
  *
  * Switching this flag on or off requires re-compilation
  * of the MulticoreBSP library.
  */
 #define MCBSP_ALLOW_MULTIPLE_REGS

 /**
  * Define MCBSP_NO_CHECKS to disable all run-time sanity
  * checks, except those regarding memory allocation.
  *
  * Switching this flag on or off requires re-compilation
  * of the MulticoreBSP library.
  */
 #define MCBSP_NO_CHECKS

 /**
  * Enables high-performance variants of bsp_put, bsp_get,
  * and bsp_send. The high-performance variant of bsp_move
  * is always enabled. If this pragma is not defined,
  * the MCBSP_ENABLE_FAKE_HP_DIRECTIVES pragma will be set
  * automatically.
  *
  * This macro does not affect the MulticoreBSP binaries--
  * each compilation unit can define either of
  * #MCBSP_ENABLE_HP_DIRECTIVES and
  * #MCBSP_ENABLE_FAKE_HP_DIRECTIVES
  * and link against the same MulticoreBSP library.
  */
 #define MCBSP_ENABLE_HP_DIRECTIVES

 /**
  * If left undefined, compilation in debug or profile mode
  * will generate a warning.
  */
 #define MCBSP_DISABLE_COMPILATION_MODE_WARNING
#endif

/**
 * When MulticoreBSP for C allocates memory, it aligns it to the following
 * boundary (in bytes).
 */
#define MCBSP_ALIGNMENT 128

/**
 * If enabled, then during the handling of communication buffers during a
 * bsp_sync, a congestion-avoiding scheduling is used.
 */
#define MCBSP_CA_SYNC

#ifndef MCBSP_USE_MUTEXES
 #define MCBSP_USE_SPINLOCK
#else
 #ifdef MCBSP_USE_SPINLOCK
  #pragma message "Warning: MCBSP_USE_MUTEXES takes precedence over MCBSP_USE_SPINLOCK"
  #undef MCBSP_USE_SPINLOCK
 #endif
#endif

#ifdef MCBSP_USE_SYSTEM_MEMCPY
 #ifdef MCBSP_USE_CUSTOM_MEMCPY
  #pragma message "Warning: MCBSP_USE_SYSTEM_MEMCPY takes precedence over MCBSP_USE_CUSTOM_MEMCPY"
  #undef MCBSP_USE_CUSTOM_MEMCPY
 #endif
#else
 #define MCBSP_USE_CUSTOM_MEMCPY
#endif

#endif

