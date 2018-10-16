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
/*************************************************************************
 * history_forecast.c
 *
 * Utility routines for history_forecast stuff
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, Sept, 1996.
 *
 *************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <rapformats/hist_fore.h>

/*************************************************************************/
/* Allocate space for an hf_file struct.
 * Assumes that basic struct is there and allocates space for data pointed
 * to by pointers in that struct 
 * User is responsible for freeing memory.
 */

int calloc_hf_struct(hf_file_t *hff, int nsta, int npts)
{
   int i;

    /* Allocate a data struct for each station. */
    if ((hff->sta_data = (sta_data_t *)ucalloc
                                      (nsta, sizeof(sta_data_t)))==NULL) {
       fprintf(stderr,"\ncalloc_hf_struct Error. Can't calloc hff->sta_data\n");
       return(FAILURE);
    }

    for (i = 0; i < nsta; i++ ) {
    /* allocate history/forecast space -- how many is set by user. */

       if ((hff->sta_data[i].hf_data = (hf_data_t *)ucalloc
                                        (npts, sizeof(hf_data_t)))==NULL) {
          fprintf(stderr,"\ncalloc_hf_struct Error. Can't calloc hff->sta_data hf_data space\n");
          return(FAILURE);
       }

    } /*end looping through stations  */

    return(SUCCESS);
}

/***************************************************************************
 * Make an history_forecasst spdb history forecast chunk into big_endian   
 * Note, right now no error checking.  If data doesn't need to be swapped
 * then swapped will be 0.  No real way to check?
 *
 ***************************************************************************/


int hf_chunk_bigend(hf_chunk_t **hfc) 

{
  /* int swapped; */
   int nsta,npts,ista;
   hf_chunk_t *lhfc;
   sta_chunk_t *sc_ptr;
   hf_data_t *hf_ptr;
   int skip_bytes;

   lhfc = *hfc;
   npts = lhfc->n_history + lhfc->n_forecast;
   nsta = lhfc->n_stations;

/* swap hf_file struct */
   /* swapped = */
   BE_from_array_32(lhfc,NUM_HFF_32*sizeof(ui32));

   sc_ptr = lhfc->sta_chunk;

/* swap each stations info */
   for (ista = 0; ista < nsta; ista++) {

      skip_bytes = ista*(sizeof(sta_chunk_t)-
                   sizeof(hf_data_t)+npts*sizeof(hf_data_t));
      sc_ptr = (sta_chunk_t *)((char *)lhfc->sta_chunk + skip_bytes);

      /* swapped = */
      BE_from_array_32(sc_ptr,NUM_STA_32*sizeof(ui32));

      hf_ptr = (hf_data_t *)((char *)sc_ptr+sizeof(sta_chunk_t) - 
                             sizeof(hf_data_t));
       
      /* swap data info */
      /* swapped = */
      BE_from_array_32(hf_ptr,npts*NUM_HFD_32*sizeof(ui32));

   }

   return(0);
}

/***************************************************************************
 * swap an hf_chunk struct from Big-endian to native format 
 ***************************************************************************/


int hf_chunk_native(hf_chunk_t **hfc) 

{
   /* int swapped; */
   int nsta,npts,ista;
   hf_chunk_t *lhfc;
   sta_chunk_t *sc_ptr;
   hf_data_t *hf_ptr;
   int skip_bytes;

   lhfc = *hfc;

/* swap hf_chunk_t */
   /* swapped = */
   BE_to_array_32(lhfc,NUM_HFF_32*sizeof(ui32));

   npts = lhfc->n_history + lhfc->n_forecast;
   nsta = lhfc->n_stations;

   sc_ptr = lhfc->sta_chunk;

   for (ista = 0; ista < nsta; ista++) {

      skip_bytes = ista*(sizeof(sta_chunk_t)-sizeof(hf_data_t) +
                   npts*sizeof(hf_data_t));
      sc_ptr = (sta_chunk_t *)((char *)lhfc->sta_chunk + skip_bytes);

      /* swap each station's info */
      /* swapped = */
      BE_to_array_32(sc_ptr,NUM_STA_32*sizeof(ui32));

      hf_ptr = (hf_data_t *)((char *)sc_ptr + sizeof(sta_chunk_t) - 
                             sizeof(hf_data_t));
       
      /* swap each station's data */
      /* swapped = */
      BE_to_array_32(hf_ptr,npts*NUM_HFD_32*sizeof(ui32));

   }

   return(0);
}

