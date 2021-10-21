
#Uncomment the below to enable checkpointing under Linux, using http://dmtcp.sourceforge.net
#WITH_DMTCP=yes

#Debug flags:
DEBUGFLAGS=-O0 -g -DMCBSP_SHOW_PINNING -DMCBSP_ENABLE_FAKE_HP_DIRECTIVES -DMCBSP_ALLOW_MULTIPLE_REGS

#Optimisation flags:
OPT=${NOWARN} -O3 -funroll-loops -g -DNDEBUG -DMCBSP_ENABLE_HP_DIRECTIVES -DMCBSP_ALLOW_MULTIPLE_REGS
#OPT=${DEBUGFLAGS}

#Linkage flags for shared library build
#Linux:
SHARED_LINKER_FLAGS=-shared -Wl,-soname,libmcbsp.so.${MAJORVERSION} -o lib/libmcbsp.so.${VERSION}
C_STANDARD_FLAGS=-D_POSIX_C_SOURCE=200112L
#OS X:
#SHARED_LINKER_FLAGS=-Wl,-U,_main -dynamiclib -install_name "libmcbsp.${MAJORVERSION}.dylib" -current_version ${VERSION} -compatibility_version ${MAJORVERSION}.0 -o libmcbsp.${MAJORVERSION}.dylib
#C_STANDARD_FLAGS=

#add -fPIC when we compile with DMTCP
ifdef WITH_DMTCP
 OPT:=${OPT} -fPIC -DMCBSP_WITH_DMTCP
endif

#Compiler-specific flags
GCCFLAGS=-Wall -Wextra
LLVMFLAGS=-Weverything -Wno-padded -Wno-missing-noreturn -Wno-cast-align -Wno-covered-switch-default -Wno-unreachable-code -Wno-format-nonliteral -Wno-float-equal -Wno-disabled-macro-expansion -Wno-reserved-id-macro
ICCFLAGS=-Wall -Wextra -diag-disable 378

#GNU Compiler Collection:
CCEXEC=gcc
CPPEXEC=g++
CFLAGS:=${GCCFLAGS} -I. -DMCBSP_USE_SYSTEM_MEMCPY
CC=${CCEXEC} ${C_STANDARD_FLAGS} -pthread -std=c99 -mtune=native -march=native
CPPFLAGS:=-Weffc++ -Wall -Wextra -I. -DMCBSP_USE_SYSTEM_MEMCPY
CPP=${CPPEXEC} -pthread -std=c++98 -mtune=native -march=native
CPP11=${CPPEXEC} -pthread -std=c++11 -mtune=native -march=native
LFLAGS:=`${RELPATH}deplibs.sh ${CC}`
AR=ar

#clang LLVM compiler:
#CCEXEC=clang
#CPPEXEC=clang++
#CFLAGS:=${LLVMFLAGS} -I.
#CC=${CCEXEC} ${C_STANDARD_FLAGS} -std=c99
#CPPFLAGS:=${LLVMFLAGS} -I.
#CPP=${CPPEXEC} -std=c++98
#CPP11=${CPPEXEC} -std=c++11 -Wno-c++98-compat
#LFLAGS:=`${RELPATH}deplibs.sh ${CC}`
#AR=ar

#Intel C++ Compiler:
#CCEXEC=icc
#CPPEXEC=${CCEXEC}
#CFLAGS:=${ICCFLAGS} -I.
#CC=${CCEXEC} ${C_STANDARD_FLAGS} -std=c99 -xHost
#CPPFLAGS:=${ICCFLAGS} -I.
#CPP=${CPPEXEC} -xHost
#CPP11=${CPPEXEC} -std=c++11 -xHost
#LFLAGS:=`${RELPATH}deplibs.sh ${CC}`
#AR=ar

