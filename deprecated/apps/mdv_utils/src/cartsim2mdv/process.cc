// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * process.cc
 *
 * Convert cartesian simulation data into mdv data files.
 *
 * 
 *
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "cartsim2mdv.h"
#include <rapformats/var_elev.h>
#include <ctetwws/smem.h>
using namespace std;

/*----------------------------------------------------------------*/
static void *build_fake_chunk(MDV_master_header_t *mmh, int ichunk)
{
    void *c;
    float *elevs;
    int naz, i, len;

    naz = mmh->max_ny;
    elevs = MEM_CALLOC(naz, float);
    for (i=0; i<naz; ++i)
	elevs[i] = 0.1*i;
    c = VAR_ELEV_build(elevs, naz, &len);
    MEM_FREE(elevs);
    return c;
}


/*----------------------------------------------------------------*/
int process(MDV_SIM_inputs_t *in, CART_SIM_grid_parms_t *G)
{
   MDV_master_header_t mmh;            	/* mdv master header   */
   MDV_field_header_t  *mfh=NULL;	/* mdv field headers   */
   MDV_vlevel_header_t *mvh=NULL;	/* mdv vlevel headers  */
   MDV_chunk_header_t *mch=NULL;        /* mdv chunk headers */
   unsigned char *raw_buf=NULL;		/* buffer for raw gint data */
   unsigned char *buf[MAX_FIELDS];	/* flipped gint data */
   void *chunk_data=NULL;
   int ichunk, ifield,vlevel;           /* loop variables */
   int nx,ny,nz,nchunk;			/* temporary variables */
   char *output_file_name;		/* ascii name of output (mdv) file */
   FILE *outfile;                       /* mdv (outfile) */
   unsigned char *toptr;


   ZERO_STRUCT(&mmh);

   /*
    * Use information from cart sim for mdv header
    */
   if (MDV_SIM_fill_master_header(in, &mmh,
				  gd->params.collection_type,
				  gd->params.vertical_type,
				  gd->params.grid_order_direction,
				  gd->params.sensor_latitude,
				  gd->params.sensor_longitude,
				  gd->params.sensor_altitude,
				  gd->params.data_source)
				   == MDV_SIM_BAD)
   {
      fprintf(stderr,"\nError occurred during fill_mdv_master_header \n");
      return(MDV_FAILURE);
   }
   /*print master header if requested */
   if (gd->params.debug)
       MDV_print_master_header(&mmh,stdout);

/* create mdv file name based on time of data */
   output_file_name = MDV_SIM_create_file_name(&mmh,
					       gd->params.output_dir,
					       gd->params.output_file_suffix);

/* open output (mdv) file */
   if ((outfile = fopen(output_file_name,"w")) == NULL) {
      fprintf(stderr,"\nError opening output file: %s",output_file_name);
      return(MDV_FAILURE);
   }
   else
   {
       if (gd->params.debug)
	   fprintf(stdout, "Opened output file %s\n", output_file_name);
   }

/* write master header into file */
   if (gd->params.debug)
       fprintf(stdout, "writing master header\n");
   if ((MDV_write_master_header(outfile,&mmh)) == MDV_FAILURE)  {
      fprintf(stderr,"\nError occurred during MDV_write_master_header\n");
      return(MDV_FAILURE);
   }

   /* allocate memory for all mdv field headers */
   if ((mfh = (MDV_field_header_t *) 
      calloc(mmh.n_fields,sizeof(MDV_field_header_t))) == NULL) {
      fprintf(stderr,"\ngint2mdv: Error at calloc of mdv field headers.\n");
      return(MDV_FAILURE);
   }


/*
 * get sim data and fill in all mdv field and vlevel headers.
 */
   for (ifield = 0; ifield < in->n_fields ; ifield++) {

      if (MDV_SIM_fill_field_header(G, &mmh, &(mfh[ifield]), ifield,
				    in->fields[ifield],
				    gd->params.sensor_latitude,
				    gd->params.sensor_longitude,
				    gd->params.sensor_altitude,
				    gd->params.projection_type)
	  == MDV_SIM_BAD )
      {
         fprintf(stderr,"\nError occurred during fill_mdv_field_header \n");
         return(MDV_FAILURE);
      }
      if (gd->params.debug) MDV_print_field_header(&(mfh[ifield]),stdout);

      /* write out field headers to file*/
      if ((MDV_write_field_header(outfile,&(mfh[ifield]),ifield)) == MDV_FAILURE)  {
         fprintf(stderr,"\nError occurred during MDV_write_field_header\n");
         return(MDV_FAILURE);
      }
      if (gd->params.debug)
	  fprintf(stdout, "wrote field header\n");

   } /* end filling in field and vlevel headers */

/* write out all the vlevel headers and create data*/
   for (ifield = 0; ifield < in->n_fields ; ifield++) {

       if (gd->params.debug)
	   fprintf(stdout, "Getting data for field %d\n", ifield);
       nx = mfh[ifield].nx;
       ny = mfh[ifield].ny;
       nz = mfh[ifield].nz;

      /* create simulated field data plane (vlevel) by plane (vlevel) */

      /* allocate buffer space for all planes of flipped data */
       if ((buf[ifield] = (unsigned char *)
	    calloc(nz,nx*ny*sizeof(unsigned char))) == NULL)
       {
	   fprintf(stderr,"\nError occurred during calloc of mdv buf space\n");
	   return(MDV_FAILURE);
       }

       toptr = buf[ifield];

       /* read in each level at a time*/
       for (vlevel = 0; vlevel < nz; vlevel ++ )
       {
	   if ((raw_buf=(unsigned char *)
		CARTSIM_get_plane(in->fields[ifield], vlevel))==NULL)
	   {
	       fprintf(stderr,
		       "\nError occurred during CARTSIM_get_plane, field %d\n",
		       ifield);
	       return(MDV_FAILURE);
	   }

	   /* Copy to buf */
	   memcpy((void *)toptr, (void *)raw_buf, nx*ny*sizeof(unsigned char));
	   free(raw_buf);
	   raw_buf = NULL;
	   toptr += nx*ny*sizeof(unsigned char);
       } /* end creating data for this vlevel*/

   } /* end writing vlevel headers and reading data */


/* write out data */

   if (gd->params.debug)
      fprintf(stdout,"\nWriting out mdv file: %s \n",output_file_name);

   for (ifield = 0; ifield < in->n_fields ; ifield++)
   {
      /* write unencoded data */
      if (gd->params.output_encoding_type == 0)
      { 
         if ((MDV_write_field_data(&(mfh[ifield]),ifield,mfh[ifield].field_data_offset,
               &buf[ifield][0], MDV_INT8,outfile)) <= 0) {
            fprintf(stderr,"\nfailure in MDV_write_field_data (unencoded), %d",ifield);
            return(MDV_FAILURE);
         }

      }  /* end writing out unencoded data */
  
      else if (gd->params.output_encoding_type == 1) { 

      /*  write out in MDV compressed (encoded) format */
         fprintf(stdout,"\nwriting encoded data ");
         if ((MDV_write_field_data(&(mfh[ifield]),ifield,mfh[ifield].field_data_offset,
               &buf[ifield][0], MDV_PLANE_RLE8,outfile)) <= 0) {
            fprintf(stderr,"\nfailure in MDV_write_field_data (encoded), %d",ifield);
            return(MDV_FAILURE);
         }

      /*  update location of next field_data_offset to reflect encoding */
         if (ifield + 1 < mmh.n_fields)
            mfh[ifield+1].field_data_offset = mfh[ifield].field_data_offset + 
                                               mfh[ifield].volume_size+2*sizeof(int);

      }  /* endif encoded data */
            
   free(buf[ifield]);
   buf[ifield] = NULL;
   }  /* end looping over fields */


   /* If appropriate, create chunk data headers and chunk*/
   if (gd->params.vertical_type == MDV_VERT_VARIABLE_ELEV)
       nchunk = 1;
   else if (gd->params.vertical_type == MDV_VERT_FIELDS_VAR_ELEV)
       nchunk = mmh.n_fields;
   else
       nchunk = 0;

   if (nchunk > 0)
   {
       if ((mch = (MDV_chunk_header_t *) 
	    calloc(1, sizeof(MDV_chunk_header_t))) == NULL) {
	   fprintf(stderr,"\ngint2mdv: Error at calloc of mdv chunk header.\n");
	   return(MDV_FAILURE);
       }
   }
   for (ichunk=0; ichunk<nchunk; ++ichunk)
   {
       if (MDV_SIM_fill_chunk_header(mch, &mmh, mfh, ichunk) == MDV_SIM_BAD)
       {
	   fprintf(stderr, "ERROR in filling chunk header\n");
	   return MDV_FAILURE;
       }

       /*
	* Fakey code here...
	*/
       chunk_data = build_fake_chunk(&mmh, ichunk);
       if (chunk_data == NULL)
       {
	   fprintf(stderr, "ERROR in building fake chunk data\n");
	   return MDV_FAILURE;
       }

       /*
	* NOTE assumes uncompressed data
	*/
       if (MDV_write_chunk(outfile, mch, chunk_data, &mmh, ichunk,
			   mch->chunk_data_offset, TRUE) == MDV_FAILURE)
       {
	   fprintf(stderr, "ERROR in writing chunk data to file\n");
	   return MDV_FAILURE;
       }
       VAR_ELEV_destroy(&chunk_data);
   }

/* clean up */
   if (gd->params.debug)
       fprintf(stdout, "DONE WITH THIS VOLUME\n");
   free(mfh);
   mfh    = NULL;
   mvh    = NULL;
   if (mch != NULL)
       free(mch);
   mch = NULL;
   fclose(outfile);
   outfile = NULL;
   return (MDV_SUCCESS);

}  /* end processin this file */

#ifdef __cplusplus
}
#endif
