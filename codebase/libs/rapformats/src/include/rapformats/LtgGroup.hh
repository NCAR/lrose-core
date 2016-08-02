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
// LtgGroup.hh
//
// C++ class for lightning group data.  Lightning group data comes
// from TRMM LIS files.  The information in these files was used to
// define what information is stored in the SPDB databases.
//
// Nancy Rehak, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// August 2009
//////////////////////////////////////////////////////////////

#ifndef _LtgGroup_hh
#define _LtgGroup_hh


#include <string>
#include <cstdio>
#include <iostream>
#include <vector>

#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

class LtgGroup
{

public:

  // constructor

  LtgGroup();

  // destructor

  ~LtgGroup();
  
  ///////////////////////////////////////////////////////////////
  // set methods
  
  void clear();

  void setTime(const time_t utime) { _time = utime; }
  void setTime(const DateTime &date_time) { _time = date_time.utime(); }
  void setLat(const double lat) { _lat = lat; }
  void setLon(const double lon) { _lon = lon; }  
  void setLocation(const double lat, const double lon) 
  {
    _lat = lat;
    _lon = lon;
  }
  void setObserveTime(const int observe_time) { _observeTime = observe_time; }
  void setNetRadiance(const double net_radiance)
    { _netRadiance = net_radiance; }
  void setFootprint(const double footprint) { _footprint = footprint; }
  void setChildCount(const int child_count) { _childCount = child_count; }
  void setGlintIndex(const double glint_index) { _glintIndex = glint_index; }
  void setOblongIndex(const double oblong_index)
    { _oblongIndex = oblong_index; }
  void setGroupingSequence(const int grouping_sequence)
    { _groupingSequence = grouping_sequence; }
  void setApproxThreshold(const unsigned char approx_thresh)
    { _approxThreshold = approx_thresh; }
  void setAlertFlag(const unsigned char alert_flag)
    { _alertFlag = alert_flag; }
  void setClusterIndex(const unsigned char cluster_index)
    { _clusterIndex = cluster_index; }
  void setDensityIndex(const unsigned char density_index)
    { _densityIndex = density_index; }
  void setNoiseIndex(const unsigned char noise_index)
    { _noiseIndex = noise_index; }
  void setGroupingStatus(const unsigned char grouping_status)
    { _groupingStatus = grouping_status; }
  

  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the object values.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure

  int disassemble(const void *buf, int len);

  //////////////////////////////////////////////////////////////////
  // get methods

  time_t getTime() const { return _time; }
  double getLat() const { return _lat; }
  double getLon() const { return _lon; }  
  void getLocation(double &lat, double &lon) const
  {
    lat = _lat;
    lon = _lon;
  }
  int getObserveTime() const { return _observeTime; }
  double getNetRadiance() const { return _netRadiance; }
  double getFootprint() const { return _footprint; }
  int getChildCount() const { return _childCount; }
  double getGlintIndex() const { return _glintIndex; }
  double getOblongIndex() const { return _oblongIndex; }
  int getGroupingSequence() const { return _groupingSequence; }
  unsigned char getApproxThreshold() const { return _approxThreshold; }
  unsigned char getAlertFlag() const { return _alertFlag; }
  unsigned char getClusterIndex() const { return _clusterIndex; }
  unsigned char getDensityIndex() const { return _densityIndex; }
  unsigned char getNoiseIndex() const { return _noiseIndex; }
  unsigned char getGroupingStatus() const { return _groupingStatus; }
  

  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  //
  // returns 0 on success, -1 on failure
  // Use getErrStr() on failure.
  
  int assemble() const;

  // get the assembled buffer pointer

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ////////////////
  // error string
  
  const string &getErrStr() const { return (_errStr); }

  /////////////////////////
  // print

  void print(FILE *out) const;
  void print(ostream &out) const;

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  // struct for data in the SPDB database

  typedef struct
  {
    ti32 time;
    fl32 lat;
    fl32 lon;
    ui32 observe_time;  // Duration of observation of the region where the
                        //   group occurred in seconds.
    fl32 net_radiance;  // Sum of even radiances composing this group in
                        //   uJ/ster/m2/um
    fl32 footprint;     // Unique areal extent in km^2.
    ui32 child_count;   // Number of events in group.
    fl32 glint_index;   // Cosine of angle.
    fl32 oblong_index;  // Metric indicating how oblong the group is.
    si32 grouping_sequence;
                        // Time sequence of group used when grouping algorithm
                        //   is applied.
    fl32 spare_fl32[2];
    
    ui08 approx_threshold;
                        // Extimated value of 8-bit threshold for the group
                        //   determined from background level or solar zenith
                        //   angle.
    ui08 alert_flag;    // Bit masked status of instrument, platform, external
                        //   factors and processing algorithms.
    ui08 cluster_index; // Pixel density metric; higher numbers indicate group
                        //   is less likely to be noise.  Values 0-99.
    ui08 density_index; // Spatial density metric; higher if group geolocated
                        //   in a region of high lightning activity.
    ui08 noise_index;   // Signal-to-signal plus noise ratio in %x100.
    ui08 grouping_status;
                        // 0 = group grouped normally 
                        // 1 = group split between orbits
                        // 2 = group split between orbits
                        // 3 = grouping algorithm failed
    ui08 spare_ui08[2];
  } spdb_group_t;


  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int SPDB_GROUP_NUM_32BIT_FIELDS;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  time_t _time;
  double _lat;
  double _lon;
  int _observeTime;
  double _netRadiance;
  double _footprint;
  int _childCount;
  double _glintIndex;
  double _oblongIndex;
  int _groupingSequence;
  
  unsigned char _approxThreshold;
  unsigned char _alertFlag;
  unsigned char _clusterIndex;
  unsigned char _densityIndex;
  unsigned char _noiseIndex;
  unsigned char _groupingStatus;
  
  mutable MemBuf _memBuf;

  mutable string _errStr;

private:

  void _BE_from_group(spdb_group_t &group) const;
  void _BE_to_group(spdb_group_t &group) const;

};


#endif
