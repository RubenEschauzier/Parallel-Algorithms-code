RELPATH=./

include include.mk
include version.mk

.PHONY: all library library shared-library help tests tools jni clean

all: library shared-library tools

library: ${OBJECTS} ${DAPOBJS}
	mkdir include || true
	mkdir lib || true
	cp bsp.h include/
	cp bsp.hpp include/
	cp mcbsp-affinity.h include/
	cp mcbsp-profiling.h include/
	cp mcbsp-resiliency.h include/
	cp mcbsp-templates.hpp include/
	${AR} rcs lib/libmcbsp${VERSION}.a ${OBJECTS} ${DAPOBJS}

shared-library: ${OBJECTS:%.o=%.shared.o} ${DAPOBJS:%.o=%.shared.o}
	mkdir lib || true
	${CPP} ${SHARED_LINKER_FLAGS} ${OBJECTS:%.o=%.shared.o} ${DAPOBJS:%.o=%.shared.o} ${LFLAGS}

help:
	@echo " "
	@echo "MulticoreBSP for C make targets:"
	@echo "================================"
	@echo " "
	@echo "Default targets: library, shared-library, tools"
	@echo " "
	@echo "Targets:"
	@echo "-------------"
	@echo "       library: builds libmcbsp, statically linked version"
	@echo "shared-library: builds libmcbsp, dynamically linked version"
	@echo "         tests: builds and starts testing applications"
	@echo "         tools: builds the MulticoreBSP default tools (bspcc and bsprun)"
	@echo "    include.mk: initialises the build environment. See that file for detailed build options."
	@echo "  machine.info: generates a recommended machine.info configuration file, using user input."
	@echo "         clean: removes all files created by compilation."
	@echo " "
	@echo "Note that this help listing does not display any additional targets some of the build options might enable."
	@echo " "

bsp.o: bsp.c bsp.h mcbsp.h
	${CC} ${OPT} ${CFLAGS} -c -o $@ $(^:%.h=)

bsp.debug.o: bsp.c bsp.h mcbsp.h
	${CC} ${DEBUGFLAGS} ${CFLAGS} -DMCBSP_USE_MUTEXES -DMCBSP_MODE=2 -DMCBSP_DISABLE_COMPILATION_MODE_WARNING -c -o $@ bsp.c

bsp.profile.o: bsp.c bsp.h mcbsp.h
	${CC} ${OPT} ${CFLAGS} -DMCBSP_MODE=3 -DMCBSP_DISABLE_COMPILATION_MODE_WARNING -c -o $@ bsp.c

bsp.shared.o: bsp.c bsp.h mcbsp.h
	${CC} -fPIC ${CFLAGS} -c -o $@ $(^:%.h=)

bsp.debug.shared.o: bsp.c bsp.h mcbsp.h
	${CC} -fPIC ${DEBUGFLAGS} ${CFLAGS} -DMCBSP_USE_MUTEXES -DMCBSP_MODE=2 -DMCBSP_DISABLE_COMPILATION_MODE_WARNING -c -o $@ bsp.c

bsp.profile.shared.o: bsp.c bsp.h mcbsp.h
	${CC} -fPIC ${OPT} ${CFLAGS} -DMCBSP_MODE=3 -DMCBSP_DISABLE_COMPILATION_MODE_WARNING -c -o $@ bsp.c

CLEAN_EXECS+=test_internal test_util test_bsp test_abort tests/test_abort.out test_spmd test_hp test_regs

