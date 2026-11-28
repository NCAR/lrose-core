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

Program to write dz=0 and grid_minz=0 if nz == 1 so that
CIDD will recognise the fields as being surface data.


Use it like this :

MdvHowBad InFile.mdv OutFile.mdv


Niles Oien September 1999

 */


#include <mdv/mdv_read.h>
#include <stdio.h>
#include <math.h>

main(int argc, char *argv[])
{

  MDV_vlevel_header_t *vhdr;
  MDV_handle_t Handle; /* All input variables hang off this thing. */
  MDV_field_header_t *Fhdr; /* For looking at field header. */ 
  int i;
  char InFileName[MAX_PATH_LEN], OutFileName[MAX_PATH_LEN];

  /* See if we got filenames on the input line.
     if not, ask for one.
     */

  if (argc > 1){
    sprintf(InFileName,"%s", argv[1]);
  } else {
    /* No filename specified, ask for one. */
    fprintf(stdout,"Input file name --->");
    fscanf(stdin,"%s",InFileName);
  }

  if (argc > 2){
    sprintf(OutFileName,"%s", argv[2]);
  } else {
    /* No filename specified, ask for one. */
    fprintf(stdout,"Output file name --->");
    fscanf(stdin,"%s",OutFileName);
  }

  MDV_init_handle(&Handle);

  /* Read the file. Exit on error. */

  if(MDV_read_all(&Handle,InFileName,MDV_INT8)){
    fprintf(stderr,"Failed to read %s.\n",InFileName);
    exit(-1);
  }


  /* Loop through the field headers. */

  for (i=0; i < Handle.master_hdr.n_fields; i++){
    Fhdr = Handle.fld_hdrs + i; /* Look at field number i. */


    fprintf(stdout,"Field %d : %s (Units %s )\n",
	    i+1,Fhdr->field_name,Fhdr->units);

    /* If there is only one plane, set to surface values. */
    if (Fhdr->nz != 1){
      fprintf(stdout,"\t%d planes, skipping.\n", Fhdr->nz);
    } else {

      if (Handle.master_hdr.vlevel_included){
	vhdr = Handle.vlv_hdrs + i;
	vhdr->vlevel_params[0] = 0.0;
      }
      Fhdr->grid_dz=0.0; Fhdr->grid_minz=0.0;
      fprintf(stdout,"\tField values set to surface.\n");
    } 
    
  } /* End of loop through fields. */
  
  /* Write the file out. */

  i = MDV_write_all(&Handle, OutFileName, MDV_PLANE_RLE8);

  if (i!= MDV_SUCCESS){
    fprintf(stderr,"Failed to write data to directory %s\n",
	    OutFileName);    
    exit(-1);
  }


  /* Let go of the MDV handle. */
  MDV_free_handle(&Handle);
  
  /* All done. */

}








