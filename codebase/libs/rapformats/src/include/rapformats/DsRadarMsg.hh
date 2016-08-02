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
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  April 1998
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_RADAR_MSG_INC_
#define _DS_RADAR_MSG_INC_


#include <string>
#include <didss/DsMessage.hh>
#include <rapformats/ds_radar.h>
#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsPlatformGeoref.hh>
#include <rapformats/DsRadarFlags.hh>
#include <rapformats/DsRadarCalib.hh>
#include <vector>
using namespace std;

class DsRadarMsg : public DsMessage
{

public:

  friend class DsRadarQueue;

  // part types

  enum partType {
    RADAR_PARAMS = DS_DATA_TYPE_RADAR_PARAMS,
    FIELD_PARAMS = DS_DATA_TYPE_RADAR_FIELD_PARAMS,
    RADAR_BEAM   = DS_DATA_TYPE_RADAR_BEAM_DATA,
    RADAR_FLAGS  = DS_DATA_TYPE_RADAR_FLAGS,
    RADAR_CALIB  = DS_DATA_TYPE_RADAR_CALIB,
    STATUS_XML   = DS_DATA_TYPE_STATUS_XML,
    PLATFORM_GEOREF = DS_DATA_TYPE_PLATFORM_GEOREF
  };

  DsRadarMsg();
  DsRadarMsg( const DsRadarMsg& source ){ copy( source ); }
  ~DsRadarMsg();
  
  DsRadarMsg& operator=( const DsRadarMsg &source );
   
  void copy( const DsRadarMsg& source );

  int disassemble( const void *in_msg, const int msg_len,
		   int *content  = NULL);

  ui08 *assemble( const int content =
		  RADAR_PARAMS | FIELD_PARAMS | RADAR_BEAM |
                  RADAR_FLAGS | RADAR_CALIB |
                  STATUS_XML | PLATFORM_GEOREF );

  void padBeams( bool padData, int numGates = 1 );

  inline bool allParamsSet() const { return paramsSet; }

  // const get fields should be used by apps reading this object

  inline const DsRadarParams& getRadarParams() const { return radarParams; }
  inline int numFields() const { return radarParams.numFields; }
  inline const vector< DsFieldParams* >&  getFieldParams() const
    { return fieldParams; }
  inline const DsFieldParams* getFieldParams(int n) const
    { return fieldParams[n]; }
  inline const DsRadarBeam& getRadarBeam() const { return radarBeam; }
  inline const DsPlatformGeoref& getPlatformGeoref() const
    { return platformGeoref; }
  inline const DsRadarFlags& getRadarFlags() const { return radarFlags; }
  inline const DsRadarCalib& getRadarCalib() const { return radarCalib; }
  inline const string& getStatusXml() const { return statusXml; }

  // non-const get fields should be used by apps writing to this object

  inline DsRadarParams& getRadarParams() { return radarParams; }
  inline vector< DsFieldParams* >&  getFieldParams() { return fieldParams; }
  inline DsFieldParams* getFieldParams(int n) { return fieldParams[n]; }
  inline DsRadarBeam& getRadarBeam() { return radarBeam; }
  inline DsPlatformGeoref& getPlatformGeoref() { return platformGeoref; }
  inline DsRadarFlags& getRadarFlags() { return radarFlags; }
  inline DsRadarCalib& getRadarCalib() { return radarCalib; }
  inline string& getStatusXml() { return statusXml; }

  // set methods

  void setRadarParams(const DsRadarParams &params);
  void setFieldParams(const DsFieldParams &params, int fieldNum);
  void setFieldParams(const vector<DsFieldParams *> params);

  void setRadarCalib(const DsRadarCalib &calib);
  void setRadarFlags(const DsRadarFlags &flags);
  void setRadarBeam(const DsRadarBeam &beam);
  void setPlatformGeoref(const DsPlatformGeoref &georef);

  void setStatusXml(const string &xml);
  void clearStatusXml();

  // clear the fields

  void clearFieldParams();

  // add a field parameter object

  void addFieldParams(const DsFieldParams &field);

  // Censor beam data based on values in one field.
  // The gate value for the censorFieldName field is tested for
  // inclusion between minValue and maxValue.
  // If not, all fields are set to the missing value.

  void censorBeamData(const string &censorFieldName,
                      double minValue,
                      double maxValue);

private:

  DsRadarParams             radarParams;
  vector< DsFieldParams* >  fieldParams;
  DsRadarBeam               radarBeam;
  DsPlatformGeoref          platformGeoref;
  DsRadarFlags              radarFlags;
  DsRadarCalib              radarCalib;
  string                    statusXml;

  // accumulated message content for all of the messages decoded so far
  int accumContent;
  
  bool paramsSet;

  bool pad; // pad output data to nGatesOut?
  int nGatesOut;
  int nGatesIn;
  
  void _clearFields();

};

#endif