#MinGW 32-bit Windows cross-compiler:
#CCEXEC=i686-w64-mingw32-gcc
#CPPEXEC=i686-w64-mingw32-g++
#PTHREADS_WIN32_PATH=pthreads-w32/
#CFLAGS:=${GCCFLAGS} -I. -I${PTHREADS_WIN32_PATH}include
#CC=${CCEXEC} ${C_STANDARD_FLAGS} -std=c99
#CPPFLAGS:=${GCCFLAGS} -I. -I${PTHREADS_WIN32_PATH}include
#CPP=${CPPEXEC} -std=c++98
#CPP11=${CPPEXEC} -std=c++11
#LFLAGS:=${RELPATH}${PTHREADS_WIN32_PATH}lib/x86/libpthreadGC2.a
#AR=i686-w64-mingw32-ar

#MinGW 64-bit Windows cross-compiler:
#CCEXEC=x86_64-w64-mingw32-gcc
#CPPEXEC=x86_64-w64-mingw32-g++
#NOWARN=
#PTHREADS_WIN32_PATH=pthreads-w32/
#CFLAGS:=${GCCFLAGS} -I. -I${PTHREADS_WIN32_PATH}include
#CC=${CCEXEC} ${C_STANDARD_FLAGS} -std=c99
#CPPFLAGS:=${GCCFLAGS} -I. -I${PTHREADS_WIN32_PATH}include
#CPP=${CPPEXEC} -std=c++98
#CPP11=${CPPEXEC} -std=c++11
#LFLAGS:=${RELPATH}${PTHREADS_WIN32_PATH}lib/x64/libpthreadGC2.a
#AR=x86_64-w64-mingw32-ar

OBJECTS=mcutil.o mcbsp.o bsp.o bsp.cpp.o
DAPOBJS=bsp.debug.o bsp.profile.o
TESTOBJ=tests/internal.o tests/util.o tests/bsp.o tests/abort.o tests/spmd.o tests/hp.o tests/regs.o
CLEAN_OBJECTS=${OBJECTS} ${OBJECTS:%.o=%.shared.o} ${KROBJECTS} ${TESTOBJ} ${DAPOBJS} ${DAPOBJS:%.o=%.shared.o} mcutil.debug.o mcbsp.debug.o
CLEAN_EXECS=

#To add specific hardware support, simply include the auxiliary file here.
#
#For example, if our machine has the accelerator `foo' installed, and a
#plugin for the foo accelerator is distributed with MulticoreBSP in the
#base directory ./accelerators/foo/*, then add the following line:
#
#include ${RELPATH}accelerators/foo/include.mk

%.shared.o: %.c %.h
	${CC} -fPIC ${OPT} ${CFLAGS} -c -o $@ $(^:%.h=)

%.shared.o: %.c
	${CC} -fPIC ${OPT} ${CFLAGS} -c -o $@ $^

%.debug.o: %.c %.h
	${CC} ${DEBUGFLAGS} -DMCBSP_DISABLE_COMPILATION_MODE_WARNING ${CFLAGS} -c -o $@ $(^:%.h=)

%.o: %.c %.h
	${CC} ${OPT} ${CFLAGS} -c -o $@ $(^:%.h=)

%.o: %.c
	${CC} ${OPT} ${CFLAGS} -c -o $@ $^

%.cpp.o: %.cpp %.hpp
	${CPP} ${OPT} ${CPPFLAGS} -c -o $@ $(^:%.hpp=)

%.cpp.o: %.cpp
	${CPP} ${OPT} ${CPPFLAGS} -c -o $@ $^

%.cpp.shared.o: %.cpp %.hpp
	${CPP} -fPIC ${OPT} ${CPPFLAGS} -c -o $@ $(^:%.hpp=)

%.cpp.shared.o: %.cpp
	${CPP} -fPIC ${OPT} ${CPPFLAGS} -c -o $@ $^

%.cpp11.o: %.cpp %.hpp
	${CPP11} ${OPT} ${CPPFLAGS} -c -o $@ $(^:%.hpp=)

%.cpp11.o: %.cpp
	${CPP11} ${OPT} ${CPPFLAGS} -c -o $@ $^

%.cpp11.shared.o: %.cpp %.hpp
	${CPP11} -fPIC ${OPT} ${CPPFLAGS} -c -o $@ $(^:%.hpp=)

%.cpp11.shared.o: %.cpp
	${CPP11} -fPIC ${OPT} ${CPPFLAGS} -c -o $@ $^

