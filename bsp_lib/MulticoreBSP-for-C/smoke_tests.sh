#!/bin/bash
if [ ! -f machine.info ]; then
	echo "Copying default machine.info for testing..."
	cp tests/machine.info ./
else
	echo "====================================================="
	echo "An existing machine.info file was found; see below."
	echo "This configuration will be used for the tests."
	echo "-----------------------------------------------------"
	cat machine.info
	echo "====================================================="
	echo " "
fi

echo "====================================================="
echo "Testing internal functions:"
echo "-----------------------------------------------------"
./test_internal
echo "====================================================="
echo " "

echo "====================================================="
echo "Testing utility functions:"
echo "-----------------------------------------------------"
./test_util > /dev/null;
if [ "$?" -ne "0" ];
	then echo "SUCCESS";
else
	echo "FAILURE";
fi
echo "====================================================="
echo " "

echo "====================================================="
echo "Testing BSPlib functions:"
echo "-----------------------------------------------------"
./test_bsp
echo "====================================================="
echo " "

echo "====================================================="
echo "Testing bsp_abort:"
echo "-----------------------------------------------------"
./test_abort 2> tests/test_abort.out; diff tests/test_abort.out tests/test_abort.chk;
if [ "$?" -eq 0 ]; then
	echo "SUCCESS";
else
	echo "FAILURE";
fi
echo "====================================================="
echo " "

echo "====================================================="
echo "Testing main as the SPMD point of entry:"
echo "-----------------------------------------------------"
./test_spmd
echo "====================================================="
echo " "

echo "====================================================="
echo "Testing high-performance RDMA primitives:"
echo "-----------------------------------------------------"
./test_hp
echo "====================================================="
echo " "

echo "====================================================="
echo "Testing multiple registrations:"
echo "-----------------------------------------------------"
./test_regs > tests/test_regs.out
if [ "$?" -eq 0 ]; then
	echo "SUCCESS";
else
	echo "FAILURE";
fi
echo "====================================================="
echo " "

