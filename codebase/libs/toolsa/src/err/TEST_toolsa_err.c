/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*
 * Name: TEST_toolsa_err.c
 *
 * Purpose:
 *
 *      To test the ERR module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/err.h>.
 *
 * Usage:
 *
 *       % TEST_toolsa_err
 *
 * Inputs: 
 *
 *       None
 *
 * Outputs:
 *
 *       Creates an output file named: TEST_toolsa_err.errlog
 *       Also expected an output file named: TEST_toolsa_err.testlog
 *       but this is not created.
 *
 * Author: Deirdre Garvey       06-JUN-1994
 *
 */

/*
 * include files
 */

#include <stdio.h>
#include <toolsa/err.h>

/*
 * definitions
 */


/*
 * test individual module subroutines
 */

/*================================================================================*/
void
TEST_ERRcontrol()
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine ERRcontrol(), as defined
   *      in <toolsa/err.h>
   */

  /*
   * Test that if use command line args with "-ERR",
   * calls to ERRcontrol() are ignored
   */


  /*
   * Test behavior is as expected.
   */

  ERRcontrol("OFF STD");
  fprintf(stderr, "This string should NOT appear in stderr\n");
  ERRcontrol("ON STD");
  fprintf(stderr, "This string should appear in stderr\n");
  ERRcontrol("OFF STD ON LOG");
  fprintf(stderr, "This string should NOT appear in stderr\n");
  fprintf(stderr, "This string should appear in a local log file\n");

  /*
   * Test bogus text strings into ERRcontrol()
   */

  ERRcontrol("OFF junk");
  ERRcontrol("ON junk");
  ERRcontrol("junk LOG");
  ERRcontrol("bogus junk");
  ERRcontrol("");
  ERRcontrol("/0");

  /*
   * Try to reset to "default" so can continue test
   */

  ERRcontrol("ON STD OFF LOG");
  fprintf(stderr, "This string should appear in stderr\n");

  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_ERRcontrol\n");
}


/*--------------------------------------------------------------------------------*/
void
TEST_ERRcontrolStr()
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine ERRcontrolStr(), as defined
   *      in <toolsa/err.h>
   */

  int max_str = 25;

  /*
   * Test behavior is as expected... But what is this??
   */


  /*
   * Test bogus input strings to ERRcontrolStr()
   */

  ERRcontrolStr("bogus string", max_str);

  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_ERRcontrolStr\n");
}

/*--------------------------------------------------------------------------------*/
void
TEST_ERRlogfile()
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine ERRlogfile(), as defined
   *      in <toolsa/err.h>
   */

  char *logname = "testlog";

  /*
   * Test that cannot call ERRprintf() before ERRlogfile()
   */

  ERRprintf(ERR_INFO, "This is a call to ERRprintf() before ERRlogfile, should fail\n");

  /*
   * Test behavior is as expected.
   * Should write to a default "errlog" file first.
   */

  ERRcontrol("ON LOG");
  ERRprintf(ERR_INFO, "This string should go into the default file: errlog\n");
  ERRcontrol("OFF LOG");
  ERRprintf(ERR_INFO, "This string should NOT go into the default file: errlog\n");
  
  /*
   * Should now write to a specified error log file 
   */

  ERRlogfile(logname);

  ERRcontrol("ON LOG");
  ERRprintf(ERR_INFO, "This string should go into the file: %s\n", logname);
  fprintf(stderr, "Trying to print to stderr while logging is ON\n");
  ERRcontrol("OFF LOG");
  ERRprintf(ERR_INFO, "This string should NOT go into the file: %s\n", logname);

  /*
   * Test bogus text strings into ERRlogfile()
   */

  ERRlogfile("This is a bogus string");
  ERRlogfile("");

#ifdef FULLTEST
  /*
   * NOTE: This next causes a segmentation violation
   */

  ERRlogfile(NULL);
#endif

  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_ERRlogfile\n");
}  


/*--------------------------------------------------------------------------------*/
void
TEST_ERRprintf()
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine ERRprintf(), as defined
   *      in <toolsa/err.h>
   */

  int error_level;

  /*
   * Test bogus error levels
   */

  error_level = 0;
  ERRprintf(error_level, "trying error level: %d\n", error_level);
  error_level = -1;
  ERRprintf(error_level, "trying error level: %d\n", error_level);
  error_level = 999;
  ERRprintf(error_level, "trying error level: %d\n", error_level); 





  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_ERRprintf\n");
}


/*--------------------------------------------------------------------------------*/
/*
 * main program driver
 */
main(int argc, char *argv[])
{
  char *appname = "TEST_toolsa_err";

  /* 
   * Initialize the ERR module. Unfortunately, the module doesn't do
   * something reasonable if not initialized properly.
   */

  ERRinit( appname, argc, argv);

 /*
  * Test the individual module subroutines
  */

  TEST_ERRcontrol();
  TEST_ERRcontrolStr();
  TEST_ERRlogfile();
  TEST_ERRprintf();

  return(0);
}

/*=============================== END OF FILE =====================================*/
