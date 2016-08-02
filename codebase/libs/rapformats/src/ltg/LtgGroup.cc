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
// LtgGroup.cc
//
// C++ wrapper for generic point data.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2000
//////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <rapformats/LtgGroup.hh>
#include <toolsa/TaStr.hh>

using namespace std;

// Global constants

const int LtgGroup::SPDB_GROUP_NUM_32BIT_FIELDS = 10;

// constructor

LtgGroup::LtgGroup()
{
  clear();
}

// destructor

LtgGroup::~LtgGroup()
{
}


////////////////////
// clear

void LtgGroup::clear()
{
  _time = 0;
  _lat = 0.0;
  _lon = 0.0;
  _observeTime = 0;
  _netRadiance = 0.0;
  _footprint = 0.0;
  _childCount = 0;
  _glintIndex = 0.0;
  _oblongIndex = 0.0;
  _groupingSequence = 0;
  _approxThreshold = 0;
  _alertFlag = 0;
  _clusterIndex = 0;
  _densityIndex = 0;
  _noiseIndex = 0;
  _groupingStatus = 0;
}


///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the object values.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int LtgGroup::disassemble(const void *buf, int len)
{
  clear();
  _errStr = "ERROR - LtgGroup::disassemble()\n";
  
  // check length of group data
  
  if (len != (int)sizeof(spdb_group_t))
  {
    TaStr::AddInt(_errStr, "  Buffer incorrect size, len: ", len);
    TaStr::AddInt(_errStr, "  Expected size: ", sizeof(spdb_group_t));
    return -1;
  }
  
  // copy data
  
  spdb_group_t group;
  memcpy((char *)(&group), buf, len);
  _BE_to_group(group);
  
  _time = group.time;
  _lat = group.lat;
  _lon = group.lon;
  _observeTime = group.observe_time;
  _netRadiance = group.net_radiance;
  _footprint = group.footprint;
  _childCount = group.child_count;
  _glintIndex = group.glint_index;
  _oblongIndex = group.oblong_index;
  _groupingSequence = group.grouping_sequence;
  _approxThreshold = group.approx_threshold;
  _alertFlag = group.alert_flag;
  _clusterIndex = group.cluster_index;
  _densityIndex = group.density_index;
  _noiseIndex = group.noise_index;
  _groupingStatus = group.grouping_status;
  
  return 0;
}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.
//
// returns 0 on success, -1 on failure
// Use getErrStr() on failure.

int LtgGroup::assemble() const
{
  // load the group data

  spdb_group_t group;
  memset(&group, 0, sizeof(spdb_group_t));
  group.time = _time;
  group.lat = _lat;
  group.lon = _lon;
  group.observe_time = _observeTime;
  group.net_radiance = _netRadiance;
  group.footprint = _footprint;
  group.child_count = _childCount;
  group.glint_index = _glintIndex;
  group.oblong_index = _oblongIndex;
  group.grouping_sequence = _groupingSequence;
  group.approx_threshold = _approxThreshold;
  group.alert_flag = _alertFlag;
  group.cluster_index = _clusterIndex;
  group.density_index = _densityIndex;
  group.noise_index = _noiseIndex;
  group.grouping_status = _groupingStatus;
  
  _BE_from_group(group);

  // assemble buffer

  _memBuf.free();
  _memBuf.add(&group, sizeof(spdb_group_t));

  return 0;

}

////////////////////////////////////////////////////////
// prints

void LtgGroup::print(FILE *out) const

{
  fprintf(out, "  ===============================\n");
  fprintf(out, "  LtgGroup - lightning group data\n");
  fprintf(out, "  ===============================\n");
  fprintf(out, "  time: %s\n", utimstr(_time));
  fprintf(out, "  lat: %g\n", _lat);
  fprintf(out, "  lon: %g\n", _lon);
  fprintf(out, "  observe time: %d secs\n", _observeTime);
  fprintf(out, "  net radiance: %g uJ/ster/m2/um\n", _netRadiance);
  fprintf(out, "  footprint: %g km^2\n", _footprint);
  fprintf(out, "  child count: %d\n", _childCount);
  fprintf(out, "  glint index: %g\n", _glintIndex);
  fprintf(out, "  oblong index: %g\n", _oblongIndex);
  fprintf(out, "  grouping sequence: %d\n", _groupingSequence);
  fprintf(out, "  approx threshold: %d\n", _approxThreshold);
  fprintf(out, "  alert flag: %d\n", _alertFlag);
  fprintf(out, "  cluster index: %d\n", _clusterIndex);
  fprintf(out, "  density index: %d\n", _densityIndex);
  fprintf(out, "  noise index: %d\n", _noiseIndex);
  fprintf(out, "  grouping status: %d\n", _groupingStatus);
}

void LtgGroup::print(ostream &out) const

{
  out << "  ===============================" << endl;
  out << "  LtgGroup - lightning group data" << endl;
  out << "  ===============================" << endl;
  out << "  time: " << utimstr(_time) << endl;
  out << "  lat: " << _lat << endl;
  out << "  lon: " << _lon << endl;
  out << "  observe time: " << _observeTime << " secs" << endl;
  out << "  net radiance: " << _netRadiance << " uJ/ster/m2/um" << endl;
  out << "  footprint: " << _footprint << " km^2" << endl;
  out << "  child count: " << _childCount << endl;
  out << "  glint index: " << _glintIndex << endl;
  out << "  oblong index: " << _oblongIndex << endl;
  out << "  grouping sequence: " << _groupingSequence << endl;
  out << "  approx threshold: " << _approxThreshold << endl;
  out << "  alert flag: " << _alertFlag << endl;
  out << "  cluster index: " << _clusterIndex << endl;
  out << "  density index: " << _densityIndex << endl;
  out << "  noise index: " << _noiseIndex << endl;
  out << "  grouping status: " << _groupingStatus << endl;
}

/////////////////////////////////////////////////////////
// byte swapping routines

void LtgGroup::_BE_from_group(spdb_group_t &group) const
{
  BE_from_array_32(&group, SPDB_GROUP_NUM_32BIT_FIELDS * 4);
}

void LtgGroup::_BE_to_group(spdb_group_t &group) const
{
  BE_to_array_32(&group, SPDB_GROUP_NUM_32BIT_FIELDS * 4);
}
