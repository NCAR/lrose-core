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
 * Name: TEST_toolsa_dlm.c
 *
 * Purpose:
 *
 *      To test the ERR module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/dlm.h>.
 *
 * Usage:
 *
 *       % TEST_toolsa_dlm
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Young Rhee       06-JUN-1994
 *
 * comment: Test on DLMlessthan_fn, and the function DLMInsert that uses
 *          DLMlessthan_fn is excluded because it is unused. 
 *          If data is type of float number, round error might occur.
 */

/*
 * include files
 */

#include <stdio.h>
#include <toolsa/err.h>
#include <toolsa/dlm.h>


/*
 * definitions
 */


/*
 * test individual module subroutines
 */

/*================================================================================*/
DLMlist*
TEST_DLMcreateList(void)
{
  /*
   * Inputs: None
   *
   * Returns: pointer to DLMlist
   *
   * Function:
   *      This routine tests the subroutine DLMcreateList(), as defined
   *      in <toolsa/dlm.h>
   */

  /*
   * Test behavior is as expected.
   */

  DLMlist* tmp;
  DLMlist* tmp2;


  /*
   * in case size_t = 0
   * it works as it is expected
   */

  /* tmp = DLMcreateList(0); */ 

  /*
   * in case size_t > 0 
   * it works as it is expected
   */

  tmp = DLMcreateList(4); 

  if(tmp == NULL)
    fprintf(stderr, "something is wrong in creating DLMlist.\n");
  else
    fprintf(stderr, "succeed in creating DLMlist.\n");

  /*
   * Test bogus text strings into DLMcreateList()
   * in case data_size < 0: it fails to return NULL
   */

  tmp2 = DLMcreateList(-20);

  if(tmp2 != NULL)
    fprintf(stderr, "something is wrong in returning on failure of DLMcreate.\n");
  else
    fprintf(stderr, "succeed in returning on failure.\n");
  
  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_ERRcontrol.\n\n");
  return(tmp);
}


/*--------------------------------------------------------------------------------*/
int
TEST_DLMaddAfter_append(DLMlist* start)
{
  /*
   * Inputs: head pointer to DLMlist
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the subroutine DLMaddAfrter(), as defined
   *      in <toolsa/dlm.h>
   */

  /*
   * Test behavior is as expected...??
   */
  
  DLMlist* tmp_dlm_ptr;
  DLMlist* tmp_dlm_ptr2;
  float    return_data;
  float    tmp_float_data = 1.987;
  float    tmp_float_data2 = 1.789;
  char*    tmp_char_data = "Young Rhee";
  int      error_occur = 0;

  /*
   * inserting a data of float number & string
   */

  tmp_dlm_ptr = DLMaddAfter(start, tmp_char_data);
  tmp_dlm_ptr = DLMaddAfter(start, &tmp_float_data);	

  return_data =*( (float* )DLMgetCurrent(start) );


  if(tmp_float_data == return_data)
     fprintf(stderr, "succeed in testing DLMaddAfter.\n");
  else {
     error_occur = 1;
     fprintf(stderr, "ERROR: failing in testing DLMaddAfter.\n");
     return(error_occur);
  }

  /* 
   * appending a node after the specified list
   */

  tmp_dlm_ptr2 = DLMappend(start, &tmp_float_data2);

  if(*( (float* )DLMgetCurrent(start) ) == tmp_float_data2)
      fprintf(stderr, "succeed in testing DLMappend.\n");
  else {
      fprintf(stderr, "ERROR: fail in testing DLMappend.\n"); 
      error_occur = 1;
      return(error_occur);
  }

  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_DLMaddAfter_append.\n\n");
  return(error_occur);
}


