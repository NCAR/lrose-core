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
/***********************************************************************
 *
 * read_basic_data() - track_client routine
 *
 * Read basic track data set
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1992
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <toolsa/umisc.h>
#include <toolsa/xdru.h>

#include <titan/tdata_index.h>

extern int read_from_buffer(int sockfd, char *data, si32 nbytes, int id);

int read_basic_data(int sockfd, int print_on)
{
  
  si32 i, icomplex, isimple, ientry;
  
  double factor;
  
  tdata_basic_header_t basic_header;
  tdata_basic_complex_params_t complex_params;
  tdata_basic_simple_params_t simple_params;
  tdata_basic_track_entry_t entry;
  
  /*
   * main header
   */
  
  if (read_from_buffer(sockfd, (char *) &basic_header,
		       (si32) sizeof(tdata_basic_header_t),
		       TDATA_BASIC_HEADER_ID)) {
    
    fprintf(stderr, "ERROR - read_basic_data\n");
    fprintf(stderr, "Packet out of order.\n");
    fprintf(stderr, "Expecting basic header packet.\n");
    exit(-1);
    
  }
  
  BE_to_array_32((ui32 *) &basic_header,
		 (ui32) sizeof(tdata_basic_header_t));
  
  factor = (double) basic_header.factor;
  
  printf("\nTDATA BASIC HEADER\n");
  
  printf("  Dbz threshold : %lg\n",
	 (double) basic_header.dbz_threshold / factor);
  printf("  Min storm size (km2 or km3) : %lg\n",
	 (double) basic_header.min_storm_size);
  printf("  Max tracking speed (km/hr) : %lg\n",
	 (double) basic_header.max_tracking_speed / factor);
  printf("  Max delta cube root (volume) : %lg\n",
	 (double) basic_header.max_delta_cube_root_volume / factor);
  printf("  Max delta time : %lg\n",
	 (double) basic_header.max_delta_time / factor);
  printf("  n_complex_tracks : %ld\n", basic_header.n_complex_tracks);
  printf("  time : %s\n", utimestr(udattim(basic_header.time)));
  printf("  data_start_time : %s\n",
	 utimestr(udattim(basic_header.data_start_time)));
  printf("  data_end_time : %s\n",
	 utimestr(udattim(basic_header.data_end_time)));
  
  /*
   * complex tracks
   */
  
  for (icomplex = 0;
       icomplex < basic_header.n_complex_tracks; icomplex++) {
    
    if (read_from_buffer(sockfd, (char *) &complex_params,
			 (si32) sizeof(tdata_basic_complex_params_t),
			 TDATA_BASIC_COMPLEX_PARAMS_ID)) {
      
      fprintf(stderr, "ERROR - read_basic_data\n");
      fprintf(stderr, "Packet out of order.\n");
      fprintf(stderr, "Expecting basic complex track params packet.\n");
      exit(-1);
      
    }
    
    BE_to_array_32((ui32 *) &complex_params,
		   (ui32) sizeof(tdata_basic_complex_params_t));
    
    printf("\n  BASIC COMPLEX TRACK PARAMS\n");
    
    printf("    complex_track_num : %ld\n",
	   complex_params.complex_track_num);
    printf("    n_simple_tracks : %ld\n",
	   complex_params.n_simple_tracks);
    
    if (complex_params.n_simple_tracks > 0) {
      printf("    simple_track_nums :");  
      for (i = 0; i < complex_params.n_simple_tracks; i++) {
	if (i == complex_params.n_simple_tracks - 1)
	  printf(" %ld\n", complex_params.simple_track_num[i]);
	else
	  printf(" %ld,", complex_params.simple_track_num[i]);
      }
    }
    
    printf("    start_time, end_time : %s, %s\n",
	   utimestr(udattim(complex_params.start_time)),
	   utimestr(udattim(complex_params.end_time)));
    
    /*
     * simple tracks
     */
    
    for (isimple = 0;
	 isimple < complex_params.n_simple_tracks; isimple++) {
      
      if (read_from_buffer(sockfd, (char *) &simple_params,
			   (si32) sizeof(tdata_basic_simple_params_t),
			   TDATA_BASIC_SIMPLE_PARAMS_ID)) {
	
	fprintf(stderr, "ERROR - read_basic_data\n");
	fprintf(stderr, "Packet out of order.\n");
	fprintf(stderr, "Expecting basic simple params packet.\n");
	exit(-1);
	
      }
      
      BE_to_array_32((ui32 *) &simple_params,
		     (ui32) sizeof(tdata_basic_simple_params_t));
      
      if (print_on) {
	
	printf("\n    SIMPLE TRACK PARAMS\n");
	
	printf("      complex/simple track nums : %ld/%ld\n",
	       simple_params.complex_track_num,
	       simple_params.simple_track_num);
	
	printf("      duration_in_scans : %ld\n",
	       simple_params.duration_in_scans);
	
	printf("      start_time, end_time : %s, %s\n",
	       utimestr(udattim(simple_params.start_time)),
	       utimestr(udattim(simple_params.end_time)));
	
	printf("      nparents, nchildren : %ld, %ld\n",
	       simple_params.nparents, simple_params.nchildren);
	
	if (simple_params.nparents > 0) {
	  printf("      parents :");  
	  for (i = 0; i < simple_params.nparents; i++) {
	    if (i == simple_params.nparents - 1)
	      printf(" %ld\n", simple_params.parent[i]);
	    else
	      printf(" %ld,", simple_params.parent[i]);
	  }
	}
	
	if (simple_params.nchildren > 0) {
	  printf("      children :");  
	  for (i = 0; i < simple_params.nchildren; i++) {
	    if (i == simple_params.nchildren -1)
	      printf(" %ld\n", simple_params.child[i]);
	    else
	      printf(" %ld,", simple_params.child[i]);
	  }
	}
	
      } /* if (print_on) */
      
      /*
       * track file entries
       */
      
      for (ientry = 0;
	   ientry < simple_params.duration_in_scans; ientry++) {
	
	if (read_from_buffer(sockfd, (char *) &entry,
			     (si32) sizeof(tdata_basic_track_entry_t),
			     TDATA_BASIC_TRACK_ENTRY_ID)) {
	  
	  fprintf(stderr, "ERROR - read_basic_data\n");
	  fprintf(stderr, "Packet out of order.\n");
	  fprintf(stderr, "Expecting basic track entry packet.\n");
	  exit(-1);
	  
	}
	
	BE_to_array_32((ui32 *) &entry,
		       (ui32) sizeof(tdata_basic_track_entry_t));
	
	if (print_on) {
	  
	  printf("\n      BASIC TRACK ENTRY\n");
	  printf("        entry number : %ld\n", ientry);
	  printf("        time : %s\n",
		 utimestr(udattim(entry.time)));
	  printf("        complex/simple track nums : %ld/%ld\n",
		 entry.complex_track_num, entry.simple_track_num);
	  
	  printf("        Datum latitude (deg) : %lg\n",
		 entry.datum_latitude);
	  printf("        Datum longitude (deg) : %lg\n",
		 entry.datum_longitude);
	  
	  printf("        Z centroid x,y,z (km) : %lg, %lg, %lg\n",
		 entry.refl_centroid_x,
		 entry.refl_centroid_y,
		 entry.refl_centroid_z);
	  
	  printf("        top (km) : %lg\n", entry.top);
	  printf("        base (km) : %lg\n", entry.base);
	  printf("        volume (km3) : %lg\n", entry.volume);
	  printf("        mean area (km2) : %lg\n", entry.area_mean);
	  printf("        precip flux (m3/s) : %lg\n",
		 entry.precip_flux);
	  printf("        mass (ktons) : %lg\n", entry.mass);
	  printf("        vil (kg/m2) : %lg\n", entry.vil_from_maxz);
	  
	  printf("        projected area (km2) : %lg\n",
		 entry.proj_area);
	  printf("        projected area centroid x (km) : %lg\n",
		 entry.proj_area_centroid_x);
	  printf("        projected area centroid y (km) : %lg\n",
		 entry.proj_area_centroid_y);
	  printf("        proj. area orientation (deg) : %lg\n",
		 entry.proj_area_orientation);
	  printf("        proj. area minor radius (km) : %lg\n",
		 entry.proj_area_minor_radius);
	  printf("        proj. area major radius (km) : %lg\n",
		 entry.proj_area_major_radius);
	  
	  printf("        proj. area cent dx/dt (km/hr) : %lg\n",
		 entry.proj_area_centroid_dx_dt);
	  printf("        proj. area cent dy/dt (km/hr) : %lg\n",
		 entry.proj_area_centroid_dy_dt);
	  printf("        dvolume/dt (km3/hr) : %lg\n",
		 entry.dvolume_dt);
	  printf("        dproj_area/dt (km3/hr) : %lg\n",
		 entry.dproj_area_dt);
	  
	} /* if (print_on) */
	
      } /* ientry */
      
    } /* isimple */
    
  } /* icomplex */
  
  return (0);
  
}

