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
 * Function:       fill_acqtbl                
 *
 * Description:                                                        
 *    Allocates memory (in global area) for aquisition table.
 *    Reads an acquisition table for specific project and fills in
 *    acqtbl struct which contains names and tag numbers.
 *
 * Input:  
 *    non required but some global variables (Glob->proj_name) are used
 * Output: 
 *    a filled acqtbl struct
 **********************************************************************/

#include "wmi_ac_recover.h"

si32 fill_acqtbl()

{
   si32 i,j,k;
   si32 ifld=0;
   ui08 filename[BUF_SIZE];
   ui08 buf[BUF_SIZE];
   ui08 temp_para2[8];
   FILE *fp;

/**********************************************************************/

   if (Glob->params.debug) fprintf(stderr, "--------fill_acqtbl------------- \n\n");

/* get memory for aquisition table -- kept for duration of run */
   Glob->acqtbl = (Acqtbl *)ucalloc(MAX_RAW_FIELDS,(unsigned) sizeof(Acqtbl));

/* determine name of acquitision table */
   sprintf(filename, Glob->params.acqtbl_file_path);

   if (Glob->params.debug) fprintf(stderr, "Acquistion Table Filename is %s \n",filename);

/* open the file */
   if ((fp = fopen(filename,"r")) == NULL) 
      exit_str("Could not open acquistion table file");
      
/* read first two lines which tell the image and state data buffer sizes */
   for (i=0; i < MAX_INFO_LINES; i++) {
      if (fgets(&buf[0],BUF_SIZE,fp) != NULL) {
         k=0;
         while (buf[k] != ' ') k++;  /* find out how long string is */
         if (strncmp(&buf[0],IMAGE_BUF_SIZE,14) == 0) {
               Glob->image_buf_size = atoi(&buf[k+1]);
         }
         if (strncmp(&buf[0],STATE_BUF_SIZE,14) == 0) {
            Glob->state_buf_size = atoi(&buf[k+1]);
         }
      }
   } /* end reading first two lines */

/* make sure sizes of the image and state buffers are entered */
/* ADD THIS INTO TDRP EVENTUALLY! */
   if (Glob->image_buf_size <= 0 || Glob->state_buf_size <= 0) {
      exit_str("Do not have size for image_buf_size/state_buf_size in aquisition table");
   }

/* read each line and extract field names and data tag numbers */
   while (fgets(&buf[0],BUF_SIZE,fp) != NULL) {

      if (buf[0] != COMMENT_CHAR) {

         sscanf(buf,"%s %d %4d %d %d %d %d %s %d %s %s\n",
                 Glob->acqtbl[ifld].name, 
                 &Glob->acqtbl[ifld].buffer, 
                 &Glob->acqtbl[ifld].tag, 
                 &Glob->acqtbl[ifld].samples, 
                 &Glob->acqtbl[ifld].size, 
                 &Glob->acqtbl[ifld].type,
                 &Glob->acqtbl[ifld].para1,
                 temp_para2,
                 &Glob->acqtbl[ifld].para3,
                 Glob->acqtbl[ifld].address,
                 Glob->acqtbl[ifld].var_type);

         /* determine if para2 is integer or hex # */
         if (temp_para2[1] != 'x')
            Glob->acqtbl[ifld].para2 = atoi(temp_para2); 
         else
            Glob->acqtbl[ifld].para2 = ERROR;

         /* fill in location in array info */
         Glob->acqtbl[ifld].array_loc = ifld;

         /* fill in units field -- this is output units for variable 
          * if a field is modified (ie converted from volts to ieee, then
          * the units sting will be changed by the calc_* function */
         strncpy(Glob->acqtbl[ifld].units,"UNKNOWN  ",8);

         /* fill in var_type with blanks so no null byte in first 8 chars */
         for (i = strlen(Glob->acqtbl[ifld].var_type); i < 8; i++) {
            memcpy(&Glob->acqtbl[ifld].var_type[i]," ",1);
         }

         ifld ++;
        
         /* exit if not enough memory to fill in acquisition table */
         if (ifld > MAX_RAW_FIELDS) 
            exit_str("Not enough fields allocated for Acquisition Table");
      }
   }

/* set the number of fields in this dataset */
   Glob->num_raw_fields = ifld;

/* print out aquisition table info */
   if (Glob->params.debug) {
      fprintf(stderr,
              "name tag buffer arrayloc samples size type p1 p2 p3 add f_type \n");
      for (j = 0; j < Glob->num_raw_fields; j++) {
         fprintf(stderr, "%3d %s %5d %3d %3d %3d %3d %3d %3d %3d %3d %s %s\n",
                            j,Glob->acqtbl[j].name,
                            Glob->acqtbl[j].tag, 
                            Glob->acqtbl[j].buffer, 
                            Glob->acqtbl[j].array_loc, 
                            Glob->acqtbl[j].samples, 
                            Glob->acqtbl[j].size, 
                            Glob->acqtbl[j].type, 
                            Glob->acqtbl[j].para1, 
                            Glob->acqtbl[j].para2, 
                            Glob->acqtbl[j].para3, 
                            Glob->acqtbl[j].address, 
                            Glob->acqtbl[j].var_type);
      }

      if (Glob->params.debug) fprintf(stderr, "---------------------------------- \n\n");
   } /* endif debug --> print out info */

   fclose(fp);

   return(OKAY);
}