/*************************************************************************** 
 * puts an hf_chunk struct into a hf_file.  It assumes chunk is in
 * big-endian and puts hf_file struct in native format 
 * It allocates memory for the hf_file struct  so user is responsible 
 * for freeing it when done.
 * 
 ***************************************************************************/

int chunk2hf(hf_file_t *hff, hf_chunk_t *hfc,int hf_chunk_size)

{
   int npts,ista,i,nsta,isp;
   sta_chunk_t *sc_ptr;
   int skip_bytes;

/* make the chunk big-endian */
   if (hf_chunk_native(&hfc)!=SUCCESS) {
      fprintf(stderr,"hf_chunk_native not successfull\n");
      return(FAILURE);
   }

   npts = hfc->n_history + hfc->n_forecast;
   nsta = hfc->n_stations;

   if (calloc_hf_struct(hff, nsta, npts ) != SUCCESS) {
      fprintf(stderr,"\nchunk2hf error. Can't calloc hf_file space\n");
      return(FAILURE);
   }

/* assign stuff */
   hff->time            = hfc->time;
   hff->forecast_length = hfc->forecast_length;
   hff->n_stations      = hfc->n_stations;
   hff->n_history       = hfc->n_history;
   hff->n_forecast      = hfc->n_forecast;
   hff->radar_lat       = hfc->radar_lat;
   hff->radar_lon       = hfc->radar_lon;
   hff->radar_alt       = hfc->radar_alt;
   hff->elev_angle      = hfc->elev_angle;
   hff->elev_km         = hfc->elev_km;
   hff->ave_u           = hfc->ave_u;
   hff->ave_v           = hfc->ave_v;
   strncpy(hff->instance, hfc->instance,HF_STRSIZE);

   for (isp=0; isp < NUM_HF_ISPARE; isp ++) 
      hff->ispare[isp]  = hfc->ispare[isp];

   for (isp=0; isp < NUM_HF_FSPARE; isp ++) 
      hff->fspare[isp]  = hfc->fspare[isp];

   for (ista = 0; ista < hff->n_stations; ista++) {

      skip_bytes = ista*(sizeof(sta_chunk_t)-sizeof(hf_data_t)+
                   npts*sizeof(hf_data_t));
      sc_ptr = (sta_chunk_t *)((char *)hfc->sta_chunk + skip_bytes);

      hff->sta_data[ista].station_lon = sc_ptr->station_lon;
      hff->sta_data[ista].station_lat = sc_ptr->station_lat;
      strncpy(hff->sta_data[ista].station_name,sc_ptr->station_name, HF_STRSIZE);

      for (i = 0; i< npts; i++) {
        if (i < hff->n_history) {
           hff->sta_data[ista].hf_data[i].time   =sc_ptr->hf_data[i].time;
           hff->sta_data[ista].hf_data[i].dbz    =sc_ptr->hf_data[i].dbz;
           hff->sta_data[ista].hf_data[i].umotion=sc_ptr->hf_data[i].umotion;
           hff->sta_data[ista].hf_data[i].vmotion=sc_ptr->hf_data[i].vmotion;
           hff->sta_data[ista].hf_data[i].dbzf   =sc_ptr->hf_data[i].dbzf;
        }
        else {
        
           hff->sta_data[ista].hf_data[i].time    = sc_ptr->hf_data[i].time;
           hff->sta_data[ista].hf_data[i].dbz     = 0.0;
           hff->sta_data[ista].hf_data[i].umotion = 0.0;
           hff->sta_data[ista].hf_data[i].vmotion = 0.0;
           hff->sta_data[ista].hf_data[i].dbzf    = sc_ptr->hf_data[i].dbzf; 
        }

        for (isp=0; isp < NUM_HF_SPARE; isp ++) 
           hff->sta_data[ista].hf_data[i].spare[isp] = 0.0;

      }
   }

   return(SUCCESS);

}

/*************************************************************************** 
 * puts an hf_file struct into a continuous hf_chunk struct.  Also makes it
 * big-endian!!!!
 * It allocates memory for chunk so user is responsible for freeing it 
 * when done.
 ***************************************************************************/

int hf2chunk(hf_file_t *hff, hf_chunk_t **hfc,int *hf_chunk_size)

