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
 * Name: TEST_toolsa_smu.c
 *
 * Purpose:
 *
 *      To test the SMU module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/smu.h>.
 *
 * Usage:
 *
 *       % TEST_toolsa_smu
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Young Rhee       06-JUN-1994
 *
 */

/*
 * include files
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <toolsa/smu.h>


/* 
 * machines where server mappers are
 */
 
  char* servmap_mach[10] = {"stratus", "elvegust", "basin"};

/*
 * definitions
 */


/*
 * test individual module subroutines
 */

/*================================================================================*/
int
TEST_SMU_ntohl(void)
{
  /*
   * Inputs: None
   *
   * Returns: 0 on succeed
   *          1 on failure to pass the test
   *
   * Function:
   *      This routine tests the subroutine SMU_ntohl_XXX(), as defined
   *      in <toolsa/smu.h>
   */

  /*
   * Test behavior is as expected.
   */

  SERVMAP_info_t*    SERVMAP_info_t_ptr;
  SERVMAP_request_t* SERVMAP_request_t_ptr;
  SERVMAP_reply_t*   SERVMAP_reply_t_ptr;
  int error_code = 0;


   SERVMAP_info_t_ptr = (SERVMAP_info_t* )malloc(sizeof(SERVMAP_info_t));

  /*
   * passing a pointer to SERVMAP_XXX which has not been malloced and 
   * assigned the values
   */

  /*
  SMU_ntohl_Info(SERVMAP_info_t_ptr);
  SMU_ntohl_Request(SERVMAP_request_t_ptr);
  SMU_ntohl_Reply(SERVMAP_reply_t_ptr);
  */

  /* result: core dump */

  /*
   * Done
   */

  if(error_code == 0)
     fprintf(stderr, "Passed test TEST_SMU_ntohl_XXX.\n\n");
  else
     fprintf(stderr, "Not Passed test TEST_SMU_ntohl_XXX.\n\n");
  return(error_code);
}
 
/*--------------------------------------------------------------------------------*/

int
TEST_SMU_register(void)
{
  /*
   * Inputs: None
   *
   * Returns: 0 on succeed
   *          1 on failure to pass the test
   *
   * Function:
   *      This routine tests the subroutine SMU_register(), as defined
   *      in <toolsa/smu.h>
   */

  /*
   * Test behavior is as expected.
   */

  SERVMAP_info_t*    SERVMAP_info_t_ptr;
  int error_code = 0;


  SERVMAP_info_t_ptr = (SERVMAP_info_t* )malloc(sizeof(SERVMAP_info_t));

  /*
   * assigning each field of SERVMAP_info_t
   */

  strcpy(SERVMAP_info_t_ptr->server_type, SERVMAP_TYPE_ALG);
  strcpy(SERVMAP_info_t_ptr->server_subtype, SERVMAP_SUBTYPE_WIA_AREA);
  strcpy(SERVMAP_info_t_ptr->instance, SERVMAP_INSTANCE_DEMO);
  strcpy(SERVMAP_info_t_ptr->host, getenv("HOST"));
  SERVMAP_info_t_ptr->port = 15000;

  /* registering server */

  SMU_register(SERVMAP_info_t_ptr, servmap_mach[2], ""); 

  /* Using local host */
  strcpy(SERVMAP_info_t_ptr->server_type, SERVMAP_TYPE_ALG);
  strcpy(SERVMAP_info_t_ptr->server_subtype, SERVMAP_SUBTYPE_WIA_AREA);
  strcpy(SERVMAP_info_t_ptr->instance, SERVMAP_INSTANCE_DEMO);
  strcpy(SERVMAP_info_t_ptr->host, getenv("HOST"));
  SERVMAP_info_t_ptr->port = 15003;
  SMU_register(SERVMAP_info_t_ptr, "local", "");

  /* skip test */

  SMU_register(SERVMAP_info_t_ptr, "NONE", "");
  
  /*
   * Done
   */

  if(error_code == 0)
     fprintf(stderr, "Passed test TEST_SMU_register.\n\n");
  else
     fprintf(stderr, "Not Passed test TEST_SMU_register.\n\n");
  return(error_code);
}
 
