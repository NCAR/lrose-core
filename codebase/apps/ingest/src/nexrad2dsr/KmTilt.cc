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
////////////////////////////////////////////////////////////////////////////////
//
//  Class for managing a tilt of 1km data indexed by azimuth
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: KmTilt.cc,v 1.11 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include "Driver.hh"
#include "KmTilt.hh"
using namespace std;

void
KmTilt::clearState()
{
   //
   // We are starting storage for a new tilt, reset the state
   //
   dataLen       = 0;
   dataIndex     = 0;
   dataAvailable = false;
   clearLut();
}

void
KmTilt::setData( float azimuth, ui08* dbzData, ui08* snrData )
{
   int azIndex;

   //
   // Store the data at the current data index
   // The length of the data is known from the prior startOfDbz() call
   //
   kmData[dataIndex].setData( azimuth, dbzData, snrData, dataLen );

   //
   // Remember the data index for this azimuth by storing the index
   // in the data lookup table using a synthetic azimuth index
   //
   azIndex = getAzIndex( azimuth );
   dataLut[azIndex] = dataIndex;

   //
   // Increment the data index for the next time around
   //
   dataIndex++;
   dataAvailable = true;

   //
   // Make sure we haven't exceeded the max number of azimuths
   // that we expect to handle.  NOTE: We should eventually improve
   // our dataLut logic adopted from the ridds2mom application to 
   // handle repeated tilts.
   //
   if ( dataIndex >= NUM_AZIMUTHS ) {
      POSTMSG( ERROR, "Maximum number of azimuths exceeded.\n"
                      "Check for repeated elevations in radar operations." );
      exit(-1);
   }
}

ui08*
KmTilt::getData( type_t type, float azimuth )
{
   int index;

   //
   // A completed lookup table should insure us that a data index
   // is always available
   //
   index = getDataIndex( azimuth );
   if ( index == UNAVAILABLE ){
     //
     // This should never happen. If it does, we cannot proceed.
     //
     POSTMSG( ERROR, "A data index is not available!");
     exit(-1);
   }

   if ( type == DBZ_DATA ) {
      return( kmData[index].getDbz() );
   }
   else {
      return( kmData[index].getSnr() );
   }
}

size_t
KmTilt::getAzIndex( float azimuth )
{
   //
   // The synthetic azimuth index includes the first decimal place 
   // of the actual azimuth which ranges from 0-360.  Thus a complete 
   // lookup table must have 360*10+1 indicies
   //
   size_t azIndex = (size_t)(azimuth * 10 + 0.5 );

   if ( azIndex >= NUM_INDICIES ) {
      POSTMSG( ERROR, "Azimuth %.1f outside of range [0-360]", azimuth );
      //
      // This should not happen - if it does, exit.
      //
      exit( -1 );
   }

   return( azIndex );
}

int
KmTilt::getDataIndex( float azimuth )
{
   size_t azIndex = getAzIndex( azimuth );
   return( dataLut[azIndex] );
}

void
KmTilt::clearLut()
{
   for( size_t i=0; i < NUM_INDICIES; i++ ) {
      dataLut[i] = UNAVAILABLE;
   }
}

void
KmTilt::completeLut()
{
   //
   // This method is an exact duplicate of what was done in 
   // ridds2mom/store_data.cc/complete_lookup_tbl()
   // NOTE: The int's should not be changed to size_t, 
   // some of the indicies need to be negative.
   //
  int             i, j, k=0;
  int             num_bins, first_bin_position = -1;
  int             bins_left;
  short           ndex_value1;
  short           ndex_value2;
  short           save_ndex_value;

  ndex_value1 = ndex_value2 = 0;

  /* locate the first index */
  for (i = 0; i < (int)NUM_INDICIES; i++) {
    if ((ndex_value1 = dataLut[i]) >= 0) {
      first_bin_position = i - 1;
      break;
    }
  }

  /* no index; the system was started after the reflectivity only tilt */
  if (i == (int)NUM_INDICIES)
    return;

  save_ndex_value = ndex_value1;

  k = first_bin_position + 2;
  for (i = k; i < (int)NUM_INDICIES; i++) {
    if ((ndex_value2 = dataLut[i]) >= 0) {
      num_bins = i - k;
      for (j = k; j < k + num_bins; j++) {
        if (j - k < num_bins / 2)
          dataLut[j] = ndex_value1;
        else
          dataLut[j] = ndex_value2;
      }

      ndex_value1 = ndex_value2;
      k = i + 1;
    }
  }

  /* fill in the front and back of the array */
  num_bins = NUM_INDICIES - k + first_bin_position;
  if (num_bins / 2 + k >= (int)NUM_INDICIES) {
    /* fill in the rest of the array with ndex_value1 */
    for (i = k; i < (int)NUM_INDICIES; i++) {
      dataLut[i] = ndex_value1;
    }
    /*
     * determine and complete the number of bins that should be
     * filled at the beginning of the array
     */
    bins_left = num_bins / 2 - (NUM_INDICIES - k);
    for (i = 0; i < bins_left; i++) {
      dataLut[i] = ndex_value1;
    }
    for (i = bins_left; i <= first_bin_position; i++) {
      dataLut[i] = save_ndex_value;
    }
  } else {
    for (i = k; i < num_bins / 2 + k; i++) {
      dataLut[i] = ndex_value1;
    }
    k += num_bins / 2;
    for (i = k; i < (int)NUM_INDICIES; i++) {
      dataLut[i] = save_ndex_value;
    }
    for (i = 0; i <= first_bin_position; i++) {
      dataLut[i] = save_ndex_value;
    }
  }
}