{
   int npts,ista,i,isp;
   hf_chunk_t *lhfc;
   sta_chunk_t *sc_ptr;
   int skip_bytes;

   npts = hff->n_history + hff->n_forecast;

    umalloc_verify();
   *hf_chunk_size = sizeof(hf_chunk_t) - sizeof(sta_chunk_t) + 
                    hff->n_stations * (sizeof(sta_chunk_t) - 
                    sizeof(hf_data_t)) + hff->n_stations *
                    (npts * sizeof(hf_data_t));

    umalloc_verify();
   if ((lhfc = (hf_chunk_t *)ucalloc(1,*hf_chunk_size)) == NULL) {
      fprintf(stderr,"problems callocing hf_chunk data \n");
      return(FAILURE);
   }
   

    umalloc_verify();
/* assign stuff */
   lhfc->time            = hff->time;
   lhfc->forecast_length = hff->forecast_length;
   lhfc->n_stations      = hff->n_stations;
   lhfc->n_history       = hff->n_history;
   lhfc->n_forecast      = hff->n_forecast;
   lhfc->radar_lat       = hff->radar_lat;
   lhfc->radar_lon       = hff->radar_lon;
   lhfc->radar_alt       = hff->radar_alt;
   lhfc->elev_angle      = hff->elev_angle;
   lhfc->elev_km         = hff->elev_km;
   lhfc->radar_lat       = hff->radar_lat;
   lhfc->ave_u           = hff->ave_u;
   lhfc->ave_v           = hff->ave_v;
   strncpy(lhfc->instance, hff->instance,HF_STRSIZE);

    umalloc_verify();
   for (isp=0; isp < NUM_HF_ISPARE; isp ++) 
      lhfc->ispare[isp]  = hff->ispare[isp];

   for (isp=0; isp < NUM_HF_FSPARE; isp ++) 
      lhfc->fspare[isp]  = hff->fspare[isp];

   for (ista = 0; ista < hff->n_stations; ista++) {

      skip_bytes = ista*(sizeof(sta_chunk_t)-sizeof(hf_data_t)+
                   npts*sizeof(hf_data_t));
      sc_ptr = (sta_chunk_t *)((char *)lhfc->sta_chunk + skip_bytes);

      sc_ptr->station_lon = hff->sta_data[ista].station_lon;
      sc_ptr->station_lat = hff->sta_data[ista].station_lat;
      strncpy(sc_ptr->station_name,hff->sta_data[ista].station_name,
             HF_STRSIZE);

    umalloc_verify();
      for (i = 0; i< npts; i++) {
        if (i < hff->n_history) {
           sc_ptr->hf_data[i].time   =hff->sta_data[ista].hf_data[i].time; 
           sc_ptr->hf_data[i].dbz    =hff->sta_data[ista].hf_data[i].dbz; 
           sc_ptr->hf_data[i].umotion=hff->sta_data[ista].hf_data[i].umotion;
           sc_ptr->hf_data[i].vmotion=hff->sta_data[ista].hf_data[i].vmotion;
           sc_ptr->hf_data[i].dbzf   =hff->sta_data[ista].hf_data[i].dbzf; 
        }
        else {
        
           sc_ptr->hf_data[i].time    = hff->sta_data[ista].hf_data[i].time; 
           sc_ptr->hf_data[i].dbz     = 0.0;
           sc_ptr->hf_data[i].umotion = 0.0;
           sc_ptr->hf_data[i].vmotion = 0.0;
           sc_ptr->hf_data[i].dbzf    = 
                              hff->sta_data[ista].hf_data[i].dbzf; 
        }

        for (isp=0; isp < NUM_HF_SPARE; isp ++) 
           sc_ptr->hf_data[i].spare[isp] = 0.0;

      }
   }

    umalloc_verify();
/* make the chunk big-endian */
   if (hf_chunk_bigend(&lhfc)!=SUCCESS) {
      fprintf(stderr,"hf_chunk_bigend not successfull\n");
      return(FAILURE);
   }

    umalloc_verify();
   *hfc = lhfc;

    umalloc_verify();
   return(SUCCESS);

}

/*************************************************************************** 
 * Prints a hf_file struct  - always expects chunk in native format * 
 ***************************************************************************/


