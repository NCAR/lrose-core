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

Program to print out what percentage of an MDV file has bad or missing
data, and give the data min and max if there are any good data.

Use it like this :

MdvHowBad blah.mdv

 */


#include <mdv/mdv_read.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

  MDV_handle_t Handle; /* All input variables hang off this thing. */
  MDV_field_header_t *Fhdr; /* For looking at field header. */ 
  int i,j,k,pbad,first;
  long numbad,numgood,total;
  char FileName[1024];
  unsigned char *databytes;
  float val,min,max;
  double data_total,mean,var,sumsqr;

  /* See if we got a filename on the input line.
     if not, ask for one.
     */

  if (argc > 1){
    /* The argument count is more than one, take the second
       argument as the file name. */
    sprintf(FileName,"%s", argv[1]);
  } else {
    /* No filename specified, ask for one. */
    fprintf(stdout,"Input file name --->");
    fscanf(stdin,"%s",FileName);
  }

  /* Need to init the Handle - this just sets it up
     as I understand it. */

  MDV_init_handle(&Handle);

  /* Read the file. Exit on error. */

  if(MDV_read_all(&Handle,FileName,MDV_INT8)){
    fprintf(stderr,"Failed to read %s.\n",FileName);
    exit(-1);
  }


  /* Loop through the field headers. */

  for (i=0; i < Handle.master_hdr.n_fields; i++){
    Fhdr = Handle.fld_hdrs + i; /* Look at field number i. */


    fprintf(stdout,"Field %d : %s (Units %s )\n",
	    i+1,Fhdr->field_name,Fhdr->units);

    /* Look at the actual data. MDV data is stored as
       a series of two dimensional planes (very often there
       is only one plane). Loop through the 
       planes (the z direction) and then
       for each plane, loop through the data (the x and y
       directions). Record minimum, max and percent bad
       for each plane, and print out. */

    for (j=0; j< Fhdr->nz; j++){ /* Loop through planes. */

      total = Fhdr->nx * Fhdr->ny; /* Total number of data points. */
      numbad = 0; numgood=0;
      data_total=0.0;

      databytes= (unsigned char *)(Handle.field_plane[i][j]);
      first=1;

      for(k=0; k<total;k++){ /* Loop through the data in x and y */
	if ((databytes[k] == (unsigned char) Fhdr->missing_data_value ) ||
	    (databytes[k] == (unsigned char) Fhdr->bad_data_value )){

	  /* Bad data. */
	  numbad ++;

	} else {

	  numgood++;

	  /* Good data - decode the byte into a physical value,
	     using the scale and bias. */

	  val = databytes[k] * Fhdr->scale + Fhdr->bias;
	  data_total = data_total + val;
	  if (first) {
	    min=val; max=val; first=0;
	  } else {
	    if (val < min) min = val;
	    if (val > max) max = val;
	  }
	}
      }

      /* Calculate the percent bad and print it. */
      pbad=(int)(100.0*((double)(numbad) / (double)(total)));
      fprintf(stdout,"\tPlane %d : %ld of %ld are bad or missing (%d percent)\n",
	      j+1,numbad,total,pbad);

      /* If the field was not 100% bad, print the min and max. */
      if (!(first)){
	fprintf(stdout,"\tData values range from %g to %g\n",
		min,max);

	mean = data_total/numgood;



	/* Take another hit at the data to get the variance. */

	sumsqr=0.0;
	for(k=0; k<total;k++){ /* Loop through the data in x and y */
	  if ((databytes[k] != (unsigned char) Fhdr->missing_data_value ) &&
	      (databytes[k] != (unsigned char) Fhdr->bad_data_value )){


	    val = databytes[k] * Fhdr->scale + Fhdr->bias;
	    sumsqr = sumsqr + (val-mean)*(val-mean);

	  }
	}

	var = sumsqr / numgood;
	
	fprintf(stdout,"\tMean : %g Variance : %g Standard Deviation : %g\n",
		mean,var,sqrt(var));
      }


    } /* End of loop through data for a plane.*/

  } /* End of loop through planes. */


  /* Let go of the MDV handle. */
  MDV_free_handle(&Handle);

  /* All done. */

  return 0;

}








