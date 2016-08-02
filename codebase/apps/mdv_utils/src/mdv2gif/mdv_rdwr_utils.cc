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
/******************************************************
* mdv_rdwr_utils.cpp                                  *
* Utility to read mdv files, work with the data as a  *
* float array, and write the new float to a mdv file  *
******************************************************/

#include "mdv_rdwr_utils.hh"
#include <stdlib.h>

void alloc_arrays(mdv_float_handle *mdv_f)
{
  MDV_field_header_t *fhdr;
  int ifield,
      iz;

  mdv_f->f_array = 
        (float ***) malloc ( mdv_f->mdv.master_hdr.n_fields * sizeof(float **)); 
  mdv_f->c_array = 
        (ui08 ***) malloc ( mdv_f->mdv.master_hdr.n_fields * sizeof(ui08 **)); 
  for(ifield = 0; ifield < mdv_f->mdv.master_hdr.n_fields; ifield++) 
  {

    fprintf(stdout, "------>Field: %d\n", ifield);
    fhdr = mdv_f->mdv.fld_hdrs + ifield;

/*
 * Allocate memory to out.
 */
    mdv_f->f_array[ifield] =
           (float **) malloc ( fhdr->nz * sizeof(float *));
    mdv_f->c_array[ifield] =
           (ui08 **) malloc ( fhdr->nz * sizeof(ui08 *));

    printf("nz %d\n",fhdr->nz);

    for (iz = 0; iz < fhdr->nz ; iz++) 
    {
      mdv_f->f_array[ifield][iz] =
             (float *) malloc ( fhdr->nx * fhdr->ny * sizeof(float));
      mdv_f->c_array[ifield][iz] =
             (ui08 *) malloc ( fhdr->nx * fhdr->ny * sizeof(ui08));
    } /* iz */


  } /* ifield */
  

}


void fill_float_array(mdv_float_handle *mdv_f)
{
  ui08 ***fdata;

  fl32 Scale;
  fl32 Bias;
  int ifield,
      iz,ixy;

  fdata = (ui08 ***) mdv_f->mdv.field_plane;

  for(ifield = 0; ifield < mdv_f->mdv.master_hdr.n_fields; ifield++)
  {
    Scale = mdv_f->mdv.fld_hdrs[ifield].scale;
    Bias = mdv_f->mdv.fld_hdrs[ifield].bias;

    for (iz = 0; iz < mdv_f->mdv.fld_hdrs->nz ; iz++) 
    {
      for (ixy = 0; ixy < (mdv_f->mdv.fld_hdrs->ny*mdv_f->mdv.fld_hdrs->nx) ; ixy++)
      {
        mdv_f->f_array[ifield][iz][ixy] =
               (float)(fdata[ifield][iz][ixy] * Scale + Bias);
      }

    } /* iz */
    
  } /* ifield */

}

  

void free_float_handle(mdv_float_handle *mdv_f)
{
  int ifield, iz;

  for(ifield=0; ifield<mdv_f->mdv.master_hdr.n_fields; ifield++)
  {
    for(iz=0; iz<mdv_f->mdv.fld_hdrs[ifield].nz; iz++)
    {
      free(mdv_f->f_array[ifield][iz]);
      free(mdv_f->c_array[ifield][iz]);
    }
    free(mdv_f->f_array[ifield]);
    free(mdv_f->c_array[ifield]);
  }
    
  free(mdv_f->f_array);
  free(mdv_f->c_array);
  MDV_free_handle(&mdv_f->mdv);
}