void print_hf_file(FILE *stream,hf_file_t *hff)
{
   int ihf,ista;

   fprintf(stream,"\nPrinting history/forecast struct.  \n");

   fprintf(stream,"hff->time:	 	%d\n", hff->time);
   fprintf(stream,"hff->forecast_length:%d\n", hff->forecast_length);
   fprintf(stream,"hff->n_stations: 	%d\n", hff->n_stations);
   fprintf(stream,"hff->n_history: 	%d\n", hff->n_history);
   fprintf(stream,"hff->n_forecast: 	%d\n", hff->n_forecast);
   fprintf(stream,"hff->radar_lat: 	%f\n", hff->radar_lat);
   fprintf(stream,"hff->radar_lon: 	%f\n", hff->radar_lon);
   fprintf(stream,"hff->radar_alt: 	%f\n", hff->radar_alt);
   fprintf(stream,"hff->elev_angle: 	%f\n", hff->elev_angle);
   fprintf(stream,"hff->elev_km: 	%f\n", hff->elev_km);
   fprintf(stream,"hff->ave_u: 		%f\n", hff->ave_u);
   fprintf(stream,"hff->ave_v: 		%f\n", hff->ave_v);
   fprintf(stream,"hff->instance: 	%s\n", hff->instance);

   for (ista=0; ista<hff->n_stations; ista++) {
      fprintf(stream,"Station %s at (%f,%f) \n",
                      hff->sta_data[ista].station_name,
                      hff->sta_data[ista].station_lon,
                      hff->sta_data[ista].station_lat);

      for (ihf=0; ihf<hff->n_history+hff->n_forecast; ihf++) {
         if (ihf < hff->n_history) {
            fprintf(stream,"history pt %d %d %f %f %f %f \n",ihf,
                   hff->sta_data[ista].hf_data[ihf].time,
                   hff->sta_data[ista].hf_data[ihf].dbz,
                   hff->sta_data[ista].hf_data[ihf].umotion,
                   hff->sta_data[ista].hf_data[ihf].vmotion,
                   hff->sta_data[ista].hf_data[ihf].dbzf);
         }
         else {
            fprintf(stream,"forecast pt %d %d %f \n",ihf,
                   hff->sta_data[ista].hf_data[ihf].time,
                   hff->sta_data[ista].hf_data[ihf].dbzf);
         }
      }
   }
   fprintf(stream,"Done printing hf_file struct.  \n");
}

/*************************************************************************** 
 * Prints a hf_chunk struct  - always expects chunk in native format 
 ***************************************************************************/

void print_hf_chunk(FILE *stream, hf_chunk_t *hfc, int chunk_size)
{
   int ista,skip_bytes,npts,ihf;
   sta_chunk_t *sc_ptr;

   fprintf(stream,"hfc->time:	 	%d\n", hfc->time);
   fprintf(stream,"hfc->forecast_length:%d\n", hfc->forecast_length);
   fprintf(stream,"hfc->n_stations: 	%d\n", hfc->n_stations);
   fprintf(stream,"hfc->n_history: 	%d\n", hfc->n_history);
   fprintf(stream,"hfc->n_forecast: 	%d\n", hfc->n_forecast);
   fprintf(stream,"hfc->radar_lat: 	%f\n", hfc->radar_lat);
   fprintf(stream,"hfc->radar_lon: 	%f\n", hfc->radar_lon);
   fprintf(stream,"hfc->radar_alt: 	%f\n", hfc->radar_alt);
   fprintf(stream,"hfc->elev_angle: 	%f\n", hfc->elev_angle);
   fprintf(stream,"hfc->elev_km: 	%f\n", hfc->elev_km);
   fprintf(stream,"hfc->ave_u: 		%f\n", hfc->ave_u);
   fprintf(stream,"hfc->ave_v: 		%f\n", hfc->ave_v);
   fprintf(stream,"hfc->instance: 	%s\n", hfc->instance);

   npts = hfc->n_history + hfc->n_forecast;

   for (ista=0; ista<hfc->n_stations; ista++) {

      skip_bytes = ista*(sizeof(sta_chunk_t)-
                   sizeof(hf_data_t)+npts*sizeof(hf_data_t));
      sc_ptr = (sta_chunk_t *)((char *)hfc->sta_chunk + skip_bytes);

      fprintf(stream,"station %s at (%f,%f) \n",sc_ptr->station_name,
               sc_ptr->station_lon,sc_ptr->station_lat);

      for (ihf=0; ihf<hfc->n_history+hfc->n_forecast; ihf++) {

         if (ihf < hfc->n_history) {

            fprintf(stream,"%d %d %f %f %f %f \n",ihf,
                   sc_ptr->hf_data[ihf].time,
                   sc_ptr->hf_data[ihf].dbz,
                   sc_ptr->hf_data[ihf].umotion,
                   sc_ptr->hf_data[ihf].vmotion,
                   sc_ptr->hf_data[ihf].dbzf);
         }
         else {

            fprintf(stream,"%d %d %f \n",ihf,
                   sc_ptr->hf_data[ihf].time,
                   sc_ptr->hf_data[ihf].dbzf);
         }
      }
   }

   fprintf(stream,"Done printing hf_chunk.  \n");
}

#ifdef __cplusplus
}
#endif
