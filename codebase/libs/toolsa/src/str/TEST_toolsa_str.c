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
 * Name: TEST_toolsa_str.c
 *
 * Purpose:
 *
 *      To test the STR module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/str.h>.
 *
 * Usage:
 *
 *       % TEST_toolsa_str
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Young Rhee       11-JUL-1994
 *
 */

/*
 * include files
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <toolsa/str.h>

/*
 * definitions
 */

char s_var[256];
int  ret_value; 

/*
 * test individual module subroutines
 */
/* ===================================================================== */

void TEST_STRbinary(void)
{

  fprintf(stderr, "Testing STRbinary.\n");

  STRbinary(10, 4, s_var);
  fprintf(stderr, "The binary representation of 10 is %s.\n", s_var);
  STRbinary(0, 4, s_var);
  fprintf(stderr, "The binary representation of 0 is %s.\n", s_var);
  STRbinary(100, 9, s_var);
  fprintf(stderr, "The binary representation of 100 is %s.\n", s_var);

  fprintf(stderr, "Passed STRbinary.\n\n");
  
}

/* ===================================================================== */

void TEST_STRblnk(void)
{

   fprintf(stderr, "Testing STRblnk.\n");

   ret_value = STRblnk(" Young is good ");
   fprintf(stderr, "The input length is 15 and output length is %d\n", ret_value);

   fprintf(stderr, "Passed STRblnk.\n\n");

}

/* ===================================================================== */

void TEST_STRbpad(void)
{

   fprintf(stderr, "Testing STRbpad.\n");

   fprintf(stderr, "Passed STRbpad.\n\n");

}

/* ====================================================================== */

void TEST_STRconcat(void)
{

   strcpy(s_var, "young ");
   fprintf(stderr, "Testing STRconcat.\n");

   STRconcat(s_var, "good boy", 256);
   fprintf(stderr, "Concated string is %s\n", s_var);

   fprintf(stderr, "Passed STRconcat.\n\n");

}

/* ====================================================================== */

void TEST_STRdelete(void)
{

   fprintf(stderr, "Testing STRdelete.\n");

   STRdelete(s_var, 6);
   fprintf(stderr, "Input is young good boy and 6 chars deleted output is %s.\n", s_var);

   fprintf(stderr, "Passed STRdelete.\n\n");

}

/* ====================================================================== */

void TEST_STRequal(void)
{

   fprintf(stderr, "Testing STRequal.\n");
   
   ret_value = STRequal("young", "YOUNG ");
   if(ret_value)
     fprintf(stderr, "suceed in comparing young & YOUNG.\n");
   else
     fprintf(stderr, "fail on comparing young & YOUNG.\n");

   fprintf(stderr, "Passed STRequal.\n\n");

}

/* ====================================================================== */

void TEST_STRgood(void)
{

   fprintf(stderr, "Testing STRgood.\n");

   if(STRgood(" "))
     fprintf(stderr, "blank is printable.\n");
   if(STRgood("young"))
     fprintf(stderr, "young is printable.\n");

   fprintf(stderr, "Passed STRgood.\n\n");

}

/* ====================================================================== */

void TEST_STRinsert(void)
{

   strcpy(s_var,"good");  
   fprintf(stderr, "Testing STRinsert.\n");

   STRinsert(s_var, "That is ", 100);
   fprintf(stderr, "input is good and Inserted string with 'That is ': %s.\n", s_var); 
  
   fprintf(stderr, "Passed STRinsert.\n\n");

}

/* ===================================================================== */

void TEST_STRmax_copy(void)
{

   strcpy(s_var, "good");
   fprintf(stderr, "Testing STRmax_coyp.\n");

   STRmax_copy(s_var, "another good", 7, 100);
   fprintf(stderr, "input is good and copied string is %s.\n", s_var);

   fprintf(stderr, "Passed STRmax_copy.\n\n");

}

/* ==================================================================== */

