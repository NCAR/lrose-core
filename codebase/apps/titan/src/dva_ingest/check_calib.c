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
 * check_calib.c
 *
 * Check the calibration against the values read in from the cal file.
 * If they are different, (a) write out new cal file and (b) rerun the
 * lookup table generation and restart dva_cart.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1998
 *
 * returns 0 on success, -1 on failure.
 *
 ************************************************************************/

#include "dva_ingest.h"

static int cal_changed(dva_rdas_cal_t *latest,
		       dva_rdas_cal_t *current);

int check_calib(bprp_beam_t *beam, dva_rdas_cal_t *cal)

{

  
  int bin_width, skip;
  int mus;
  si16 stmp;

  double slope, viplo, plo, viphi, phi;
  double dbzhi, dbzlo;
  double radar_constant;
  double peak_power;
  double atten_per_km;
  double gate_spacing;
  double start_range;
  double range_100_nm;
  double range_corr_100;

  dva_rdas_cal_t latest_cal;

  PMU_auto_register("In check_calib");

  mus = beam->hdr.noise;
  skip = ((beam->hdr.site_blk) & 0x0ff);
  bin_width = ((beam->hdr.site_blk)/256) & 0x0f;
  gate_spacing = (double) bin_width * KM_PER_US;
  start_range = (double) skip * KM_PER_US + gate_spacing / 2.0;
  viplo = beam->hdr.viplo;
  viphi = beam->hdr.viphi;
  memcpy(&stmp, &beam->hdr.plo, sizeof(si16));
  plo = (stmp >> 5);
  memcpy(&stmp, &beam->hdr.phi, sizeof(si16));
  phi = (stmp >> 5);
  
  slope = (phi-plo)/ (viphi-viplo);
  peak_power = beam->hdr.xmt >> 5;

  atten_per_km = Glob->params.atmos_attenuation;
  radar_constant = Glob->params.radar_constant;
  
  range_100_nm = 53.9593; /* nm */
  
  range_corr_100 = 20.0 * log10(range_100_nm) +
    (100.0 * atten_per_km) - peak_power - radar_constant;

  dbzhi = phi + range_corr_100;
  /* round to 2 decimals */
  dbzhi = (double) ((int) (dbzhi * 100.0 + 0.5)) / 100.0;
  dbzlo = plo + range_corr_100;
  /* round to 2 decimals */
  dbzlo = (double) ((int) (dbzlo * 100.0 + 0.5)) / 100.0;
  
  if (Glob->params.debug >= DEBUG_VERBOSE) {

    fprintf(stderr, "gate_spacing, start_range: %g, %g\n",
	    gate_spacing, start_range);
    
    fprintf(stderr, "viphi, viplo: %g, %g\n", viphi, viplo);
    fprintf(stderr, "phi, plo: %g, %g\n", phi, plo);
    fprintf(stderr, "hdr.plo, hdr.phi: %d, %d\n",
	    beam->hdr.plo, beam->hdr.phi);
    fprintf(stderr, "dbzhi, dbzlo: %g, %g\n", dbzhi, dbzlo);
    fprintf(stderr, "slope: %g\n", slope);
    
    fprintf(stderr, "peak_power, atten_per_km: %g, %g\n",
	    peak_power, atten_per_km);

    fprintf(stderr, "radar_constant: %g\n", radar_constant);
    fprintf(stderr, "mus: %d\n", mus);

  }

  latest_cal.viphi = viphi;
  latest_cal.viplo = viplo;
  latest_cal.dbzhi = dbzhi;
  latest_cal.dbzlo = dbzlo;
  latest_cal.mus = mus;
  latest_cal.start_range = start_range;
  latest_cal.gate_spacing = gate_spacing;
  latest_cal.ngates = BPRP_GATES_PER_BEAM;

  if (cal_changed(cal, &latest_cal)) {

    fprintf(stderr, "NOTE: radar cal has changed\n");
    
    fprintf(stderr, "memcmp ret val: %d\n",
	    memcmp(cal, &latest_cal, sizeof(dva_rdas_cal_t)));
    
    fprintf(stderr, "prev_cal:\n");
    fprintf(stderr, "---> %g %g %g %g %d %g %g %d\n",
	    cal->viphi, cal->dbzhi,
	    cal->viplo, cal->dbzlo,
	    cal->mus, cal->gate_spacing,
	    cal->start_range, cal->ngates);

    fprintf(stderr, "latest_cal:\n");
    fprintf(stderr, "---> %g %g %g %g %d %g %g %d\n",
	    latest_cal.viphi, latest_cal.dbzhi,
	    latest_cal.viplo, latest_cal.dbzlo,
	    latest_cal.mus, latest_cal.gate_spacing,
	    latest_cal.start_range, latest_cal.ngates);

    /*
     * write out the new cal
     */

    fprintf(stderr, "Writing out new calibration.\n");

    *cal = latest_cal;
    if (write_rdas_cal(cal)) {
      tidy_and_exit(-1);
    }
    
    sleep(1);

    fprintf(stderr, "Recomputing lookup tables\n");
    fprintf(stderr, "Running command '%s'\n",
	    Glob->params.calibration_change_command_line);
    
    system(Glob->params.calibration_change_command_line);


  }
  
  return (0);

}

/**********************************************
 * cal_changed()
 *
 * Returns TRUE if cal has changed, FALSE if not
 */

static int cal_changed(dva_rdas_cal_t *latest,
		       dva_rdas_cal_t *current)

{

  if (fabs(latest->viphi - current->viphi) > 0.001) {
    return (TRUE);
  }

  if (fabs(latest->viplo - current->viplo) > 0.001) {
    return (TRUE);
  }

  if (fabs(latest->dbzhi - current->dbzhi) > 0.001) {
    return (TRUE);
  }

  if (fabs(latest->dbzlo - current->dbzlo) > 0.001) {
    return (TRUE);
  }

  if (latest->mus - current->mus != 0) {
    return (TRUE);
  }
  if (fabs(latest->gate_spacing - current->gate_spacing) > 0.001) {
    return (TRUE);
  }

  if (fabs(latest->start_range - current->start_range) > 0.001) {
    return (TRUE);
  }

  if (latest->ngates - current->ngates != 0) {
    return (TRUE);
  }

  return (FALSE);

}
		       
