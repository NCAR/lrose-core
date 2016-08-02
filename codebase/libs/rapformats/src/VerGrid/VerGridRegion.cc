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
 * VerGridRegion.cc
 *
 * Verify Grid Region class
 *
 * RAP, NCAR, Boulder CO
 *
 * Mike Dixon
 *
 * June 1998
 *
 *********************************************************************/


#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <rapformats/VerGridRegion.hh>
using namespace std;

////////////////////////
// constructor
//
// Assume just 1 region

VerGridRegion::VerGridRegion()

{

  _chunkBuf = MEMbufCreate();
  
  nRegions = 1;

  int nbytes_needed =
    sizeof(VerGridRegionHdr_t) + nRegions * sizeof(VerGridRegionData_t);
  
  MEMbufPrepare(_chunkBuf, nbytes_needed);

  hdr = (VerGridRegionHdr_t *) MEMbufPtr(_chunkBuf);

  data = (VerGridRegionData_t *)
    ((char *) MEMbufPtr(_chunkBuf) + sizeof(VerGridRegionHdr_t));

}

/////////////
// destructor

VerGridRegion::~VerGridRegion()

{
   MEMbufDelete(_chunkBuf);
}

/////////////////
// Setting Header

void VerGridRegion::setHdr(int n_regions,
			   time_t forecast_time,
			   int forecast_lead_time,
			   float forecast_ht,
			   float forecast_level_lower,
			   float forecast_level_upper,
			   float truth_ht,
			   float truth_level_lower,
			   float truth_level_upper)

{

  nRegions = n_regions;
  
  int nbytes_needed =
    sizeof(VerGridRegionHdr_t) + nRegions * sizeof(VerGridRegionData_t);
  
  MEMbufPrepare(_chunkBuf, nbytes_needed);
  
  hdr = (VerGridRegionHdr_t *) MEMbufPtr(_chunkBuf);

  data = (VerGridRegionData_t *)
    ((char *) MEMbufPtr(_chunkBuf) + sizeof(VerGridRegionHdr_t));

  hdr->n_regions = n_regions;
  hdr->forecast_time = forecast_time;
  hdr->forecast_lead_time = forecast_lead_time;
  hdr->forecast_ht = forecast_ht;
  hdr->forecast_level_upper = forecast_level_upper;
  hdr->forecast_level_lower = forecast_level_lower;
  hdr->truth_ht = truth_ht;
  hdr->truth_level_upper = truth_level_upper;
  hdr->truth_level_lower = truth_level_lower;
  
}

///////////////
// setting data
//

void VerGridRegion::setData(int region_num,
			    char *name,
			    double latitude,
			    double longitude,
			    double radius,
			    double percent_covered_forecast,
			    double percent_covered_truth)

{

  VerGridRegionData_t *dat = data + region_num;
  STRncopy(dat->name, name, VERGRID_REGION_NAME_LEN);
  dat->latitude = latitude;
  dat->longitude = longitude;
  dat->radius = radius;
  dat->percent_covered_forecast = percent_covered_forecast;
  dat->percent_covered_truth = percent_covered_truth;

}
			    
//////////////////
// read from chunk

void VerGridRegion::readChunk(void *chunk, int len)

{

  MEMbufReset(_chunkBuf);

  MEMbufAdd(_chunkBuf, chunk, len);

  hdr = (VerGridRegionHdr_t *) MEMbufPtr(_chunkBuf);

  data = (VerGridRegionData_t *)
    ((char *) MEMbufPtr(_chunkBuf) + sizeof(VerGridRegionHdr_t));

  hdr_from_BE();

  nRegions = hdr->n_regions;

  data_from_BE();
  
}

///////////////
// Swapping
//

void VerGridRegion::to_BE()
{
  hdr_to_BE();
  data_to_BE();
}

void VerGridRegion::hdr_to_BE()

{
  BE_from_array_32(hdr, sizeof(VerGridRegionHdr_t));
}

void VerGridRegion::data_to_BE()

{
  for (int i = 0; i < nRegions; i++) {
    BE_from_array_32(data + i,
		     sizeof(VerGridRegionData_t) - VERGRID_REGION_NAME_LEN);
  }
}

void VerGridRegion::from_BE()
{
  hdr_from_BE();
  data_from_BE();
}

void VerGridRegion::hdr_from_BE()

{
  BE_to_array_32(hdr, sizeof(VerGridRegionHdr_t));
}

void VerGridRegion::data_from_BE()

{
  for (int i = 0; i < nRegions; i++) {
    BE_to_array_32(data + i,
		   sizeof(VerGridRegionData_t) - VERGRID_REGION_NAME_LEN);
  }
}

////////
// print
//

void VerGridRegion::print(FILE *out, const char *spacer) const

{

  fprintf(out, "\n");
  fprintf(out, "%sVerify Grid region data\n", spacer);
  fprintf(out, "%s-----------------------\n", spacer);
  
  // header

  fprintf(out, "%s  n_regions: %d\n", spacer, hdr->n_regions);

  fprintf(out, "%s  forecast_time: %s\n", spacer,
	  utimstr(hdr->forecast_time));
  fprintf(out, "%s  forecast_lead_time: %d\n", spacer,
	  hdr->forecast_lead_time);

  fprintf(out, "%s  forecast_ht: %g\n", spacer, hdr->forecast_ht);
  fprintf(out, "%s  forecast_level_upper: %g\n", spacer,
	  hdr->forecast_level_upper);
  fprintf(out, "%s  forecast_level_lower: %g\n", spacer,
	  hdr->forecast_level_lower);

  fprintf(out, "%s  truth_ht: %g\n", spacer, hdr->truth_ht);
  fprintf(out, "%s  truth_level_upper: %g\n", spacer,
	  hdr->truth_level_upper);
  fprintf(out, "%s  truth_level_lower: %g\n", spacer,
	  hdr->truth_level_lower);
  fprintf(out, "\n");

  for (int i = 0; i < nRegions; i++) {

    VerGridRegionData_t *dat = data + i;

    fprintf(out, "%s  Region %s\n", spacer, dat->name);
    fprintf(out, "\n");
    fprintf(out, "%s    latitude: %g\n", spacer, dat->latitude);
    fprintf(out, "%s    longitude: %g\n", spacer, dat->longitude);
    fprintf(out, "%s    radius: %g\n", spacer, dat->radius);
    fprintf(out, "%s    percent_covered_forecast: %g\n", spacer,
	    dat->percent_covered_forecast);
    fprintf(out, "%s    percent_covered_truth: %g\n", spacer,
	    dat->percent_covered_truth);
    fprintf(out, "\n");

  } // i

}

