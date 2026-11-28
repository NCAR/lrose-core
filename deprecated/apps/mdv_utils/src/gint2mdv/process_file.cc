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
 * process_file.cc
 *
 * Convert a gint file into an mdv file. 
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, January, 1996.
 *
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "gint2mdv.h"
using namespace std;

int process_file(char *input_file_name)

{

   Tvolume_header gh;            	/* gint header pointer*/
   MDV_master_header_t mmh;            	/* mdv master header   */
   MDV_field_header_t  *mfh; 		/* mdv field headers   */
   MDV_vlevel_header_t *mvh; 		/* mdv vlevel headers  */

   unsigned char *raw_buf; 		/* buffer for raw gint data */
   unsigned char *buf[MAX_FIELDS]; 	/* flipped gint data */

   int ifield,vlevel;                   /* loop variables */

   unsigned char *to_ptr, *from_ptr; 	/* used to flip data around x-axis */
   int row;				/* loop variables */
   int nx,ny,nz; 			/* temporary variables */

   char *output_file_name;		/* ascii name of output (mdv) file */

   FILE *infile, *outfile;              /* gint (infile) and mdv (outfile) */

/*--------------------------------------------------------------------------------------*/

/* initialize structures */
   ZERO_STRUCT(&gh);
   ZERO_STRUCT(&mmh);

/* convert file from gint to mdv format */
   if (gd->params.debug)
      fprintf(stdout,"\nProcessing file %s \n",input_file_name);

/* open gint file */
   if ((infile = ta_fopen_uncompress(input_file_name,"r")) == NULL) {
      fprintf(stderr,"\nError opening input file: %s\n",input_file_name);
      return(MDV_FAILURE);
   }

/* read in gint header */
   if (GINT_get_header(&gh, infile))  {
      fprintf(stderr,"\nError occurred during read header of %s\n",
              input_file_name);
      return(MDV_FAILURE);
   }

/* fill in all applicable master_header values with gint_header values */
   if (fill_mdv_master_header(&gh,&mmh)) {
      fprintf(stderr,"\nError occurred during fill_mdv_master_header \n");
      return(MDV_FAILURE);
   }

/* print master header if requested */
   if (gd->params.debug) MDV_print_master_header(&mmh,stdout);

/* create mdv file name based on time of data */
   output_file_name = create_mdv_file_name(&mmh);

/* open output (mdv) file */
   if ((outfile = fopen(output_file_name,"w")) == NULL) {
      fprintf(stderr,"\nError opening output file: %s",output_file_name);
      return(MDV_FAILURE);
   }
   
/* write master header int file */
    
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

   /* allocate memory for all mdv vlevel headers */
   if (mmh.vlevel_included) {   
      if ((mvh = (MDV_vlevel_header_t *) 
         calloc(mmh.n_fields,sizeof(MDV_vlevel_header_t))) == NULL) {
         fprintf(stderr,"\ngint2mdv: Error at calloc of mdv vlevel headers.\n");
         return(MDV_FAILURE);
      }
   } /*endif vlevels are included in this dataset */

/* get gint data and fill in all mdv field and vlevel headers */
   for (ifield = 0; ifield < gh.vh->n_fields ; ifield++) {

      if (fill_mdv_field_header(&gh,&mmh,&(mfh[ifield]),ifield)) {
         fprintf(stderr,"\nError occurred during fill_mdv_field_header \n");
         return(MDV_FAILURE);
      }

      if (gd->params.debug) MDV_print_field_header(&(mfh[ifield]),stdout);

      /* write out field headers to file*/
      if ((MDV_write_field_header(outfile,&(mfh[ifield]),ifield)) == MDV_FAILURE)  {
         fprintf(stderr,"\nError occurred during MDV_write_field_header\n");
         return(MDV_FAILURE);
      }

      /* fill vlevel headers */
      if (fill_mdv_vlevel_header(&gh,mmh,&(mvh[ifield]))) {
         fprintf(stderr,"\nError occurred during fill_mdv_vlevel_header \n");
         return(MDV_FAILURE);
      }

      if (gd->params.debug) 
         MDV_print_vlevel_header(&(mvh[ifield]),mfh[ifield].nz, 
                                 (char *)mfh[ifield].field_name_long,stdout);

   } /* end filling in field and vlevel headers */


/* write out all the vlevel headers and read in data*/

   if (gd->params.debug)
      fprintf(stdout,"\nReading gint data from: %s ",input_file_name);

   for (ifield = 0; ifield < gh.vh->n_fields ; ifield++) {

       nx = mfh[ifield].nx;
       ny = mfh[ifield].ny;
       nz = mfh[ifield].nz;

      /* write out vlevel headers */
      if ((MDV_write_vlevel_header(outfile,&(mvh[ifield]),
                                   &mmh,ifield)) == MDV_FAILURE)  {
            fprintf(stderr,"\nError occurred during MDV_write_vlevel_header\n");
      } 

      /* read in gint field data plane (vlevel) by plane (vlevel) */

      /* allocate buffer space for all planes of flipped data */
      if ((buf[ifield] = (unsigned char *)
         calloc(nz,nx*ny*sizeof(unsigned char))) == NULL) {
         fprintf(stderr,"\nError occurred during calloc of mdv buf space\n");
         return(MDV_FAILURE);
      }

      to_ptr = buf[ifield];
      /* read in each level at a time and flip */
      for (vlevel = 0; vlevel < nz; vlevel ++ ) {

         if ((raw_buf=(unsigned char *)GINT_get_plane(ifield, vlevel, infile, &gh))==NULL)  {
            fprintf(stderr,"\nError occurred during GINT_get_plane, field %d\n",ifield);
            return(MDV_FAILURE);
          }


         /* flip data around so 0,0 point at lower lefthand corner 
          * instead of upper right */


         from_ptr = raw_buf + (ny - 1)*(nx);

         for (row = 0; row < ny; row ++) {
            memcpy(to_ptr, from_ptr,nx*sizeof(unsigned char));
            to_ptr   += nx;
            from_ptr -= nx;
         }

         free(raw_buf);
         raw_buf = NULL;

      } /* end reading and rearranging gint data for this vlevel*/

   } /* end writing vlevel headers and reading data */


/* write out data */

   if (gd->params.debug)
      fprintf(stdout,"\nWriting out mdv file: %s \n",output_file_name);

   for (ifield = 0; ifield < mmh.n_fields ; ifield++) {

      /* write unencoded data */
      if (gd->params.output_encoding_type == 0) { 

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

   /* write updated current_index file */
   if (gd->params.mode == REALTIME) {
  
      if (update_cindex(mmh.time_centroid) != MDV_SUCCESS) {
         fprintf(stderr,"\nError writing current_index"); 
         return(MDV_FAILURE);
      }
   } /* endif needed to write out a current index file */

/* clean up */
   GINT_free_header(&gh);
   free(mfh);
   free(mvh);
   mfh    = NULL;
   mvh    = NULL;
   fclose(infile);
   fclose(outfile);
   outfile = NULL;

   return(MDV_SUCCESS);

}  /* end processin this file */

#ifdef __cplusplus
}
#endif
