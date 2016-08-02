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
 * store_data.c
 *
 * Reformat RIDDS radar beam to LL moments data.
 *
 * Mike Dixon RAP NCAR Boulder CO USA.
 *
 * May 1997
 *
 **************************************************************************/

#include "ridds2mom.h"
#include <rapformats/swap.h>
using namespace std;

/**********************
 * file scope variables
 */                            

static double Noise_dbz_at_100km;

/************************************************************************/

/*
 * file scope prototypes
 */

static int store_nexrad_data(RIDDS_data_hdr * input_data,
                             ui08 *output_data,
                             unsigned short *azimuth,
                             int new_vol);

static int store_constant_data(RIDDS_data_hdr * input_data,
                               ui08 *output_data,
                               unsigned short *azimuth,
                               int new_vol);

static void init_snr_lut(double gate_size, short *noise);

static void check_lut(short *azi_lut);

static void init_azi_lut(short *azi_lut);

static void insert_lookup_tbl(short ndex, short rounded_azi,
                              short azi_lut[]);

static void complete_lookup_tbl(short *azi_lut);

/************************************************************************/

void
init_moments( double noise_dbz )
{
  Noise_dbz_at_100km = noise_dbz;
}

int
store_data(RIDDS_data_hdr * input_data,
           ui08 *output_data,
           unsigned short *azimuth,
           int new_vol)
{

   if ( Glob->params.input_stream_format == NEXRAD_FORMAT ) {
      return store_nexrad_data( input_data, output_data, azimuth, new_vol );
   }
   else {
      return store_constant_data( input_data, output_data, azimuth, new_vol );
   }
}