/*--------------------------------------------------------------------------------*/
int
TEST_DLMdestroyList(DLMlist* origin)
{
  /*
   * Inputs: head pointer to the List
   *
   * Returns: 1: on failure
              0: on succeed
   *
   * Function:
   *      This routine tests the subroutine DLMdestroyList(), as defined
   *      in <toolsa/dlm.h>
   */


  /*
   * Test according to specification on DLMdestroyList in dlm.h  
   */

  int error_occur = 0;

  /*
   * Delete the whole list
   */

  DLMdestroyList(origin);
  fprintf(stderr, "succeed in deleting the whole list.\n");

  /*
   * try to delete the empty list
   */
  
  /* DLMdestroyList(origin); 
   * fprintf(stderr, "succeed in deleting empty list.\n");
   */
 
  /*
   * comment: It is ambiguous what the head pointer going to point to, or
   * head pointer is going to be free.
   * case of destroying empty list: core dump.
   */
  
  error_occur = 1;
  return(error_occur);

  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_DLMdestroyList.\n\n");
  return(error_occur);
}


/*--------------------------------------------------------------------------------*/
int
TEST_DLMgetCurrent(DLMlist* origin)
{
  /*
   * Inputs: head pointer to the List
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *        This routine tests the subroutine DLMgetCurrent(), as defined
   *        in <toolsa/dlm.h>
   */

  /*
   * Test according to specification on DLMgetCurrent in dlm.h
   */

  /*
   * get current data
   */

  float tmp_float_var;
  char  tmp_char_var;
  int   error_occur = 0;

  tmp_float_var = *((float* )DLMgetCurrent(origin));
  fprintf(stderr, "succeed in getting current data.\n");

  /*
   * get current data out of empty list
   */

  DLMdestroyList(origin);
  origin = DLMcreateList(4);
  DLMgetCurrent(origin);
  fprintf(stderr, "succeed in getting current data from empty list.\n");

  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_DLMgetCurrent.\n\n");
  DLMdestroyList(origin);
  return(error_occur);
}
 
/*---------------------------------------------------------------------------------*/

int
TEST_DLMtraverse(DLMlist* origin)
{
  /*
   * Inputs: head pointer to the List
   *
   * Returns: 1: on failure
   *          0: on succeed
   *
   * Function:
   *        This routine tests the subroutine DLMfirst(), DLMnext(),
   *        DLMlast(), and DLMprev() as defined
   *        in <toolsa/dlm.h>
   */
 
  /*
   * Test according to specification on DLMfirst DLMnext DLMprev DLMlast in dlm.h
   */
 
  float tmp_float_var;
  char* tmp_char_var;
  int   error_occur = 0;

  origin = DLMcreateList(4);
 
  /*
   * test on empty list
   */

  if(!DLMfirst(origin) && !DLMnext(origin) && 
                       !DLMprev(origin) && !DLMlast(origin))  
     fprintf(stderr, "succeed in traversing on empty list.\n");
  else  {
     fprintf(stderr, "failure on traversing on empty list.\n");
     error_occur = 1;
     return(error_occur);
  }

  TEST_DLMaddAfter_append(origin);

  /*
   * move to first data
   */
  
  tmp_char_var = DLMfirst(origin);
  fprintf(stderr, "first data is %s.\n", tmp_char_var);
  fprintf(stderr, "succeed in moving to first data.\n");
 
  /* 
   * move to next data
   */ 

  tmp_float_var = *((float *)DLMnext(origin));
  fprintf(stderr, "succeed in moving to next data.\n");

  /*
   * move to last data
   */

  tmp_float_var = *((float *)DLMlast(origin));
  fprintf(stderr, "succeed in moving to last data.\n");

  /* 
   * move to previous data
   */ 
  
  tmp_float_var = *((float *)DLMprev(origin));
  fprintf(stderr, "succeed in moving to previous data.\n");
  
  
  /*
   * Done
   */   
 
  fprintf(stderr, "Passed test TEST_DLMtraverse.\n\n"); 
  DLMdestroyList(origin);
  return(error_occur);
}

/*---------------------------------------------------------------------------------*/

