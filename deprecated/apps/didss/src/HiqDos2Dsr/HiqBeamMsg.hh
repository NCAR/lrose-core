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
//////////////////////////////////////////////////////////
// $Id: HiqBeamMsg.hh,v 1.2 2016/03/06 23:53:40 dixon Exp $
//
// HiQ Message class
/////////////////////////////////////////////////////////

#ifndef HiqBeamMsg_hh
#define HiqBeamMsg_hh

#include <iostream>

#include <dataport/port_types.h>

#include "HiqMsg.hh"

using namespace std;


class HiqBeamMsg : public HiqMsg
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    DATA_SIMPLEPP = 0,
    DATA_POLYPP,
    DATA_DUALPP,
    DATA_POL1,
    DATA_POL2,
    DATA_POL3,
    DATA_SIMPLE16,
    DATA_DOW,
    DATA_FULLPOL1,
    DATA_FULLPOLP,
    DATA_MAXPOL,
    DATA_HVSIMUL,
    DATA_SHRTPUL,
    DATA_SMHVSIM
  } data_format_t;


  ////////////////////
  // Public methods //
  ////////////////////

  HiqBeamMsg(const bool debug = false);
  HiqBeamMsg(const HiqBeamMsg &rhs);
  ~HiqBeamMsg();

  bool init(const char *buffer);

  void print(ostream &stream) const;
  void printSummary(ostream &stream) const;


  ////////////////////
  // Access methods //
  ////////////////////

  // Set methods

  virtual void setAzimuth(const double azimuth)
  {
    _azimuth = azimuth;
  }

  virtual void setElevation(const double elevation)
  {
    _elevation = elevation;
  }

  // Get methods

  virtual si16 *getAbp() const
  {
    return _abp;
  }

  virtual double getAzimuth() const
  {
    return _azimuth;
  }

  virtual data_format_t getDataFormat() const
  {
    return (data_format_t)_dataFormat;
  }

  virtual time_t getDataTime() const
  {
    return _time;
  }

  virtual double getElevation() const
  {
    return _elevation;
  }

  virtual double getGateSpacing() const   // m
  {
    return (_rcvrPulseWidth / 0.000001) * 150.0;
  }

  virtual int getHits(void) const
  {
    return _hits;
  }

  virtual double getHorizXmitPower() const
  {
    return _horizXmitPower;
  }

  virtual msg_type_t getMsgType() const
  {
    return BEAM_MSG;
  }

  virtual int getNumGates() const
  {
    return _gates;
  }

  virtual double getPrf(void) const
  {
    return 1.0 / _prt;
  }

  virtual double getPrt(void) const
  {
    return _prt;
  }

  virtual double getPrt2(void) const
  {
    return _prt2;
  }

  virtual double getRadarAltitude() const
  {
    return _radarAltitude;
  }

  virtual double getRadarLatitude() const
  {
    return _radarLatitude;
  }

  virtual double getRadarLongitude() const
  {
    return _radarLongitude;
  }

  virtual int getRayCount() const
  {
    return _rayCount;
  }

  virtual double getRcvrPulseWidth(void) const
  {
    return _rcvrPulseWidth;
  }

  virtual int getSamplesPerBeam(void) const
  {
    return _hits;
  }

  virtual int getScanType(void) const
  {
    return _scanType;
  }

  virtual double getStartRange() const
  {
    return 0.0;
  }

  virtual double getTargetElevation(void) const
  {
    return _elevation;
  }

  virtual int getTiltNumber(void) const
  {
    return _scanNum;
  }

  virtual double getUnambiguousRange(void) const
  {
    return _prt * 299.79 *500.0;
  }

  virtual double getVertXmitPower() const
  {
    return _vertXmitPower;
  }

  virtual int getVolumeNumber(void) const
  {
    return _volumeNum;
  }


private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int HEADER_SIZE;


  /////////////////////
  // Private members //
  /////////////////////

  // Data values in the header of the beam message.  The types of these
  // members must be exactly the same size as the field size in the
  // message itself since these are used for pulling the data from the
  // message.  Note that any type changes here will require changes to
  // the code where the associated byte-swapping routine is called.

  ui08 _description[4];         // == 'DWEL'
  si16 _recordLen;              // header + data
  si16 _gates;
  si16 _hits;
  fl32 _rcvrPulseWidth;
  fl32 _prt;
  fl32 _delay;                  // delay to first gate
  ui08 _clutterFilter;
  ui08 _timeSeries;
  si16 _tsGate;
  si32 _time;                   // seconds since 1970
  si16 _subSeconds;             // fractional seconds (.1 mS)
  fl32 _azimuth;
  fl32 _elevation;
  fl32 _radarLongitude;
  fl32 _radarLatitude;
  fl32 _radarAltitude;
  fl32 _ewVelocity;
  fl32 _nsVelocity;
  fl32 _vertVelocity;
  ui08 _dataFormat;             // see VIRAQFormats enum
  fl32 _prt2;
  fl32 _fixedAngle;
  ui08 _scanType;
  ui08 _scanNum;                // NOT SET FOR UAE RADARS!!
  ui08 _volumeNum;              // NOT SET FOR UAE RADARS!!
  si32 _rayCount;
  ui08 _transition;
  fl32 _horizXmitPower;         // on the fly hor power
  fl32 _vertXmitPower;          // on the fly ver power
  ui08 _spare[100];

  si16 *_abp;
  int _abpSize;
  
};

#endif
