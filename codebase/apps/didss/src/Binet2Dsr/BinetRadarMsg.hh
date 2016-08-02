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
// $Id: BinetRadarMsg.hh,v 1.6 2016/03/06 23:53:39 dixon Exp $
//
// Edge Message class
/////////////////////////////////////////////////////////

#ifndef BinetRadarMsg_hh
#define BinetRadarMsg_hh

#include <string>
#include <iostream>

#include <dataport/port_types.h>

#include "BinetMsg.hh"
#include "ProductConstants.hh"

using namespace std;


class BinetRadarMsg : public BinetMsg
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  BinetRadarMsg(const bool debug = false);
  ~BinetRadarMsg();

  bool init(const char *buffer);

  void print(ostream &stream) const;


  ////////////////////
  // Access methods //
  ////////////////////

  virtual double getAntennaGain() const
  {
    return _antennaGain;
  }

  virtual double getDataSysSat() const
  {
    return _dataSysSat;
  }

  virtual double getFrequency() const
  {
    return _frequency;
  }

  virtual double getHorizBeamWidth() const
  {
    return _horizBeamWidth;
  }

  virtual msg_type_t getMsgType() const
  {
    return RADAR_MSG;
  }

  virtual double getNoisePower() const
  {
    return _noisePower;
  }

  virtual double getPeakPower() const
  {
    return _peakPower;
  }

  virtual double getPhaseOffset() const
  {
    return _phaseOffset;
  }

  virtual char getPolarization() const
  {
    return _polarization;
  }

  virtual double getRadarConstant() const
  {
    return _radarConstant;
  }

  virtual string getRadarName() const
  {
    return (char *)_radarName;
  }

  virtual double getReceiverGain() const
  {
    return _receiverGain;
  }

  virtual double getVertAntennaGain() const
  {
    return _vertAntennaGain;
  }

  virtual double getVertBeamWidth() const
  {
    return _vertBeamWidth;
  }

  virtual double getVertNoisePower() const
  {
    return _vertTestPulsePower;
  }

  virtual double getVertRcvrGain() const
  {
    return _vertReceiverGain;
  }

  virtual double getWavelength() const
  {
    return ProductConstants::C / 10000.0 / _frequency;
  }

  virtual double getXmitPulsewidth() const
  {
    return _xmitPulseWidth;
  }


private:

  /////////////////////
  // Private members //
  /////////////////////

  // Data values in the radar parameters message.  The types of these
  // members must be exactly the same size as the field size in the
  // message itself since these are used for pulling the data from the
  // message.  Note that any type changes here will require changes to
  // the code where the associated byte-swapping routine is called.

  ui08 _description[4];         // == 'RHDR'
  si16 _recordLen;
  si16 _revision;
  si16 _year;
  ui08 _radarName[8];
  ui08 _polarization;          // H or V
  fl32 _testPulsePower;        // TP power (referred to antenna flange)
  fl32 _testPulseFreq;         // test pulse frequency
  fl32 _frequency;             // transmit frequency
  fl32 _peakPower;             // typical xmit power (at antenna flange)
  fl32 _noiseFigure;
  fl32 _noisePower;            // for subtracting from data
  fl32 _receiverGain;          // gain from antenna flange to PIRAQ input
  fl32 _dataSysSat;            // PIRAQ input power required for full scale
  fl32 _antennaGain;
  fl32 _horizBeamWidth;
  fl32 _vertBeamWidth;
  fl32 _xmitPulseWidth;        // transmitted pulse width
  fl32 _radarConstant;         // radar constant
  fl32 _phaseOffset;           // offset for phi dp
  fl32 _vertReceiverGain;      // ver gain from antenna flange to VIRAQ
  fl32 _vertTestPulsePower;    // vert TP power referred to antenna flange
  fl32 _vertAntennaGain;
  fl32 _misc[6];
  ui08 _text[960];

};

#endif
