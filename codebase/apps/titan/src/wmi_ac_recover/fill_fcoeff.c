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

/**********************************************************************
 * Function:       fill_fcoeff                
 * 
 * Description :                                                        
 *    Allocates memory (in global area) for Function Coefficients table.
 *    Reads  function coefficients table for specific project and fills in
 *    FucntionCoefficients struct which contains field names, tag numbers 
 *    and coefficients.
 *    Note: This routine should be called AFTER the fill_acqtbl routine.
 * Input
 *    None required by global variable Glob->proj_name is used
 * Output
 *    A filled function coefficient table
 **********************************************************************/

#include "wmi_ac_recover.h"

si32 fill_fcoeff()

{
   si32 i;
   si32 ifld=0;
   ui08 filename[BUF_SIZE];
   ui08 buf[BUF_SIZE];
   FILE *fp;

/**********************************************************************/

   if (Glob->params.debug) fprintf(stderr, "--------fill_fcoeff------------- \n\n");

/* get memory for function coefficient table -- kept for duration of run */
   Glob->fc = (FunctionCoefficients *) ucalloc(MAX_RAW_FIELDS,(unsigned) 
                                    sizeof(FunctionCoefficients));

/* make filename for function coefficients table */
   sprintf(filename, Glob->params.fcoeff_file_path);

   if (Glob->params.debug) fprintf(stderr, "Fucntion coefficients table filename is %s\n",filename);

   if ((fp = fopen(filename,"r")) == NULL) { 
      exit_str("Could not open function coefficients file");
   }
      
/* read each line and extract field names, tag numbers and coefficients */
   while (fgets(buf,BUF_SIZE,fp) != NULL) {

      if (buf[0] != COMMENT_CHAR) {

         sscanf(buf,"%s %d %s %lf %lf %lf %lf \n",
                 Glob->fc[ifld].name, 
                 &Glob->fc[ifld].tag, 
                 Glob->fc[ifld].var_type,
                 &Glob->fc[ifld].c1,
                 &Glob->fc[ifld].c2,
                 &Glob->fc[ifld].c3,
                 &Glob->fc[ifld].c4);

         /* fill in var_type with blanks so no null byte in first 8 chars */
         for (i = strlen(Glob->fc[ifld].var_type); i < 8; i++) {
            memcpy(&Glob->fc[ifld].var_type[i]," ",1);
         }

         ifld ++;
        
         /* exit if not enough memory for function coeffient table */
         if (ifld > MAX_RAW_FIELDS) 
            exit_str("Not enough memory allocated for Function Coefficients Table");
      }
   } /* end while reading file */

/* make sure that there are the same number of function coefficients 
 * as there are fields in the acquisition table */
   if (ifld != Glob->num_raw_fields)
      exit_str("Didn't read the same number of fields in the \
                Function Coefficients Table");

/* print out function coefficient table info */
   if (Glob->params.debug) {
      fprintf(stderr, "name tag f_type c1 c2 c3\n");
      for (i = 0; i < Glob->num_raw_fields; i++) {
         fprintf(stderr, "%3d %s %5d %s %f %f %f %f \n",
                            i,
                            Glob->fc[i].name,
                            Glob->fc[i].tag, 
                            Glob->fc[i].var_type,
                            Glob->fc[i].c1, 
                            Glob->fc[i].c2, 
                            Glob->fc[i].c3, 
                            Glob->fc[i].c4); 
      }

      fprintf(stderr, "---------------------------------- \n\n");
   } /* endif debug and printing */

   fclose(fp);

   return(OKAY);

}