int
store_nexrad_data(RIDDS_data_hdr * input_data,
	          ui08 *output_data,
                  unsigned short *azimuth,
                  int new_vol)
{
  char           *beam_ptr = (char *) input_data;
  char           *vel_ptr = (beam_ptr + input_data->vel_data_playback);
  char           *ref_ptr = (beam_ptr + input_data->ref_data_playback);
  char           *sw_ptr = (beam_ptr + input_data->sw_data_playback);
  int             gate_total = 0;
  Reformat_data  *moments;
  Reformat_data  *moments_local;
  Reformat_data  *moments_out;

  static int      first_beam_processed = TRUE;
  static short    noise_dbz[MAX_NUM_GATES];
  static Low_prf_data save_ref[MAX_AZI];
  static short    azi_lut[MAX_AZI_PER_TILT];

  int             dup_counter = 0;
  int             i, j;
  int             gt;
  double          gate_width;
  int             reflectivity_only = FALSE;
  int             velocity_only = FALSE;
  int             first_azi_in_vel_tilt = FALSE;
  static short    ndex = -1;
  static short    tilt_number = FIRST_TIME;
  short           rounded_azi;

  /*
   *  initialize only on start of program
   */

  if (first_beam_processed) {
    gate_width = (double) input_data->vel_gate_width / 1000;
    init_snr_lut(gate_width, noise_dbz);
    init_azi_lut(azi_lut);
    for (i = 0; i < MAX_AZI; i++) {
      save_ref[i].azimuth = 0;
    }
    first_beam_processed = FALSE;
  }

  if (tilt_number != input_data->elev_num || new_vol) {
    ndex = -1;
    tilt_number = input_data->elev_num;
  }

  /*
   * determine if we are receiving only reflectivity or only velocity
   * info
   */

  if (vel_ptr == beam_ptr) {
    if (ndex == -1)
      init_azi_lut(azi_lut);
    reflectivity_only = TRUE;
  } else if (ref_ptr == (char *) input_data) {
    if (ndex == -1)
      first_azi_in_vel_tilt = TRUE;
    velocity_only = TRUE;
  }

  /*
   * determine the number of gates
   */

  if (reflectivity_only) {
    gate_total = input_data->ref_num_gates;
    ndex++;
  } else {
    gate_total = input_data->vel_num_gates;
  }

  /*
   * alloc local moments array
   */
  
  moments_local =
    (Reformat_data *) umalloc(gate_total * sizeof(Reformat_data));
  
  /*
   * loop through gates
   */
  
  moments = moments_local;
  for (gt = 0; gt < gate_total; gt++, moments++) {
    
    if (reflectivity_only) {

      // this is a low prf tilt - 1000m gates

      moments->vel = 0;
      moments->width = 0;

      if (*ref_ptr == 0 || *ref_ptr == 1) {
	// 0 - below SNR)
	// 1 - range ambiguous
	moments->dbz = 0;
	moments->snr = 0;
      } else {
	moments->dbz = *ref_ptr - 2;
	moments->snr = moments->dbz - noise_dbz[gt];
      }

      if (gt == 0) {

	// round the azimuth to 1 decimal place

	rounded_azi = (short) (((float) *azimuth + 5) / 10);
	if (rounded_azi < MAX_AZI_PER_TILT) {
	  insert_lookup_tbl(ndex, rounded_azi, azi_lut);
	} else {
	  ufree(moments);
	  return (-1);
	}
      }

      /*
       * save the reflectivity data for combining with the
       * next tilt
       */

      for (j = 0; j < Glob->gate_ratio; j++) {
	save_ref[ndex].ref[gt * 4 + j] = moments->dbz;
	if (gt == 0)
	  save_ref[ndex].azimuth = *azimuth;
      }

    } else if (velocity_only) {

      if (first_azi_in_vel_tilt) {
	complete_lookup_tbl(azi_lut);
	check_lut(azi_lut);
	first_azi_in_vel_tilt = FALSE;
      }

      /*
       * combine with the previously saved low prf
       * reflectivity data
       */

      if (*vel_ptr == 0 || *vel_ptr == 1) {
	// 0 - below SNR)
	// 1 - range ambiguous
	moments->vel = 0;
      } else {
	moments->vel = *vel_ptr - 2;
      }

      rounded_azi = (short) (((float) *azimuth + 5) / 10);
      if (rounded_azi < MAX_AZI_PER_TILT) {
	ndex = azi_lut[rounded_azi];
      } else {
	ufree(moments);
	return (-1);
      }

      if (ndex >= 0) {
	moments->dbz = save_ref[ndex].ref[gt];
	*azimuth = save_ref[ndex].azimuth;
      } else {
	moments->dbz = 0;
      }

      if (*sw_ptr == 0 || *sw_ptr == 1) {
	moments->width = 0;
      } else
	moments->width = *sw_ptr - 2;

      moments->snr = 0;

    } else {

      if (*vel_ptr == 0 || *vel_ptr == 1) {
	moments->vel = 0;
      } else {
	moments->vel = *vel_ptr - 2;
      }

      if (*ref_ptr == 0 || *ref_ptr == 1) {
	moments->dbz = 0;
	moments->snr = 0;
      } else {
	moments->dbz = *ref_ptr - 2;
	moments->snr = moments->dbz - noise_dbz[gt];
      }

      if (*sw_ptr == 0 || *sw_ptr == 1) {
	moments->width = 0;
      } else {
	moments->width = *sw_ptr - 2;
      }

    }

    /*
     * since the resolution of the reflectivity field is less
     * than the velocity field duplicate data so gate spacing is
     * identical
     */

    if (!reflectivity_only) {
      if (++dup_counter >= Glob->gate_ratio) {
	dup_counter = 0;
      }
      if (dup_counter == 0) {
	ref_ptr++;
      }
      vel_ptr++;
      sw_ptr++;
    } else {
      ref_ptr++;
    }

  } // gt

  // copy moments data into output data array. We skip 1km of
  // data because this has negative range which RDI does not like.

  moments_out = (Reformat_data  *) output_data;
  int nskip;

  if (reflectivity_only) {
    nskip = 1;
  } else {
    nskip = Glob->gate_ratio;
  }

  DsRadarParams &radarParams = Glob->radarMsg->getRadarParams();
  int ngates_good = gate_total - nskip;

  if (radarParams.numFields == 4) {

    // all fields

    memset(moments_out, 0, nskip * sizeof(Reformat_data));
    memcpy(moments_out + nskip,  moments_local + nskip,
	   ngates_good * sizeof(Reformat_data));

  } else {

    // fields are specified
    
    memset(moments_out, 0, nskip * radarParams.numFields * sizeof(ui08));
    ui08 *outpos = output_data;

    if (Glob->params.output_ds_snr) {
      ui08 *ml = &moments_local[nskip].snr;
      ui08 *mo = outpos++;
      for (int i = 0; i < ngates_good;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

    if (Glob->params.output_ds_dbz) {
      ui08 *ml = &moments_local[nskip].dbz;
      ui08 *mo = outpos++;
      for (int i = 0; i < ngates_good;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

    if (Glob->params.output_ds_vel) {
      ui08 *ml = &moments_local[nskip].vel;
      ui08 *mo = outpos++;
      for (int i = 0; i < ngates_good;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

    if (Glob->params.output_ds_spw) {
      ui08 *ml = &moments_local[nskip].width;
      ui08 *mo = outpos++;
      for (int i = 0; i < ngates_good;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

  }

  ufree(moments_local);

  return (0);

}

int
store_constant_data(RIDDS_data_hdr * input_data,
	            ui08 *output_data,
                    unsigned short *azimuth,
                    int new_vol)
{
  char           *beam_ptr = (char *) input_data;
  char           *vel_ptr = (beam_ptr + input_data->vel_data_playback);
  char           *ref_ptr = (beam_ptr + input_data->ref_data_playback);
  char           *sw_ptr = (beam_ptr + input_data->sw_data_playback);
  int             gate_total = 0;
  Reformat_data  *moments;
  Reformat_data  *moments_local;
  Reformat_data  *moments_out;

  static int      first_beam_processed = TRUE;
  static short    noise_dbz[MAX_NUM_GATES];

  int             gt;
  double          gate_width;
  static short    tilt_number = FIRST_TIME;

  /*
   *  initialize only on start of program
   */

  if (first_beam_processed) {
    gate_width = (double) input_data->vel_gate_width / 1000;
    init_snr_lut(gate_width, noise_dbz);
    first_beam_processed = FALSE;
  }

  /*
   * initialize the azimuth lookup on start of each tilt
   */

  if (tilt_number != input_data->elev_num || new_vol) {
    tilt_number = input_data->elev_num;
  }

  /*
   * determine the number of gates
   */

  gate_total = input_data->ref_num_gates;

  /*
   * alloc local moments array
   */
  
  moments_local =
    (Reformat_data *) umalloc(gate_total * sizeof(Reformat_data));
  
  /*
   * loop through gates
   */
  
  moments = moments_local;
  for (gt = 0; gt < gate_total; gt++, moments++) {
    
      //
      // Set the reflectivity and signal-to-noise values
      //
      if (*ref_ptr == 0 || *ref_ptr == 1) {
	// 0 - below SNR)
	// 1 - range ambiguous
	moments->dbz = 0;
	moments->snr = 0;
      } else {
	moments->dbz = *ref_ptr - 2;
	moments->snr = moments->dbz - noise_dbz[gt];
      }

      //
      // Set the velocity value
      //
      if (*vel_ptr == 0 || *vel_ptr == 1) {
	// 0 - below SNR)
	// 1 - range ambiguous
	moments->vel = 0;
      } else {
	moments->vel = *vel_ptr - 2;
      }

      //
      // Spectral width -- not available
      //
      if (*sw_ptr == 0 || *sw_ptr == 1) {
	moments->width = 0;
      } else
	moments->width = *sw_ptr - 2;

      ref_ptr++;
      vel_ptr++;
      sw_ptr++;

  } // gt

  //
  // copy moments data into output data array
  //


  moments_out = (Reformat_data  *) output_data;
  DsRadarParams &radarParams = Glob->radarMsg->getRadarParams();

  if (radarParams.numFields == 4) {

    // all fields
    
    memcpy(moments_out, moments_local, gate_total * sizeof(Reformat_data));

  } else {

    // fields are specified
    
    ui08 *outpos = output_data;
    
    if (Glob->params.output_ds_snr) {
      ui08 *ml = &moments_local[0].snr;
      ui08 *mo = outpos++;
      for (int i = 0; i < gate_total;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

    if (Glob->params.output_ds_dbz) {
      ui08 *ml = &moments_local[0].dbz;
      ui08 *mo = outpos++;
      for (int i = 0; i < gate_total;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

    if (Glob->params.output_ds_vel) {
      ui08 *ml = &moments_local[0].vel;
      ui08 *mo = outpos++;
      for (int i = 0; i < gate_total;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

    if (Glob->params.output_ds_spw) {
      ui08 *ml = &moments_local[0].width;
      ui08 *mo = outpos++;
      for (int i = 0; i < gate_total;
	   i++, ml += 4, mo += radarParams.numFields) {
	*mo = *ml;
      }
    }

  }

  ufree(moments_local);

  return (0);
}

/*-----------------------------------------------------------*/

static void
init_snr_lut(double gate_size, short *noise)
{
  int             gate_num;
  double          noise_dbz;                      
  double          log_of_100 = log10(100.);    
  double          radial_dist;                              

  for (gate_num = 0; gate_num < MAX_NUM_GATES; gate_num++) {
    radial_dist = (gate_num + 1) * gate_size;
    noise_dbz = Noise_dbz_at_100km + 20.
      * (log10(radial_dist) - log_of_100);

    if (0)                                      
      fprintf(stderr, "r = %f - noise = %f\n", radial_dist, noise_dbz);

    noise[gate_num] = (short) ((noise_dbz * 100) / DBZ_SCALE);

  }
  if (0)
    for (gate_num = 0; gate_num < MAX_NUM_GATES; gate_num++) {
      fprintf(stderr, "gate num %d - noise X 100 = %d\n",
              gate_num, noise[gate_num]);
    }
}

/*-----------------------------------------------------------*/

static void
check_lut(short *azi_lut)
{
  int             i;
  int             count = 0;

  for (i = 0; i < MAX_AZI_PER_TILT; i++) {
    if (azi_lut[i] == -1)
      count++;
  }
}
/*-----------------------------------------------------------*/

static void
init_azi_lut(short *azi_lut)
{
  int             i;

  for (i = 0; i < MAX_AZI_PER_TILT; i++) {
    azi_lut[i] = -1;
  }
}

/*-----------------------------------------------------------*/

static void
insert_lookup_tbl(short ndex, short rounded_azi, short azi_lut[])
{
  azi_lut[rounded_azi] = ndex;
}

/*-----------------------------------------------------------*/

static void
complete_lookup_tbl(short *azi_lut)
{
  int             j;
  int             i;
  int             k = 0;
  int             first_bin_position, num_bins;
  short           ndex_value1;
  short           ndex_value2;
  short           save_ndex_value;
  short           bins_left;

  ndex_value1 = ndex_value2 = 0;

  /* locate the first index */
  for (i = 0; i < MAX_AZI_PER_TILT; i++) {
    if ((ndex_value1 = azi_lut[i]) >= 0) {
      first_bin_position = i - 1;
      break;
    }
  }

  /* no index; the system was started after the reflectivity only tilt */
  if (i == MAX_AZI_PER_TILT)
    return;

  save_ndex_value = ndex_value1;

  k = first_bin_position + 2;
  for (i = k; i < MAX_AZI_PER_TILT; i++) {
    if ((ndex_value2 = azi_lut[i]) >= 0) {
      num_bins = i - k;
      for (j = k; j < k + num_bins; j++) {
        if (j - k < num_bins / 2)
          azi_lut[j] = ndex_value1;
        else
          azi_lut[j] = ndex_value2;
      }

      ndex_value1 = ndex_value2;
      k = i + 1;
    }
  }

  /* fill in the front and back of the array */
  num_bins = MAX_AZI_PER_TILT - k + first_bin_position;
  if (num_bins / 2 + k >= MAX_AZI_PER_TILT) {
    /* fill in the rest of the array with ndex_value1 */
    for (i = k; i < MAX_AZI_PER_TILT; i++) {
      azi_lut[i] = ndex_value1;
    }
    /*
     * determine and complete the number of bins that should be
     * filled at the beginning of the array
     */
    bins_left = num_bins / 2 - (MAX_AZI_PER_TILT - k);
    for (i = 0; i < bins_left; i++) {
      azi_lut[i] = ndex_value1;
    }
    for (i = bins_left; i <= first_bin_position; i++) {
      azi_lut[i] = save_ndex_value;
    }
  } else {
    for (i = k; i < num_bins / 2 + k; i++) {
      azi_lut[i] = ndex_value1;
    }
    k += num_bins / 2;
    for (i = k; i < MAX_AZI_PER_TILT; i++) {
      azi_lut[i] = save_ndex_value;
    }
    for (i = 0; i <= first_bin_position; i++) {
      azi_lut[i] = save_ndex_value;
    }
  }
}
