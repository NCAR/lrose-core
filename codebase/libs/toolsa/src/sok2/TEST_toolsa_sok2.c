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
 * Name: TEST_toolsa_sok2.c
 *
 * Purpose:
 *
 *      To test the SOK2 module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/sok2.h>.
 *
 * Usage:
 *
 *       % TEST_toolsa_sok2
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Young Rhee       17-JUN-1994
 *
 */

/*
 * include files
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <toolsa/sok2.h>
#include <toolsa/servmap.h>
#define  PROC_RUN_ON_MACH "basin"

/*
 * definitions
 */

int SOK_index1, SOK_index2; 

/*
 * test individual module subroutines
 */
/* ===================================================================== */

int TEST_servermap(void)
{
   int return_code;

   return_code = system("rsh basin ps -auwx | grep servmap");
   fprintf(stderr, "This is return from servmap: %d\n", return_code);
   return( return_code );
}

/*================================================================================*/

void
TEST_SOK2init(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2init(), as defined
   *      in <toolsa/sok2.h>
   */

  int error_code;
 
  fprintf(stderr, "TEST_SOK2init():\n");

  error_code = SOK2init("young");

  /*
   * Done
   */

  if(error_code == 1)
     fprintf(stderr, "Passed test TEST_SOK2init.\n");
  else
     fprintf(stderr, "Not Passed test TEST_SOK2init.\n");

  fprintf(stderr, "Done.\n\n");
}
 
/*--------------------------------------------------------------------------------*/

void
TEST_SOK2exit(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2exit(), as defined
   *      in <toolsa/sok2.h>
   */

  int error_code;

  fprintf(stderr, "TEST_SOK2exit():\n");

  SOK2exit(0);

  error_code = 1;

  /*
   * Done
   */

  if(error_code == 1)
     fprintf(stderr, "Passed test TEST_SOK2exit.\n");
  else
     fprintf(stderr, "Not Passed test TEST_SOK2exit.\n");

  fprintf(stderr, "Done.\n\n");
}
 
/*--------------------------------------------------------------------------------*/

void
TEST_SOK2setServiceFile(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2setServiceFile(), as defined
   *      in <toolsa/sok2.h>
   */

  int error_code;

  fprintf(stderr, "TEST_SOK2setServiceFile():\n");

  /*
   * test with incorrect input
   */

  fprintf(stderr, "Following tests get incorrect inputs.\n");
  error_code = SOK2setServiceFile(SOK2_ENV, "");
  if(error_code)
     fprintf(stderr, "Passed test TEST_SOK2setServiceFile (SOK2_ENV, "").\n");
  else 
     fprintf(stderr, "Not Passed test TEST_SOK2setServiceFile (SOK2_ENV, "").\n");

  error_code = SOK2setServiceFile(SOK2_FILE, "");
  if(error_code)
     fprintf(stderr, "Passed test TEST_SOK2setServiceFile (SOK2_FILE, "").\n");
  else 
     fprintf(stderr, "Not Passed test TEST_SOK2setServiceFile (SOK2_ENV, "").\n");

  error_code = SOK2setServiceFile(SOK2_SERVMAP, PROC_RUN_ON_MACH);
  if(error_code)
     fprintf(stderr, "Passed test TEST_SOK2setServiceFile (SOK2_SERVMAP, "").\n");
  else 
     fprintf(stderr, "Not Passed test TEST_SOK2setServiceFile (SOK2_SERVMAP, "").\n");
  


  error_code = SOK2setServiceFile(SOK2_ENV, "HOST");
  if(error_code)
     fprintf(stderr, "Passed test TEST_SOK2setServiceFile (SOK2_ENV, HOST).\n");
  else 
     fprintf(stderr, "Not Passed test TEST_SOK2setServiceFile (SOK2_ENV, HOST).\n");

  error_code = SOK2setServiceFile(SOK2_FILE, "alg_serv.pb");
  if(error_code)
     fprintf(stderr, "Passed test TEST_SOK2setServiceFile (SOK2_FILE, alg_serv.pb).\n");
  else 
     fprintf(stderr, "Not Passed test TEST_SOK2setServiceFile (SOK2_FILE, alg_serv.pb).\n");

  error_code = SOK2setServiceFile(SOK2_SERVMAP, PROC_RUN_ON_MACH); 
  if(error_code)
     fprintf(stderr, "Passed test TEST_SOK2setServiceFile (SOK2_SERVMAP, %s).\n", PROC_RUN_ON_MACH);
  else 
     fprintf(stderr, "Not Passed test TEST_SOK2setServiceFile (SOK2_SERVMAP, %s).\n", PROC_RUN_ON_MACH);


  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}

