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
/*******************************************************************
 * rescale.c
 *
 * Field rescaling
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * July 1997.
 ********************************************************************/

#include "Alenia2Mom.h"
using namespace std;

/*
 * Look up Table
 */

static ui08 Lut[3][256];
static int Z_range;

void rescale_dbz_init(void)

{

  ui08 *l;

  int i, z_range;
  int final_byte;

  double orig_scale = ALENIA_DBZ_SCALE;
  double final_scale = Glob->params.final_dbz_scale;
  double final_bias = Glob->params.final_dbz_bias;

  double orig_bias;
  double dbz;

  for (z_range = 0; z_range < 3; z_range++) {

    if (z_range == ALENIA_Z_RANGE_MED) {
      orig_bias = ALENIA_DBZ_BIAS_MED;
    } else if (z_range == ALENIA_Z_RANGE_HIGH) {
      orig_bias = ALENIA_DBZ_BIAS_HIGH;
    } else {
      orig_bias = ALENIA_DBZ_BIAS_LOW;
    }

    l = Lut[z_range];

    /*
     * clear array
     */
    
    memset(l, 0, 256);

    /*
     * load array
     */

    for (i = 2; i < 256; i++) {

      dbz = orig_bias + (double) i * orig_scale;

      final_byte = (int) (((dbz - final_bias) / final_scale) + 0.5);
      if (final_byte < 0) {
	final_byte = 0;
      }
      if (final_byte > 255) {
	final_byte = 255;
      }

      l[i] = (ui08) final_byte;

    }

  } /* z_range */

  return;

}

/***************
 * set_z_range()
 *
 * Set the Z_range to be used for next rescaling
 */

void rescale_set_z_range(int z_range) {
  
  Z_range = z_range;

}

/***************
 * rescale_dbz()
 *
 * Rescale the dBZ array to the final scale and bias
 */

void rescale_dbz(ui08 *dbz_bytes, int ngates)

{
  
  ui08 *l, *d;
  int i;

  l = Lut[Z_range];
  d = dbz_bytes;

  for (i = 0; i < ngates; i++, d++) {
    *d = l[*d];
  }

}

