#!/bin/sh
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2001 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2001/11/19 23:15:6 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Name: Runtests.sh
#
# Function:
#
#	This script executes the test programs for the current directory.
#	This may be accomplished in one of two ways:
#		1) through the RUNtests.sh script (default)
#		2) as a custom test script for the current directory
#
#	By default, this script executes the test programs specified 
#	within it by running the RUNtests.sh script which takes the test 
#	program names as input. 
#
#	RUNtests.sh
#	-----------
#	RUNtests.sh checks that the test programs exist before trying
#	to execute them. A test is considered to have passed if it meets
#	the following conditions:
#		1) returns a 0 exit code to this script (i.e., exit(0))
#		2) does not write any text to stdout (i.e., no printf())
#
#
# Usage:
#
#	The tests to run must be in the current directory.
#
#	To run Runtests.sh no command line arguments are expected. Enter:
#
#		% Runtests.sh
#
# Output:
#
#	Runtests.sh returns 0 if all tests pass. Returns 1 if any tests fail.
#
#	If this script uses RUNtests.sh, it also creates the output for
#	RUNtests.sh (see below).
#
#	RUNtests.sh
#	-----------
#	Creates an output file called "Runtests.out" in the current directory
#	containing the error output (if any) from the tests. 
#
#	RUNtests returns a message as to success or failure of the test;
#	it also returns 0 if all tests pass. Returns 1 if any tests fail.
#
#
# Author: Deirdre Garvey	22-JUN-1994
#
#=================================================================================
#
#	Test program names
#
CURRENT_DIR=`pwd`
TEST_PROGS="TEST_toolsa_str"

#
#	Location (if not in path) and name of test script
#
SCRIPT_LOC=/awpg/make_include
RUN_SCRIPT=RUNtests.sh

#
# 	Run tests and get return code
#
${SCRIPT_LOC}/${RUN_SCRIPT} ${TEST_PROGS}
TEST_RETURN=$?

#
#	Return return code from test script
#
exit ${TEST_RETURN}






