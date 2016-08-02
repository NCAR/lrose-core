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
///////////////////////////////////////////////////////////////
// AlertMeta.cc
//
// Class representing the meta data for a gauge in the alert network.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cstdio>
#include <string>

#include <dataport/bigend.h>
#include <hydro/AlertMeta.hh>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
using namespace std;


// Define global constants



/*********************************************************************
 * Constructors
 */

AlertMeta::AlertMeta(void)
{
  clear();
}


AlertMeta::AlertMeta(const int provider_id,
		     const string &afos_id,
		     const string &name,
		     const double elevation,
		     const double latitude,
		     const double longitude,
		     const int num_instruments,
		     const int num_levels,
		     const string &local_time_zone,
		     const string &location_descr,
		     const string &station_type,
		     const string &maint_schedule,
		     const string &site_descr,
		     const bool debug_flag) :
  _debugFlag(debug_flag),
  _providerId(provider_id),
  _afosId(afos_id),
  _name(name),
  _elevation(elevation),
  _latitude(latitude),
  _longitude(longitude),
  _localTimeZone(local_time_zone),
  _locationDescr(location_descr),
  _stationType(station_type),
  _numInstruments(num_instruments),
  _numLevels(num_levels),
  _maintenanceSchedule(maint_schedule),
  _siteDescr(site_descr),
  _gaugeTypeMask(0)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

AlertMeta::~AlertMeta()
{
  // Do nothing
}


/*********************************************************************
 * assemble() - Loads up the internal buffer from the object to prepare
 *              it for writing to an SPDB database.
 *              Handles byte swapping.
 *
 * Returns true on success, false on failure
 */

bool AlertMeta::assemble()
{
  const string method_name = "assemble()";
  
  // Initialize the error string, just in case

  _errStr = string("ERROR: ") + _className() + "::" + method_name + "\n";
  
//  // Check if we have a valid object
//
//  if (!check()) {
//    _errStr += "ERROR - GenPt::assemble()\n";
//    return -1;
//  }

  // Load the header

  header_t hdr;

  memset(&hdr, 0, sizeof(hdr));

  hdr.provider_id = _providerId;
  hdr.num_instruments = _numInstruments;
  hdr.num_levels = _numLevels;
  hdr.gauge_type_mask = _gaugeTypeMask;
  
  hdr.name_len = _name.size() + 1;
  hdr.location_descr_len = _locationDescr.size() + 1;
  hdr.station_type_len = _stationType.size() + 1;
  hdr.maint_schedule_len = _maintenanceSchedule.size() + 1;
  hdr.site_descr_len = _siteDescr.size() + 1;
  
  hdr.elevation = _elevation;
  hdr.latitude = _latitude;
  hdr.longitude = _longitude;
  
  STRcopy(hdr.afos_id, _afosId.c_str(), ALERT_META_AFOS_ID_LEN);
  STRcopy(hdr.local_tz, _localTimeZone.c_str(), ALERT_META_LOCAL_TZ_LEN);
  
  hdr.buf_len = sizeof(header_t) +
    hdr.name_len + hdr.location_descr_len + hdr.station_type_len +
    hdr.maint_schedule_len + hdr.site_descr_len;
  
  _swapHeaderToBE(hdr);

  // Assemble the buffer

  _memBuf.free();
  _memBuf.add(&hdr, sizeof(header_t));

  _memBuf.add(_name.c_str(), _name.size() + 1);
  _memBuf.add(_locationDescr.c_str(), _locationDescr.size() + 1);
  _memBuf.add(_stationType.c_str(), _stationType.size() + 1);
  _memBuf.add(_maintenanceSchedule.c_str(), _maintenanceSchedule.size() + 1);
  _memBuf.add(_siteDescr.c_str(), _siteDescr.size() + 1);
  
  return true;
}


/*********************************************************************
 * clear() - Clear the information in the object.
 */

void AlertMeta::clear(void)
{
  _providerId = 0;
  _afosId = "";
  _name = "";
  _elevation = 0.0;
  _latitude = 0.0;
  _longitude = 0.0;
  _localTimeZone = "GMT";
  _stationType = "";
  _numInstruments = 0;
  _numLevels = 0;
  _maintenanceSchedule = "";
  _siteDescr = "";
  
  _gaugeTypeMask = 0;
  
  _memBuf.free();
}


/*********************************************************************
 * disassemble() - Disassembles a buffer and sets the object values.
 *                 Handles byte swapping.
 *
 * Returns true on success, false on failure
 */

