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
 * WayPoint.cc: Class representing a flight route as stored in an SPDB
 *              database.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <string>
#include <ctime>

#include <dataport/bigend.h>
#include <Spdb/WayPoint.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
using namespace std;


// Global variables

const double WayPoint::BAD_POSITION = -999.0;
const time_t WayPoint::BAD_ETA = 0;

/*********************************************************************
 * Constructors
 */

WayPoint::WayPoint(const string& label,
		   const time_t eta,
		   const double lat,
		   const double lon,
		   const bool debug_flag) :
  _debugFlag(debug_flag),
  _label(label),
  _eta(eta),
  _lat(lat),
  _lon(lon)
{
  // Do nothing
}


WayPoint::WayPoint(const string& label,
		   const DateTime& eta,
		   const double lat,
		   const double lon,
		   const bool debug_flag) :
  _debugFlag(debug_flag),
  _label(label),
  _eta(eta),
  _lat(lat),
  _lon(lon)
{
  // Do nothing
}


WayPoint::WayPoint(const void *spdb_buffer,
		   const bool debug_flag) :
  _debugFlag(debug_flag)
{
  // Retrieve the way point information from the SPDB buffer

  spdb_way_pt_t way_pt = *(spdb_way_pt_t *)spdb_buffer;
  spdbToNative(&way_pt);
  
  _label = way_pt.label;
  
  time_t eta = way_pt.eta;
  _eta.set(eta);
  
  _lat = way_pt.lat;
  _lon = way_pt.lon;
}


/*********************************************************************
 * Destructor
 */

WayPoint::~WayPoint()
{
  // Do nothing.
}


/*********************************************************************
 * print() - Print the way point to the given stream.
 */

void WayPoint::print(FILE *stream) const
{
  fprintf(stream, "Way Point:\n");
  fprintf(stream, "   label = <%s>\n", _label.c_str());
  fprintf(stream, "   eta = %s\n", _eta.dtime());
  fprintf(stream, "   lat = %f\n", _lat);
  fprintf(stream, "   lon = %f\n", _lon);
  
}


/*********************************************************************
 * writeSpdb() - Write the way point to the given byte buffer in SPDB
 *               format.  Note that the data will be written to the
 *               buffer in big-endian format.
 */

void WayPoint::writeSpdb(void *buffer) const
{
  spdb_way_pt_t *way_pt = (spdb_way_pt_t *)buffer;
  
  STRcopy(way_pt->label, _label.c_str(), MAX_WAY_PT_LABEL_LEN);
  way_pt->eta = _eta.utime();
  way_pt->spare = 0;
  way_pt->lat = _lat;
  way_pt->lon = _lon;
  
  spdbToBigend(way_pt);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * spdbToBigend() - Swaps the spdb_way_pt_t structure from native
 *                  format to big-endian format.
 */

void WayPoint::spdbToBigend(spdb_way_pt_t *way_pt)
{
  BE_from_array_32((char *)way_pt + MAX_WAY_PT_LABEL_LEN,
		   sizeof(spdb_way_pt_t) - MAX_WAY_PT_LABEL_LEN);
}


/*********************************************************************
 * spdbToNative() - Swaps the spdb_way_pt_t structure from big-endian
 *                  format to native format.
 */

void WayPoint::spdbToNative(spdb_way_pt_t *way_pt)
{
  BE_to_array_32((char *)way_pt + MAX_WAY_PT_LABEL_LEN,
		 sizeof(spdb_way_pt_t) - MAX_WAY_PT_LABEL_LEN);
}
