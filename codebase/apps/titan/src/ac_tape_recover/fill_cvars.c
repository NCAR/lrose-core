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
 * Function:       fill_cvars                
 * 
 * Description :                                                        
 *    Allocates memory (in global area) for Calculated Variables table.
 *    Reads the description and the coefficients necessary for computing
 *    the field.
 *
 * Input:  
 *    none required but some global data (Glob->params.callsign) is used
 * Output: 
 *    a filled CalculatedVariable struct
 **********************************************************************/

#include "ac_tape_recover.h"

si32 fill_cvars()

{

   si32 i, ifld=0;
   ui08 filename[BUF_SIZE];
   ui08 buf[BUF_SIZE];
   FILE *fp;

/**********************************************************************/

   if (Glob->params.debug) fprintf(stderr, "--------fill_cvars------------- \n\n");

/* get memory for calculated variable table -- kept for duration of run */
   Glob->cv = (CalculatedVariables *) ucalloc(MAX_CALC_FIELDS,(unsigned) 
                                    sizeof(CalculatedVariables));

/* make filename for calcualted variables table */
   sprintf(filename, Glob->params.cvars_file_path);

   if (Glob->params.debug) fprintf(stderr, "Calculated Variables table filename is %s \n",filename);

/* open the file */
   if ((fp = fopen(filename,"r")) == NULL) { 
      exit_str("Could not open calculated variables file");
   }
      
/* read each line and extract field names, variable type and coefficients */
   while (fgets(buf,BUF_SIZE,fp) != NULL) {

      if (buf[0] != COMMENT_CHAR) {

         sscanf(buf,"%s %s %lf %lf %lf %lf \n",
                    Glob->cv[ifld].name, 
                    Glob->cv[ifld].var_type,
                    &Glob->cv[ifld].c1,
                    &Glob->cv[ifld].c2,
                    &Glob->cv[ifld].c3,
                    &Glob->cv[ifld].c4);

         /* fill in unknown for units, this will be filled in later
          * by individual calc_* functions */
         strncpy(Glob->cv[ifld].units,"UNKNOWN ",8);

         /* fill in var_type with blanks so no null byte in first 8 chars */
         for (i = strlen(Glob->cv[ifld].var_type); i < 8; i++) {
            memcpy(&Glob->cv[ifld].var_type[i]," ",1);
         }

         ifld ++;
        
         /* exit if there isn't enough memory for calc variables table */
         if (ifld > MAX_CALC_FIELDS) 
            exit_str("Not enough memory allocated for Calculated Variables Table");
      }
   }  /* end reading data */

   /* set the total number of calculated fields */
   Glob->num_calc_fields = ifld;

/* print out calculated variables table info */
   if (Glob->params.debug) {
      fprintf(stderr, "name f_type c1 c2 c3\n");
      for (i = 0; i < Glob->num_calc_fields; i++) {
         fprintf(stderr, "%3d %s %s %f %f %f %f \n",
                            i,
                            Glob->cv[i].name,
                            Glob->cv[i].var_type,
                            Glob->cv[i].c1, 
                            Glob->cv[i].c2, 
                            Glob->cv[i].c3, 
                            Glob->cv[i].c4); 
      }

      fprintf(stderr, "---------------------------------- \n\n");
   } /* endif if debug and printing */

   fclose(fp);

   return(OKAY);

}