tests: mcbsp.o mcutil.o bsp.o mcbsp.debug.o mcutil.debug.o
	$(MAKE) ${TESTOBJ}
	${CC} ${CFLAGS} -o test_internal tests/internal.o mcbsp.o mcutil.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_util     tests/util.o     mcbsp.o mcutil.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_bsp      tests/bsp.o      mcbsp.o mcutil.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_abort	 tests/abort.o	  mcbsp.o mcutil.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_spmd     tests/spmd.o     mcbsp.o mcutil.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_hp       tests/hp.o       mcbsp.o mcutil.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_regs     tests/regs.o     mcbsp.o mcutil.o bsp.o ${LFLAGS}
	rm -f tests/internal.o tests/util.o tests/bsp.o tests/abort.o tests/test_abort.out tests/spmd.o tests/hp.o tests/regs.o
	./smoke_tests.sh
	$(MAKE) OPT="${DEBUGFLAGS}" ${TESTOBJ}
	${CC} ${CFLAGS} -o test_internal tests/internal.o mcbsp.debug.o mcutil.debug.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_util     tests/util.o     mcbsp.debug.o mcutil.debug.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_bsp      tests/bsp.o      mcbsp.debug.o mcutil.debug.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_abort	 tests/abort.o	  mcbsp.debug.o mcutil.debug.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_spmd     tests/spmd.o     mcbsp.debug.o mcutil.debug.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_hp       tests/hp.o       mcbsp.debug.o mcutil.debug.o bsp.o ${LFLAGS}
	${CC} ${CFLAGS} -o test_regs     tests/regs.o     mcbsp.debug.o mcutil.debug.o bsp.o ${LFLAGS}
	rm -f tests/internal.o tests/util.o tests/bsp.o tests/abort.o tests/test_abort.out tests/spmd.o tests/hp.o tests/regs.o
	./smoke_tests.sh

include.mk: include.default
	cp include.default include.mk

tools: tools/bspcc tools/bspcxx tools/bsprun
	chmod +x tools/bsprun

tools/bspcc: bspcc.in deplibs.sh library
	echo "#!/bin/bash" > $@
	echo "MCBSP_PATH=\"`pwd`/\"" >> $@
	echo "MCBSP_LFLAGS=\"\$${MCBSP_PATH}lib/libmcbsp${VERSION}.a `./deplibs.sh ${CC}`\"" >> $@
	echo "BSP_LANG=\"c99\"" >> $@
	echo "PREF_CC=\"${CCEXEC}\"" >> $@
	echo "PREF_CXX=\"${CPPEXEC}\"" >> $@
	cat bspcc.in >> $@
	chmod +x $@

tools/bspcxx: deplibs.sh library
	echo "#!/bin/bash" > $@
	echo "MCBSP_PATH=\"`pwd`/\"" >> $@
	echo "MCBSP_LFLAGS=\"\$${MCBSP_PATH}lib/libmcbsp${VERSION}.a `./deplibs.sh ${CPP}`\"" >> $@
	echo "BSP_LANG=\"c++98\"" >> $@
	echo "PREF_CC=\"${CCEXEC}\"" >> $@
	echo "PREF_CXX=\"${CPPEXEC}\"" >> $@
	cat bspcc.in >> $@
	chmod +x $@

jni:
	mkdir jni || true
	cp *.c jni/
	cp *.h jni/
	cp Android.in jni/Android.mk
	mv jni/mcbsp-default-hooks.h jni/bsp-active-hooks.h

machine.info: | machine.info benchmarks
	(cd benchmarks; make generate_machine_info)
	benchmarks/generate_machine_info; \
	touch machine.info

mcbsp.h: bsp-active-hooks.h

bsp-active-hooks.h: mcbsp-default-hooks.h
	cp $^ $@

#To add specific hardware support, simply include the auxiliary targets
#makefile here.
#
#For example, if our machine has the accelerator `foo' installed, and a
#plugin for the foo accelerator is distributed with MulticoreBSP in the
#base directory ./accelerators/foo/*, then add the following line:
#
#include ${RELPATH}accelerators/foo/targets.mk

clean:
	rm -f ${CLEAN_OBJECTS}
	rm -f tests/test_abort.out
	rm -f ${CLEAN_EXECS}
	rm -rf include
	rm -rf lib
	rm -f tools/bspcc tools/bspcxx
	rm -f bsp-active-hooks.h