int
TEST_DLMpushpop(DLMlist* origin)
{
  /*
   * Inputs: head pointer to the List
   *
   * Returns: 1: on failure
   *          0: on succeed
   *
   * Function:
   *        This routine tests the subroutine DLMpop() and DLMpush()
   *        as defined in <toolsa/dlm.h>
   */
 
  /*
   * Test according to specification on DLMpop DLMpush in dlm.h   
   */

 
  float tmp_float_var1 = 1.23;
  float tmp_float_var2 = 4.56;
  float tmp_float_var3 = 7.89;
  float a , b;
  int   error_occur = 0;
 
  origin = DLMcreateList(4);

  /*
   * test on empty list
   */
 
  if(!DLMpop(origin))
     fprintf(stderr, "succeed in poping on empty List.\n");
  else {
     fprintf(stderr, "ERROR: failure on poping on empty List.\n");
     error_occur = 1;
  }
 
 
  /*
   * push the datas into the List
   */
 
  DLMpush(origin, &tmp_float_var1);
  DLMpush(origin, &tmp_float_var2);
  DLMpush(origin, &tmp_float_var3);
  a = *((float* )DLMfirst(origin));
  b = *((float* )DLMlast(origin));
  
  if( a == tmp_float_var3 && b == tmp_float_var1)
    fprintf(stderr, "succeed in pushing datas into the List.\n");
  else {
    fprintf(stderr, "ERROR: failure on pushing datas into the List.\n");
    error_occur = 1;
  } 

  /*
   * Done
   */
 
  fprintf(stderr, "Passed test TEST_DLMpushpop.\n\n");
  DLMdestroyList(origin);
  return(error_occur);

}

/*---------------------------------------------------------------------------------*/

int
TEST_DLMpromote_remove(DLMlist* origin)
{
  /*
   * Inputs: head pointer to the List
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *        This routine tests the subroutine DLMpromote() and DLMremove()
   *        as defined in <toolsa/dlm.h>
   */
 
  /*
   * Test according to specification on DLMpromote DLMremove in dlm.h   
   */

 
  float tmp_float_var1 = 1.23;
  float tmp_float_var2 = 4.56;
  float tmp_float_var3 = 7.89;
  float a, b;
  int   error_occur = 0;
 
  origin = DLMcreateList(4);

  /*
   * test on empty list
   */
 
  DLMpromote(origin, &tmp_float_var1);
  DLMremove(origin, &tmp_float_var2);
  fprintf(stderr, "succeed in promoting and removing on empty List.\n");
 
 
  /*
   * promote the specified datas into top of the List
   */
 
  DLMpush(origin, &tmp_float_var1);
  DLMpush(origin, &tmp_float_var2);
  DLMpush(origin, &tmp_float_var3);

  DLMpromote(origin, &tmp_float_var1);
  a = *((float* )DLMfirst(origin));
  fprintf(stderr, "first promoted is %f.\n", a);
  if(a != tmp_float_var1) {
    fprintf(stderr, "ERROR: Failure on promoting.\n");
    error_occur = 1;
  }

  DLMdestroyList(origin);

  origin = DLMcreateList(4);

  DLMpush(origin, &tmp_float_var1);
  DLMpush(origin, &tmp_float_var2);
  DLMpush(origin, &tmp_float_var3);

  DLMremove(origin, &tmp_float_var1);
 
  b = *((float* )DLMlast(origin));
  fprintf(stderr, "last left after removing is %f.\n", b);
  if(b != tmp_float_var2) {
    fprintf(stderr, "ERR: failure on removintg.\n");
    error_occur = 1;
  }
  else
    fprintf(stderr, "succeed in promoting and removing datas into the List.\n");

  /*
   * Done
   */

  /*
   * comment: promote and remove does not promote the specified data node 
   *          and remove it from the list
   */ 
 
  fprintf(stderr, "Passed test TEST_DLMpromote_remove.\n\n");
  DLMdestroyList(origin);
  return(error_occur);

}

/*---------------------------------------------------------------------------------*/


