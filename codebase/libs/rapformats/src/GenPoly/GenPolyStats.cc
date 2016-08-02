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
/////////////////////////////////////////////////////////////
// GenPolyStats.cc
//
// A GenPoly database made specifically for storing statistics
// for polygons at different elevation angles in radar data.
//
// Nancy Rehak, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2009
//////////////////////////////////////////////////////////////


#include <rapformats/GenPolyStats.hh>

using namespace std;

// constants

const string GenPolyStats::DROPSIZE_THRESH_FIELD_PREFIX = "dropsize thresh ";
const string GenPolyStats::THRESHOLD_FIELD_PREFIX = "threshold ";

const string GenPolyStats::CENTROID_LAT_FIELD_NAME = "centroid lat";
const string GenPolyStats::CENTROID_LON_FIELD_NAME = "centroid lon";
const string GenPolyStats::DATA_AREA_FIELD_NAME = "data area";
const string GenPolyStats::DATA_CENTROID_LAT_FIELD_NAME = "data centroid lat";
const string GenPolyStats::DATA_CENTROID_LON_FIELD_NAME = "data centroid lon";
const string GenPolyStats::DATA_HEIGHT_FIELD_NAME = "data height";
const string GenPolyStats::DATA_RANGE_FIELD_NAME = "data range";
const string GenPolyStats::ELEV_ANGLE_FIELD_NAME = "elev angle";
const string GenPolyStats::SCAN_MODE_FIELD_NAME = "scan mode";
const string GenPolyStats::SCAN_TIME_OFFSET_FIELD_NAME = "scan time offset";
const string GenPolyStats::VLEVEL_INDEX_FIELD_NAME = "vlevel index";

// constructor

GenPolyStats::GenPolyStats()
{
  clear();
}

// destructor

GenPolyStats::~GenPolyStats()
{
}