/*--------------------------------------------------------------------------------*/

int
TEST_SMU_register2(void)
{
  /*
   * Inputs: None
   *
   * Returns: 0 on succeed
   *          1 on failure to pass the test
   *
   * Function:
   *      This routine tests the subroutine SMU_register(), as defined
   *      in <toolsa/smu.h>
   */

  /*
   * Test behavior is as expected.
   */

  SERVMAP_info_t*    SERVMAP_info_t_ptr;
  int error_code = 0;


   SERVMAP_info_t_ptr = (SERVMAP_info_t* )malloc(sizeof(SERVMAP_info_t));

  /*
   * assigning each field of SERVMAP_info_t
   */

  strcpy(SERVMAP_info_t_ptr->server_type, SERVMAP_TYPE_ALG);
  strcpy(SERVMAP_info_t_ptr->server_subtype, SERVMAP_SUBTYPE_WIA_AREA);
  strcpy(SERVMAP_info_t_ptr->instance, SERVMAP_INSTANCE_DEMO);
  strcpy(SERVMAP_info_t_ptr->host, getenv("HOST"));
  SERVMAP_info_t_ptr->port = 15001;

  /* registering server */

  SMU_register(SERVMAP_info_t_ptr, servmap_mach[2], "");

  /*
   * Done
   */

  if(error_code == 0)
     fprintf(stderr, "Passed test TEST_SMU_register.\n\n");
  else
     fprintf(stderr, "Not Passed test TEST_SMU_register.\n\n");
  return(error_code);
}
 
/*--------------------------------------------------------------------------------*/

int
TEST_SMU_unregister1(void)
{
  /*
   * Inputs: None
   *
   * Returns: 0 on succeed
   *          1 on failure to pass the test
   *
   * Function:
   *      This routine tests the subroutine SMU_unregister(), as defined
   *      in <toolsa/smu.h>
   */

  /*
   * Test behavior is as expected.
   */

  SERVMAP_info_t*    SERVMAP_info_t_ptr;
  int error_code = 0;


   SERVMAP_info_t_ptr = (SERVMAP_info_t* )malloc(sizeof(SERVMAP_info_t));

  /*
   * assigning each field of SERVMAP_info_t
   */

  strcpy(SERVMAP_info_t_ptr->server_type, SERVMAP_TYPE_ALG);
  strcpy(SERVMAP_info_t_ptr->server_subtype, SERVMAP_SUBTYPE_WIA_AREA);
  strcpy(SERVMAP_info_t_ptr->instance, SERVMAP_INSTANCE_DEMO);
  strcpy(SERVMAP_info_t_ptr->host, getenv("HOST"));
  SERVMAP_info_t_ptr->port = 15000;

  /* unregistering server */

  SMU_unregister(SERVMAP_info_t_ptr, servmap_mach[2], "");

  /* skip test */
  SMU_unregister(SERVMAP_info_t_ptr, "local", "");
  SMU_unregister(SERVMAP_info_t_ptr, "NONE", "");

  /*
   * Done
   */

  if(error_code == 0)
     fprintf(stderr, "Passed test TEST_SMU_unregister.\n\n");
  else
     fprintf(stderr, "Not Passed test TEST_SMU_unregister.\n\n");
  return(error_code);
}
 
/*--------------------------------------------------------------------------------*/