void TEST_STRncopy(void)
{

   fprintf(stderr, "Testing STRncopy.\n");

   STRncopy(s_var, "young kap", 6);
   fprintf(stderr, "Ncopied string is %s.\n", s_var);

   fprintf(stderr, "Passed STRncopy.\n\n");

}

/* ================================================================== */

void TEST_STRparse(void)
{

   char  *outputstr[4];
   int   i;

   fprintf(stderr, "Testing STRparse.\n");

   STRncopy(s_var, "kk ll mm hh", 12);
   fprintf(stderr, "input string is %s.\n", s_var);
   for(i=0; i < 4; i++)
      outputstr[i] = (char *)malloc(sizeof( 5 ));

   STRparse(s_var, outputstr, 11, 4, 3);
   for(i=0; i < 4; i++)
      fprintf(stderr, "each element is: %s.\n", outputstr[i]);

   fprintf(stderr, "Passed STRparse.\n\n");
}

/* ================================================================== */
   
void TEST_STRparse_double(void)
{
   double  double_var[5];
   int     i;

   fprintf(stderr, "Testing STRparse_double.\n");

   STRncopy(s_var, "12 34 4.59", 11);
   fprintf(stderr, "input string is %s.\n", s_var);
   STRparse_double(s_var, double_var, 11, 5);
   for(i=0; i < 3; i++)
      fprintf(stderr, "each # is: %lf.\n", double_var[i]);

   fprintf(stderr, "Passed STRparse_double.\n\n");

}

/* ================================================================== */

void TEST_STRpos(void)
{
   int   returned_pos;
 
   fprintf(stderr, "Testing STRpos.\n");

   STRncopy(s_var, "Many of kk is bad.", 19);
   returned_pos = STRpos(s_var, "kk");
   fprintf(stderr, "POS of 'kk'fisrt appearing in '%s' is %d.\n",s_var,returned_pos);

   fprintf(stderr, "Passed STRpos.\n\n");

} 

/* ================================================================== */
   
void TEST_STRremove(void)
{
 
   fprintf(stderr, "Testing STRremove.\n");

   STRncopy(s_var, "Many of kk is bad.", 19);
   fprintf(stderr, "input is '%s'.\n", s_var);
   STRremove(s_var, 'k');
   fprintf(stderr, "'%s' lost 'k' part.\n",s_var);

   fprintf(stderr, "Passed STRremove.\n\n");

} 

/* ================================================================== */

void TEST_STRswap(void)
{
 
   char tmp[10];

   fprintf(stderr, "Testing STRswap.\n");

   STRncopy(s_var, "Many of kk is bad.", 19);
   STRncopy(tmp, "I did it.", 10);

   fprintf(stderr, "first input is '%s'.\n", s_var);
   fprintf(stderr, "second input is '%s'.\n", tmp);

   STRswap(s_var, tmp, 5);
   fprintf(stderr, "'%s' swapped 5 bytes from '%s'.\n",s_var, tmp);

   fprintf(stderr, "Passed STRswap.\n\n");

} 

/* ================================================================== */

void TEST_STRtokn(void)
{
   char* in_var[3];

   fprintf(stderr, "Testing STRtokn.\n");
  
   in_var[0] = (char* )malloc(sizeof(20));
   STRncopy(in_var[0], "young is ??", 12);
   STRtokn(in_var, s_var, 6, " ");
   fprintf(stderr, "token gotten from '%s' is '%s'.\n", in_var[0], s_var);

   fprintf(stderr, "Passed STRtokn.\n\n");

} 

/* ================================================================== */

void main()
{

  TEST_STRbinary();
  TEST_STRblnk();
  TEST_STRbpad();
  TEST_STRconcat();
  TEST_STRdelete();
  TEST_STRequal();
  TEST_STRgood();
  TEST_STRinsert();
  TEST_STRmax_copy();
  TEST_STRncopy();
  TEST_STRparse();
  TEST_STRparse_double();
  TEST_STRpos();
  TEST_STRremove();
  TEST_STRswap();
  TEST_STRtokn();

}

/*=============================== END OF FILE =====================================*/
