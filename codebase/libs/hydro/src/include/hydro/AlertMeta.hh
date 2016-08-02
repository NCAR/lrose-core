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
// AlertMeta.hh
//
// Class representing the meta data for a gauge in the alert network.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2000
//
///////////////////////////////////////////////////////////////

#ifndef AlertMeta_H
#define AlertMeta_H

#include <iostream>
#include <cstdio>
#include <string>

#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>
using namespace std;


// Constant defines
//
// These should actually be private static constants, but then they
// couldn't be used in the typedef below in any way that I can figure
// out.

#define ALERT_META_AFOS_ID_LEN    8
#define ALERT_META_LOCAL_TZ_LEN   8



class AlertMeta
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructors

  AlertMeta();
  
  AlertMeta(const int provider_id,
	    const string &afos_id,
	    const string &name,
	    const double elevation,
	    const double latitude,
	    const double longitude,
	    const int num_instruments,
	    const int num_levels = 0,
	    const string &local_time_zone = "GMT",
	    const string &location_descr = "",
	    const string &station_type = "",
	    const string &maint_schedule = "",
	    const string &site_descr = "",
	    const bool debug_flag = false);

  // destructor
  
  ~AlertMeta();


  ////////////////////
  // Access methods //
  ////////////////////

  // Clear the information in the object.

  void clear(void);
  

  // Add the given gauge type value to the meta data.
  // The gauge type must be between 0 and 31, inclusive.

  inline void addGaugeType(const int gauge_type)
  {
    if (gauge_type < 0 || gauge_type > 31)
      return;
    
    _gaugeTypeMask &= (1 < gauge_type);
  }
  
  // Retrieve the provider id for the gauge

  inline int getProviderId(void) const
  {
    return _providerId;
  }
  

  // Retrieve the AFOS id for the gauge

  inline const string &getAfosId(void) const
  {
    return _afosId;
  }
  

  // Retrieve the latitude of the gauge

  inline double getLatitude(void) const
  {
    return _latitude;
  }
  

  // Retrieve the longitude of the gauge

  inline double getLongitude(void) const
  {
    return _longitude;
  }
  

  // Retrieve the station type for the gauge

  inline string getStationType(void) const
  {
    return _stationType;
  }
  

  /////////////////////////
  // SPDB access methods //
  /////////////////////////

  // Loads up the internal buffer from the object to prepare it for
  // writing to an SPDB database.
  // Handles byte swapping.
  //
  // Returns true on success, false on failure

  bool assemble();
  

  // Get the assembled buffer pointer.

  void *getBufPtr(void) const
  {
    return _memBuf.getPtr();
  }
  

  // Get the assembled buffer length.

  int getBufLen(void) const
  {
    return _memBuf.getLen();
  }
  

  // Disassembles a buffer and sets the object values.
  // Handles byte swapping.
  //
  // Returns true on success, false on failure

  bool disassemble(const void *buf, int len);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the current AlertMeta information to the given stream for
  // debugging purposes.

  void print(ostream &stream) const;
  
  void print(FILE *stream) const;
  

  ///////////////////////////////
  // Error information methods //
  ///////////////////////////////

  const string &getErrStr(void) const
  {
    return _errStr;
  }
  

protected:
  
private:

  //////////////////////
  // Private typedefs //
  //////////////////////

  // The header written to the SPDB database.  The data in the database
  // appears in the following order:
  //
  //       AlertMeta::header_t
  //       Name string: name_len chars, including trailing null
  //       Location description string: location_descr_len chars, including
  //                 trailing null (location_descr_len is 0 if missing)
  //       Station type string: station_type_len chars, including
  //                 trailing null (station_type_len is 0 if missing)
  //       Maintenance schedule string: maint_schedule_len chars, including
  //                 trailing null (maint_schedule_len is 0 if missing)
  //       Site description string: site_descr_len chars, including trailing
  //                 null (site_descr_len is 0 if missing)

  typedef struct
  {
    si32 provider_id;
    si32 num_instruments;
    si32 num_levels;
    si32 gauge_type_mask;
    
    si32 name_len;
    si32 location_descr_len;
    si32 station_type_len;
    si32 maint_schedule_len;
    si32 site_descr_len;
    
    fl32 elevation;
    fl32 latitude;
    fl32 longitude;
    
    char afos_id[ALERT_META_AFOS_ID_LEN];
    char local_tz[ALERT_META_LOCAL_TZ_LEN];

    si32 buf_len;
    si32 si_spare[3];
  } header_t;
  

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  // Meta data for the gauge

  int _providerId;
  string _afosId;
  string _name;
  double _elevation;
  double _latitude;
  double _longitude;
  string _localTimeZone;
  string _locationDescr;
  string _stationType;
  int _numInstruments;
  int _numLevels;
  string _maintenanceSchedule;
  string _siteDescr;
  
  si32 _gaugeTypeMask;
  

  // Memory buffer for assembling the data in preparation for writing
  // to SPDB.

  MemBuf _memBuf;
  
  // Error information

  mutable string _errStr;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Swap the header from bigendian to memory format.

  static void _swapHeaderFromBE(header_t &hdr);
  
  // Swap the header from memory to bigendian format.

  static void _swapHeaderToBE(header_t &hdr);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("AlertMeta");
  }
  
};

#endif