int
TEST_SMU_unregister2(void)
{
  /*
   * Inputs: None
   *
   * Returns: 0 on succeed
   *          1 on failure to pass the test
   *
   * Function:
   *      This routine tests the subroutine SMU_unregister(), as defined
   *      in <toolsa/smu.h>
   */

  /*
   * Test behavior is as expected.
   */

  SERVMAP_info_t*    SERVMAP_info_t_ptr;
  int error_code = 0;


   SERVMAP_info_t_ptr = (SERVMAP_info_t* )malloc(sizeof(SERVMAP_info_t));

  /*
   * assigning each field of SERVMAP_info_t
   */

  strcpy(SERVMAP_info_t_ptr->server_type, SERVMAP_TYPE_ALG);
  strcpy(SERVMAP_info_t_ptr->server_subtype, SERVMAP_SUBTYPE_WIA_AREA);
  strcpy(SERVMAP_info_t_ptr->instance, SERVMAP_INSTANCE_DEMO);
  strcpy(SERVMAP_info_t_ptr->host, getenv("HOST"));
  SERVMAP_info_t_ptr->port = 15001;

  /* unregistering server */

  SMU_unregister(SERVMAP_info_t_ptr, servmap_mach[2], "");

  /*
   * Done
   */

  if(error_code == 0)
     fprintf(stderr, "Passed test TEST_SMU_unregister.\n\n");
  else
     fprintf(stderr, "Not Passed test TEST_SMU_unregister.\n\n");
  return(error_code);
}
 
/*--------------------------------------------------------------------------------*/

int
TEST_SMU_requestInfo(void)
{
  /*
   * Inputs: None
   *
   * Returns: 0 on succeed
   *          1 on failure to pass the test
   *
   * Function:
   *      This routine tests the subroutine SMU_requestInfo(), as defined
   *      in <toolsa/smu.h>
   */

  /*
   * Test behavior is as expected.
   */

  SERVMAP_info_t**     SERVMAP_info_t_ptr;
  SERVMAP_request_t*   SERVMAP_request_t_ptr;

  int howmany_servers;
  int error_code = 0;


  SERVMAP_request_t_ptr= (SERVMAP_request_t* )malloc(sizeof(SERVMAP_request_t));

  /*
   * assigning each field of SERVMAP_request_t
   */

  strcpy(SERVMAP_request_t_ptr->server_type, SERVMAP_TYPE_ALG);
  strcpy(SERVMAP_request_t_ptr->server_subtype, SERVMAP_SUBTYPE_WIA_AREA);
  strcpy(SERVMAP_request_t_ptr->instance, SERVMAP_INSTANCE_DEMO);

  /* requesting the infos of registered server */

  error_code = SMU_requestInfo(SERVMAP_request_t_ptr, &howmany_servers, 
                               SERVMAP_info_t_ptr, servmap_mach[2], "");

  printf("Here is the servers #: %d \n", howmany_servers);

  /* 
   * in case where no server is found matching with requesting info
   */

  if(SERVMAP_info_t_ptr == NULL)
    printf("yes, it is null!!\n");

  /*
   * Done
   */

  if(error_code == 1)
     fprintf(stderr, "Passed test TEST_SMU_requestInfo.\n\n");
  else
     fprintf(stderr, "Not Passed test TEST_SMU_requestInfo.\n\n");
  return(error_code);
}

/*--------------------------------------------------------------------------------*/

/*
 * main program driver
 */

main(int argc, char *argv[])
{

  int   servnotfound = 1;
  int   servnotfound2 = 1;
  int   servnotfound3 = 1;

  /*
   * checking to see if server mapper is running on these machines
   */

  servnotfound = system("rsh stratus ps -auxw | grep servmap");

  if(servnotfound) 
    fprintf(stderr, "No server map is found on %s!!\n", servmap_mach[0]);
    
  servnotfound2 = system("rsh elvegust ps -auxw | grep servmap");

  if(servnotfound2) 
    fprintf(stderr, "No server map is found on %s!!\n", servmap_mach[1]);

  servnotfound3 = system("rsh basin ps -auxw | grep servmap");

  if(servnotfound3) 
    fprintf(stderr, "No server map is found on %s!!\n", servmap_mach[2]);

  if(servnotfound && servnotfound2 && servnotfound3) {
    fprintf(stderr, "No server map is found on any machine\n\n");
    exit(1);
  }

  /*
   * Test the individual module subroutines
   */


  TEST_SMU_ntohl();

  /* 
   * assigning two different servers on two different machine
   */

  TEST_SMU_register();
  TEST_SMU_register2();
  TEST_SMU_requestInfo();

  TEST_SMU_unregister2();
  TEST_SMU_requestInfo(); 
  TEST_SMU_unregister1();
  
  return(0);
}

/*=============================== END OF FILE =====================================*/
