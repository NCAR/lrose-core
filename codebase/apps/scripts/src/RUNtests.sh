#!/bin/sh
#
# (c) Copyright 1992 Conor P. Cahill (uunet!virtech!cpcahil).  
# You may copy, distribute, and use this software as long as this
# copyright statement is not removed.
#
# Name: RUNtests.sh
#
# Function:
#
#	This script executes a series of test programs specified on the 
#	command line. It checks that the test programs exist before trying
#	to execute them. A test is considered to have passed if it meets
#	the following conditions:
#		1) returns a 0 exit code to this script (i.e., exit(0))
#		2) does not write any text to stdout (i.e., no printf())
#
# Usage:
#
#	The tests to run must be in the current directory. You can specify
#	any number of tests to run on the command line:
#
#		% Runtests.sh testname testname1 testname2 testnamex
#
# Output:
#
#	Creates an output file called "Runtests.out" in the current directory
#	containing the error output (if any) from the tests. 
#
#	Returns a message as to success or failure of the test.
#
#	Script returns 0 if all tests pass. Returns 1 if any tests fail.
#
# Author: Deirdre Garvey	15-JUN-1994  (Modified "Runtests.sh" from
#					      above copyright source.)
## SCCS info
##   %W% %D% %T%
##   %F% %E% %U%
##
## RCS info
##   $Author: dixon $
##   $Locker:  $
##   $Date: 2014/10/26 20:06:12 $
##   $Id: RUNtests.sh,v 1.1 2014/10/26 20:06:12 dixon Exp $
##   $Revision: 1.1 $
##   $State: Exp $
##
##   $Log: RUNtests.sh,v $
##   Revision 1.1  2014/10/26 20:06:12  dixon
##   adding-from_awpg
##
##   Revision 3.0  1994/11/10 22:25:28  deirdre
##   Major revision. Release to FAATC/CRDAs
##
# Revision 2.1  1994/11/10  22:22:20  deirdre
# Initial checkin
#
# Revision 2.2  1994/06/22  18:53:10  deirdre
# Change comments to make clearer
#
##
#=================================================================================
#
# 	Parse command line input
#

if test $# -lt 1 ; then
	echo "Usage: RUNtests.sh testname [testname_1 .. testname_n]"
	echo " "
	exit 1
fi

NUMTESTS=$#
TESTLIST=$@

#
# 	Before we run any tests, lets make sure we have the test programs
# 	we need
#

for TESTNAME in ${TESTLIST} ; do
	if test -f ${TESTNAME} ; then
		echo "Okay, test program ${TESTNAME} exists."
	else
		echo "The test program ${TESTNAME} is not available."
		echo "Please run 'make test' before you run this shell."
		echo "Exiting..."
		exit 1
	fi
done

#
#	Need the timeout execution wrapper to prevent infinite loops
#

echo "need timeout executable to run the tests..."

#
#	Create temporary output files
#

OUT=Runtests.out
TMPOUT=/tmp/err.$$
TMPFILE=/tmp/ttt.$$
failed=0

rm -f $OUT
cat <<endcat > $OUT

This file contains the outputs from the tests run by the Runtests script.
For more info on a particular test, check the README file

endcat

#
# 	Run each test and verify that it had no output and 
#	returned a zero exit code.
#

for TESTNAME in ${TESTLIST} ; do

	echo "************ Running test: ${TESTNAME} ..." >> $OUT

	rm -f ${TMPOUT}

	# Check whether the test has an input data file

	if test -f ${TESTNAME}.infile ; then
		echo "Reading input data from file ${TESTNAME}.input"
		./${TESTNAME} < ${TESTNAME}.input > ${TMPOUT} 2>&1
	else
		./${TESTNAME} > ${TMPOUT} 2>&1
	fi

	TEST_RETURN=$?

	if [ ${TEST_RETURN} != 0 ] && [ -s ${TMPOUT} ]; then
		echo "FAILED ${TESTNAME} test"
		echo "FAILED ${TESTNAME} test" >> $OUT
		failed=`expr $failed + 1`
	fi
done

cat ${TMPOUT} >> $OUT

#
#	Cleanup
#

#rm -f ${TMPOUT} ${TMPFILE}

if [ $failed = 0 ]; then

	echo "All tests seem to have passed.  Review $OUT to make sure"

fi

rm -f core a.out

#
#	Done
#

exit $failed