int
TEST_DLMsetCurrent(DLMlist* origin)
{
  /*
   * Inputs: head pointer to the List
   *
   * Returns: 1: on failure
   *          0: on succeed
   *
   * Function:
   *        This routine tests the subroutine DLMsetCurrent() 
   *        as defined in <toolsa/dlm.h>
   */
 
  /*
   * Test according to specification on DLMsetCurrent in dlm.h   
   */

 
  float tmp_float_var1 = 1.23;
  float tmp_float_var2 = 4.56;
  float tmp_float_var3 = 7.89;
  float a , b;
  int   error_occur = 0;
 
  origin = DLMcreateList(4);

  /*
   * test on empty list
   */
 
  DLMsetCurrent(origin, &tmp_float_var1);
  fprintf(stderr, "succeed in setting current on empty List.\n");
 
 
  /*
   * Set current data node according to the specified data
   */
 
  DLMpush(origin, &tmp_float_var1);
  DLMpush(origin, &tmp_float_var2);
  DLMpush(origin, &tmp_float_var3);

  DLMsetCurrent(origin, &tmp_float_var1);
  b = *((float* )DLMgetCurrent(origin));
  fprintf(stderr, "Current is %f.\n", b);
  if( b != tmp_float_var1) {
    fprintf(stderr, "ERROR: failure on setting current.\n");
    error_occur = 1;
  }
  else
    fprintf(stderr, "succeed in setting current datas into the List.\n");

  /*
   * Done
   */

  /*
   * comment: DLMsetCurrent does not set the current node according to the
   * specified data
   */

  fprintf(stderr, "Passed test TEST_DLMpromote_remove.\n\n");
  DLMdestroyList(origin);
  return(error_occur);
}

/*---------------------------------------------------------------------------------*/

int
TEST_DLMclone(DLMlist* origin)
{
  /*
   * Inputs: head pointer to the List
   *
   * Returns: 1: on failure
   *          0: on succeed
   *
   * Function:
   *        This routine tests the subroutine DLMclone() 
   *        as defined in <toolsa/dlm.h>
   */
 
  /*
   * Test according to specification on DLMclone in dlm.h   
   */

 
  float tmp_float_var1 = 1.23;
  float tmp_float_var2 = 4.56;
  float tmp_float_var3 = 7.89;
  float a , b;
  int   error_occur = 0;
  DLMlist* tmp;
 
  origin = DLMcreateList(4);

 
  /*
   * Set current data node according to the specified data
   */
 
  DLMpush(origin, &tmp_float_var1);
  DLMpush(origin, &tmp_float_var2);
  DLMpush(origin, &tmp_float_var3);

  tmp = DLMclone(origin);
  b = *((float* )DLMlast(tmp));
  if( b != tmp_float_var1) {
    fprintf(stderr, "ERROR: failure on clone.\n");
    error_occur = 1;
  }
  else
    fprintf(stderr, "succeed in DLMclone.\n");

  /*
   * Done
   */

  fprintf(stderr, "Passed test TEST_DLMclone.\n\n");
  DLMdestroyList(origin);
  return(error_occur);

}

/*--------------------------------------------------------------------------------*/


/*
 * main program driver
 */
main(int argc, char *argv[])
{

  /* 
   * Initialize the DLM module. Unfortunately, the module doesn't do
   * something reasonable if not initialized properly.
   */

  DLMlist* head;
  
  head = TEST_DLMcreateList();

  /*
   * Test the individual module subroutines
   */

  TEST_DLMaddAfter_append(head);
  TEST_DLMdestroyList(head);

  head = TEST_DLMcreateList();
  TEST_DLMaddAfter_append(head); 
  TEST_DLMgetCurrent(head);
  TEST_DLMtraverse(head);
  TEST_DLMpushpop(head);
  TEST_DLMpromote_remove(head);
  TEST_DLMsetCurrent(head);
  TEST_DLMclone(head);

  return(0);
}

/*=============================== END OF FILE =====================================*/