bool AlertMeta::disassemble(const void *buf, int len)
{
  const string method_name = "disassemble()";
  
  // Clear the current object information

  clear();

  // Initialize the error string, just in case

  _errStr = string("ERROR: ") + _className() + "::" + method_name + "\n";
  
  // Check minimum len for header.
  
  if (len < (int)sizeof(header_t))
  {
    TaStr::AddInt(_errStr, "  Buffer too short for header, len: ", len);
    TaStr::AddInt(_errStr, "  Minimum valid len: ",
		  sizeof(header_t));

    return false;
  }
  
  // Local copy of buffer

  _memBuf.add(buf, len);
  
  // Get header
  
  header_t *hdr = (header_t *)_memBuf.getPtr();

  _swapHeaderFromBE(*hdr);

  _providerId = hdr->provider_id;
  _numInstruments = hdr->num_instruments;
  _numLevels = hdr->num_levels;
  _gaugeTypeMask = hdr->gauge_type_mask;
  
  _elevation = hdr->elevation;
  _latitude = hdr->latitude;
  _longitude = hdr->longitude;

  _afosId = hdr->afos_id;
  _localTimeZone = hdr->local_tz;
  
  // Check the expected buffer length.
  
  if (len != hdr->buf_len)
  {
    TaStr::AddInt(_errStr, "  Buffer wrong length, len: ", len);
    TaStr::AddInt(_errStr, "  Expected len: ", hdr->buf_len);

    return false;
  }
  
  // name string
  
  char *string_ptr = (char *)((char *)_memBuf.getPtr() + sizeof(header_t));
  char *name = string_ptr;

  if (hdr->name_len > 0)
  {
    name[hdr->name_len - 1] = '\0';
    _name = name;
  }
  
  // location description string
  
  string_ptr += hdr->name_len;
  char *location_descr = string_ptr;

  if (hdr->location_descr_len > 0)
  {
    location_descr[hdr->location_descr_len - 1] = '\0';
    _locationDescr = location_descr;
  }
  
  // station type string
  
  string_ptr += hdr->location_descr_len;
  char *station_type = string_ptr;

  if (hdr->station_type_len > 0)
  {
    station_type[hdr->station_type_len - 1] = '\0';
    _stationType = station_type;
  }
  
  // maintenance schedule string
  
  string_ptr += hdr->station_type_len;
  char *maint_schedule = string_ptr;

  if (hdr->maint_schedule_len > 0)
  {
    maint_schedule[hdr->maint_schedule_len - 1] = '\0';
    _maintenanceSchedule = maint_schedule;
  }
  
  // site description string
  
  string_ptr += hdr->maint_schedule_len;
  char *site_descr = string_ptr;

  if (hdr->site_descr_len > 0)
  {
    site_descr[hdr->site_descr_len - 1] = '\0';
    _siteDescr = site_descr;
  }
  
  return true;
}


/*********************************************************************
 * print() - Print the current AlertMeta information to the given stream
 *           for debugging purposes.
 */

void AlertMeta::print(ostream &stream) const
{
  stream << "AlertMeta information:" << endl;
  stream << "======================" << endl;
  stream << "debug flag = " << _debugFlag << endl;
  stream << "provider id = " << _providerId << endl;
  stream << "AFOS id = " << _afosId << endl;
  stream << "name = " << _name << endl;
  stream << "elevation = " << _elevation << endl;
  stream << "latitude = " << _latitude << endl;
  stream << "longitude = " << _longitude << endl;
  stream << "local time zone = " << _localTimeZone << endl;
  stream << "location descr = " << _locationDescr << endl;
  stream << "station type = " << _stationType << endl;
  stream << "num instruments = " << _numInstruments << endl;
  stream << "num levels = " << _numLevels << endl;
  stream << "maint schedule = " << _maintenanceSchedule << endl;
  stream << "site descr = " << _siteDescr << endl;
  stream << "gauge type mask = " << _gaugeTypeMask << endl;
  stream << endl;
}


void AlertMeta::print(FILE *stream) const
{
  fprintf(stream, "AlertMeta information:\n");
  fprintf(stream, "======================\n");
  fprintf(stream, "debug flag = %d\n", _debugFlag);
  fprintf(stream, "provider id = %d\n", _providerId);
  fprintf(stream, "AFOS id = %s\n", _afosId.c_str());
  fprintf(stream, "name = %s\n", _name.c_str());
  fprintf(stream, "elevation = %f\n", _elevation);
  fprintf(stream, "latitude = %f\n", _latitude);
  fprintf(stream, "longitude = %f\n", _longitude);
  fprintf(stream, "local time zone = %s\n", _localTimeZone.c_str());
  fprintf(stream, "location descr = %s\n", _locationDescr.c_str());
  fprintf(stream, "station type = %s\n", _stationType.c_str());
  fprintf(stream, "num instruments = %d\n", _numInstruments);
  fprintf(stream, "num levels = %d\n", _numLevels);
  fprintf(stream, "maint schedule = %s\n", _maintenanceSchedule.c_str());
  fprintf(stream, "site descr = %s\n", _siteDescr.c_str());
  fprintf(stream, "gauge type mask = %x\n", _gaugeTypeMask);
  fprintf(stream, "\n");
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/*********************************************************************
 * _swapHeaderFromBE() - Swap the header from bigendian to memory format.
 */

void AlertMeta::_swapHeaderFromBE(header_t &hdr)
{
  BE_to_array_32(&hdr, sizeof(header_t));
}


/*********************************************************************
 * _swapHeaderToBE() - Swap the header from memory to bigendian format.
 */

void AlertMeta::_swapHeaderToBE(header_t &hdr)
{
  BE_from_array_32(&hdr, sizeof(header_t));
}
