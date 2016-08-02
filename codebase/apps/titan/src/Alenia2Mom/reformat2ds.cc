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
/***************************************************************************
 * reformat2ds.c
 *
 * Reformat the data stream into DIDSS FMQ
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include "Alenia2Mom.h"
using namespace std;

/**********************
 * file scope variables
 */

static int Debug;
static char *Radar_name;
static char *Site_name;
static si32 Latitude;
static si32 Longitude;
static si32 Altitude;
static si32 Polarization;
static si32 Beam_width;
static si32 Avg_xmit_pwr;
static si32 Wavelength;
static double Noise_dbz_at_100km;

/*********************************
 * initialize file scope variables
 */

void 
init_reformat2ds(char *radar_name,
		 char *site_name,
		 double latitude,
		 double longitude,
		 double altitude,
		 int polarization,
		 double beam_width,
		 double avg_xmit_pwr,
		 double wavelength,
		 double noise_dbz_at_100km,
		 int debug)
{

  Debug = debug;
  Radar_name = STRdup(radar_name);
  Site_name = STRdup(site_name);
  Latitude = (si32) floor(latitude * 100000.0 + 0.5);
  Longitude = (si32) floor(longitude * 100000.0 + 0.5);
  Altitude = (si32) floor(altitude * 1000.0 + 0.5);
  Polarization = polarization;
  Beam_width = (si32) floor(beam_width * 100.0 + 0.5);
  Avg_xmit_pwr = (si32) (avg_xmit_pwr + 0.5);
  Wavelength = (si32) (wavelength * 100.0 + 0.5);
  Noise_dbz_at_100km = noise_dbz_at_100km;

}

/*
 * reformat to DS_RADAR format
 */

int reformat2ds(ui08 *read_buffer, int nread,
		ui08 **write_buffer,
		int *nwrite)
     
{

  static ui08 *buffer_copy = NULL;
  static int n_copy_alloc = 0;

  /*
   * make copy of buffer
   */

  if (n_copy_alloc < nread) {
    if (buffer_copy == NULL) {
      buffer_copy = (ui08 *) umalloc(nread);
    } else {
      buffer_copy = (ui08 *) urealloc(buffer_copy, nread);
    }
    n_copy_alloc = nread;
  }
  memcpy(buffer_copy, read_buffer, nread);

  fprintf(stderr, "reformat2ds not yet implemented\n");
  return (-1);

}