/*--------------------------------------------------------------------------------*/

void
TEST_SOK2findService(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2findService(), as defined
   *      in <toolsa/sok2.h>
   */

  int   error_code;
  char* host_name;
  int   port_num;

  fprintf(stderr, "TEST_SOK2findService():\n");

  if(SOK2setServiceFile(SOK2_FILE, "alg_serv.pb") == 1)
  {
	error_code = SOK2findService("ac_tracks", &host_name, &port_num);
        if(error_code == 1)
           fprintf(stderr, "success on finding Service.\n");

        fprintf(stderr, "Following tests get incorrect inputs.\n");
	error_code = SOK2findService("young", &host_name, &port_num);
        if(error_code == -7)
		fprintf(stderr, "service not in service file.\n");
  }
  else
  	fprintf(stderr, "No RAP_SERVICES file found.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}
 
/*--------------------------------------------------------------------------------*/

void
TEST_SOK2register(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2registerxxx(), as defined
   *      in <toolsa/sok2.h>
   */

  int error_code;

  fprintf(stderr, "TEST_SOK2register():\n");
  SOK2register(15007, SERVMAP_TYPE_PRODSERV, SERVMAP_SUBTYPE_AWPS, "Demo");
  SOK2registerTimer(15008, SERVMAP_TYPE_PRODSERV, SERVMAP_SUBTYPE_AWPS, "Demo");
  SOK2registerStatus(15009, SERVMAP_TYPE_PRODSERV, SERVMAP_SUBTYPE_AWPS, "Demo", 200, 300);

  fprintf(stderr, "Success.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}
 
/*--------------------------------------------------------------------------------*/

void
TEST_SOK2servmapInfo(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2servmapInfo(), as defined
   *      in <toolsa/sok2.h>
   */

  int   error_code;
  char* host_name;
  int   port_num;
  int   sok_idx;

  fprintf(stderr, "TEST_SOK2servmapInfo():\n");
  if(SOK2setServiceFile(SOK2_SERVMAP, PROC_RUN_ON_MACH) == 1) 
  {
	error_code = SOK2servmapInfo(SERVMAP_TYPE_PRODSERV, SERVMAP_SUBTYPE_AWPS	             , SERVMAP_INSTANCE_DEMO, &host_name, &port_num);
	if(error_code == 1)
 	       fprintf(stderr, "Success on SOK2servmapInfo.\n");
        else if(error_code == 0)
	       fprintf(stderr, "No servers fit the type, subtype, and instance.\n");
	else if(error_code == -1)
	       fprintf(stderr, "Could not contact server mapper at either host specified in SOK2setServiceFile() calls.\n");


        if(error_code > 0)
        {
	  fprintf(stderr, "Testing opening client with a working server.\n");
          sok_idx = SOK2openClient(host_name, port_num, -1);
   
          if(sok_idx >= 0)
 	    fprintf(stderr, "Success on connection with a working server.\n");
          else
	    fprintf(stderr, "fails to connect.\n");
        }


        fprintf(stderr, "Following tests get invalid inputs.\n");
	error_code = SOK2servmapInfo(SERVMAP_TYPE_PRODSERV, SERVMAP_SUBTYPE_LAPS	             , SERVMAP_INSTANCE_DEMO, &host_name, &port_num);

	if(error_code == 1)
 	       fprintf(stderr, "Success on SOK2servmapInfo.\n");
        else if(error_code == 0)
	       fprintf(stderr, "No servers fit the type, subtype, and instance.\n");
	else if(error_code == -1)
	       fprintf(stderr, "Could not contact server mapper at either host specified in SOK2setServiceFile() calls.\n");
  }
  else
    fprintf(stderr, "No server mapper machine is found.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}
 
/*--------------------------------------------------------------------------------*/

void
TEST_SOK2openClient(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2openClient(), as defined
   *      in <toolsa/sok2.h>
   */

  int   error_code;

  fprintf(stderr, "TEST_SOK2openClient():\n");
  if(SOK2setServiceFile(SOK2_SERVMAP, PROC_RUN_ON_MACH) == 1)
  {

	SOK_index2 = SOK2openClient(PROC_RUN_ON_MACH, 15010, 0);
	switch (SOK_index2)
        {
	    case -1:
	       fprintf(stderr, "General error.\n");
               break;
	    case -2:
	       fprintf(stderr, "unknown host machine.\n");
               break;
	    case -3:
	       fprintf(stderr, "connect() call failed.\n");
               break;
	    case -4:
	       fprintf(stderr, "host=local and gethostname failed.\n");
               break;
        }

  }
  else
	fprintf(stderr, "No service rap file is found.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}
 
/*--------------------------------------------------------------------------------*/

void
TEST_SOK2openServer(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2openServer(), as defined
   *      in <toolsa/sok2.h>
   */

  int error_code;

  fprintf(stderr, "TEST_SOK2openServer():\n");
  SOK_index2 = SOK2openServer(15012);
  if(SOK_index2 == -1)
	fprintf(stderr, "General Error.\n");
  else
	fprintf(stderr, "Success on opening Server socket.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}
 
/*--------------------------------------------------------------------------------*/

void
TEST_SOK2close(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2close(), as defined
   *      in <toolsa/sok2.h>
   */

  int error_code;

  fprintf(stderr, "TEST_SOK2close():\n");

  fprintf(stderr, "first closing.\n");
  if(SOK2close(SOK_index1))
	fprintf(stderr, "Success on closing sockets.\n");
  else
	fprintf(stderr, "failure on closing sockets: index out of range.\n");

  fprintf(stderr, "second closing.\n");
  if(SOK2close(SOK_index2))
	fprintf(stderr, "Success on closing sockets.\n");
  else
	fprintf(stderr, "failure on closing sockets: index out of range.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}

/*--------------------------------------------------------------------------------*/

void
TEST_SOK2statusConnection(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2statusConnection(), as defined
   *      in <toolsa/sok2.h>
   */

  int       error_code;
  int       still_pending;
  

  fprintf(stderr, "TEST_SOK2statusConnection():\n");
  if(SOK2setServiceFile(SOK2_SERVMAP, PROC_RUN_ON_MACH) == 1)
  {
   
         error_code = SOK2statusConnection(SOK_index2, 15010, &still_pending);
         if(error_code == 1)
           fprintf(stderr, "connected.\n");
         else
           fprintf(stderr, "error.\n");
 
   }
   else
      fprintf(stderr, "No matching product server is found.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}

/* --------------------------------------------------------------------------------- */

void
TEST_SOK2getMessage(void)
{
  /*
   * Inputs: None
   *
   * Returns: None
   *
   * Function:
   *      This routine tests the subroutine SOK2getMessage(), as defined
   *      in <toolsa/sok2.h>
   */

  int       error_code;
  char**    mess;
  int       howlong_mess;
  
  int       port_num;
  char*     host_name;
  int       Sidx, Cidx;
  SOK2head* headptr;
  


  fprintf(stderr, "TEST_SOK2getMessage():\n");
  if(SOK2setServiceFile(SOK2_SERVMAP, PROC_RUN_ON_MACH) == 1)
  {
   
      fprintf(stderr, "The latest opened client's index: %d\n", SOK_index2);
      TEST_SOK2statusConnection();

      while(SOK_index2 != Cidx) {
            error_code = SOK2getMessage(-1, &Sidx, &Cidx, headptr,
                                       mess, &howlong_mess);
      }
 
   }
   else
      fprintf(stderr, "No matching product server is found.\n");

  /*
   * Done
   */

  fprintf(stderr, "Done.\n\n");
}


/*--------------------------------------------------------------------------------*/


/*
 * main program driver
 */

main(int argc, char *argv[])
{
  /*
   * Test the individual module subroutines
   */

  /* No server mapper is running, quit */ 
  if(TEST_servermap() != 0) {
     fprintf(stderr, "No server mapper is running.\n");
     exit(1);
  }

  TEST_SOK2init();
  TEST_SOK2setServiceFile();
  TEST_SOK2findService();
  TEST_SOK2register();   
  TEST_SOK2servmapInfo();

  TEST_SOK2openClient();  

  TEST_SOK2getMessage(); 

  TEST_SOK2close();
  TEST_SOK2exit();
  
  return(0);
}

/*=============================== END OF FILE =====================================*/
