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
/**********************************************************************
 * VerGridRegion.hh
 *
 * Verify grid data handling.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * June 1998
 *
 **********************************************************************/

#ifndef VerGridRegion_h
#define VerGridRegion_h

#define VERGRID_REGION_NAME_LEN 16


#include <dataport/port_types.h>
#include <toolsa/membuf.h>
using namespace std;

/*
 * VerGrid region header struct
 */

typedef struct {

  si32 n_regions;
  si32 forecast_time;        // time at which forecast is made - unix time
  si32 forecast_lead_time;   // secs
  si32 spare_int[3];

  fl32 forecast_ht;          // km, -1 for composite
  fl32 forecast_level_lower; // lower limit for forecast data
  fl32 forecast_level_upper; // upper limit for forecast data
  fl32 truth_ht;             // km, -1 for composite
  fl32 truth_level_lower;    // lower limit for truth data
  fl32 truth_level_upper;    // upper limit for truth data
  fl32 spare_float[4];
  
} VerGridRegionHdr_t;

/*
 * VerGrid region data struct
 */

typedef struct {

  fl32 latitude;
  fl32 longitude;
  fl32 radius;
  fl32 percent_covered_forecast;
  fl32 percent_covered_truth;
  fl32 spare[3];
  char name[VERGRID_REGION_NAME_LEN];

} VerGridRegionData_t;

class VerGridRegion

{
  
public:

  VerGridRegion();

  /////////////
  // destructor

  virtual ~VerGridRegion();
  
  /////////////////
  // Setting Header

  void setHdr(int n_regions,
	      time_t forecast_time,
	      int forecast_lead_time,
	      float forecast_ht,
	      float forecast_level_lower,
	      float forecast_level_upper,
	      float truth_ht,
	      float truth_level_lower,
	      float truth_level_upper);

  ///////////////
  // setting data
  //
  
  void setData(int region_num,
	       char *name,
	       double latitude,
	       double longitude,
	       double radius,
	       double percent_covered_forecast,
	       double percent_covered_truth);

  //////////////////
  // read from chunk
  
  void readChunk(void *chunk, int len);

  ///////////////
  // Swapping
  //

  void to_BE();
  void hdr_to_BE();
  void data_to_BE();
  void from_BE();
  void hdr_from_BE();
  void data_from_BE();

  ////////
  // print
  //

  void print(FILE *out, const char *spacer) const;

  ////////////////////
  // number of regions
  //

  int nRegions;

  ///////////////////////////
  // header and data location

  VerGridRegionHdr_t *hdr;   // pointer to header in buffer
  VerGridRegionData_t *data; // pointer to data array in buffer

  //////////////////
  // buffer location

  void *chunkPtr() { return (MEMbufPtr(_chunkBuf)); }
  int chunkLen() { return (MEMbufLen(_chunkBuf)); }

protected:

private:

  MEMbuf *_chunkBuf; // chunk buffer

};
  
#endif