extern void float2byte(mdv_float_handle *mdv_f)
{
  float min,max;
  float range;

  int ifield, iz, ixy;
  

  //determine the min and max, and initialize with extreme values
  for (ifield = 0; ifield < mdv_f->mdv.master_hdr.n_fields; ifield++)
  {
     min = max = mdv_f->f_array[0][0][0];
     for( iz =0; iz < mdv_f->mdv.fld_hdrs[ifield].nz ;iz++)
     {
       for( ixy = 0; ixy < mdv_f->mdv.fld_hdrs[ifield].ny * mdv_f->mdv.fld_hdrs[ifield].nx; ixy++)
       {
         if( min > mdv_f->f_array[ifield][iz][ixy] )
           min = mdv_f->f_array[ifield][iz][ixy];
         if( max < mdv_f->f_array[ifield][iz][ixy] )
           max = mdv_f->f_array[ifield][iz][ixy];
       }
     }
     printf("min %f max %f \n",min,max);
   
     range = max-min;
     mdv_f->mdv.fld_hdrs[ifield].scale = range/250.0;
     mdv_f->mdv.fld_hdrs[ifield].bias = min - ( mdv_f->mdv.fld_hdrs[ifield].scale ) * 2.0;
     printf("scale %f bias %f \n",mdv_f->mdv.fld_hdrs[ifield].scale, mdv_f->mdv.fld_hdrs[ifield].bias);

     for( iz =0; iz < mdv_f->mdv.fld_hdrs[ifield].nz ;iz++)
     {
       for( ixy = 0; ixy < mdv_f->mdv.fld_hdrs[ifield].ny * mdv_f->mdv.fld_hdrs[ifield].nx; ixy++)
       {
          mdv_f->c_array[ifield][iz][ixy] = (ui08 )
                  ((mdv_f->f_array[ifield][iz][ixy] - mdv_f->mdv.fld_hdrs[ifield].bias)/
                  (mdv_f->mdv.fld_hdrs[ifield].scale) + 0.5);
         /*printf("\n c_val[%d][%d][%d] %d",ifield,iz,ixy,mdv_f->c_array[ifield][iz][ixy]);*/
       }
     }/*for iz*/
   }/* for ifield*/
 }
     


extern void Set_Vol_Hdrs(mdv_float_handle *mdv_f)
{
  int ifield;

  /* clear the master header */
  MDV_master_header_t *mhdr = &mdv_f->mdv.master_hdr;

  /* fill the master header */
  MDV_alloc_handle_arrays(&mdv_f->mdv, mhdr->n_fields,
                          mhdr->max_nz, mhdr->n_chunks);

  /* fill in field headers and vlevel headers */
  for(ifield = 0; ifield < mhdr->n_fields; ifield++)
  {
//      MDV_field_header_t *fhdr = mdv_f->mdv.fld_hdrs + ifield;
//      MDV_vlevel_header_t *vhdr = mdv_f->mdv.vlv_hdrs + ifield;
  } /* ifield */

}
 
extern int Write_Complete_Vol(mdv_float_handle *mdv_f,char *dirout)
{
  int ifield,iz;
  int iret;

  MDV_master_header_t *mhdr = &mdv_f->mdv.master_hdr;

  mhdr->time_gen = time(NULL);

  /* set field plane pointers into data arrays */
  for(ifield = 0; ifield < mhdr->n_fields; ifield++)
  {
    MDV_field_header_t *fhdr = mdv_f->mdv.fld_hdrs + ifield;
    for(iz = 0; iz < fhdr->nz; iz++)
    {
      mdv_f->mdv.field_plane[ifield][iz] = mdv_f->c_array[ifield][iz];
    /*  printf("c_array[a][b][50] %d\n",mdv_f->c_array[ifield][iz][50]);    */
   /*   printf("field_plane[0][i][50] %d\n",(ui08)(mdv_f->mdv.field_plane[ifield][iz][50]));*/
      printf("setting field plane %d \n",iz);
    } /* iz */
  } /* ifield */

  printf("Setting stderr \n");
  fprintf(stderr,"Writing MDV file, time %s, to dir %s\n",
         utimstr(mhdr->time_centroid), dirout);

  printf(" start MDV_write_to_dir \n");
  iret = MDV_write_to_dir(&mdv_f->mdv, dirout,
                              MDV_PLANE_RLE8, TRUE);
  printf(" finished MDV_write_to_dir \n");

  /* reset the field planes to NULL so they are not freed by MDV routines*/

  for(ifield = 0; ifield < mhdr->n_fields; ifield++)
  {
    MDV_field_header_t *fhdr = mdv_f->mdv.fld_hdrs + ifield;
    for(iz = 0; iz < fhdr->nz; iz++)
    {
       mdv_f->mdv.field_plane[ifield][iz] = NULL;
    } /* iz */
  } /* ifield */

  if(iret == MDV_SUCCESS)
  {
    return (0);
  }
  else
  {
    return(-1);
  }

}

extern void Set_Header_Time(mdv_float_handle *mdv_f, int timetostart)
{
  // Create a pointer to the master header
  MDV_master_header_t *mhdr = &mdv_f->mdv.master_hdr;

  // Set the time in unix time
  mhdr->time_begin = mdv_f->dtime.unix_time - timetostart;
  mhdr->time_end = mdv_f->dtime.unix_time; 
  mhdr->time_centroid = mhdr->time_begin 
             + (mhdr->time_end - mhdr->time_begin)/2;
  mhdr->time_expire = mdv_f->dtime.unix_time;
}
