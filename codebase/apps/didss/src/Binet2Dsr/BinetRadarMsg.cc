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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:39 $
//   $Id: BinetRadarMsg.cc,v 1.4 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BinetRadarMsg : Class representing a VIRAQ header read from a Binet
 *               data stream.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <dataport/smallend.h>
#include <toolsa/DateTime.hh>

#include "BinetRadarMsg.hh"
using namespace std;


BinetRadarMsg::BinetRadarMsg(const bool debug) :
  BinetMsg(debug)
{
}

BinetRadarMsg::~BinetRadarMsg() 
{
}


bool BinetRadarMsg::init(const char *buffer)
{
  char *buffer_ptr = (char *)buffer;

  // Copy each data field from the message.  Note that each field
  // must be individually copied using the memcpy() routine because
  // the data fields do not respect word boundaries.  The data in the
  // message is in small-endian format so each field is byte-swapped
  // as it is read, using the appropriate byte-swapping routine (byte
  // fields do not need swapping).

  memcpy(_description, buffer_ptr, sizeof(_description));
  buffer_ptr += sizeof(_description);

  memcpy(&_recordLen, buffer_ptr, sizeof(_recordLen));
  _recordLen = SE_to_si16(_recordLen);
  buffer_ptr += sizeof(_recordLen);

  memcpy(&_revision, buffer_ptr, sizeof(_revision));
  _revision = SE_to_si16(_revision);
  buffer_ptr += sizeof(_revision);

  memcpy(&_year, buffer_ptr, sizeof(_year));
  _year = SE_to_si16(_year);
  buffer_ptr += sizeof(_year);

  memcpy(_radarName, buffer_ptr, sizeof(_radarName));
  buffer_ptr += sizeof(_radarName);

  memcpy(&_polarization, buffer_ptr, sizeof(_polarization));
  buffer_ptr += sizeof(_polarization);

  memcpy(&_testPulsePower, buffer_ptr, sizeof(_testPulsePower));
  SE_swap_array_32((void *)&_testPulsePower, sizeof(_testPulsePower));
  buffer_ptr += sizeof(_testPulsePower);

  memcpy(&_testPulseFreq, buffer_ptr, sizeof(_testPulseFreq));
  SE_swap_array_32((void *)&_testPulseFreq, sizeof(_testPulseFreq));
  buffer_ptr += sizeof(_testPulseFreq);

  memcpy(&_frequency, buffer_ptr, sizeof(_frequency));
  SE_swap_array_32((void *)&_frequency, sizeof(_frequency));
  buffer_ptr += sizeof(_frequency);

  memcpy(&_peakPower, buffer_ptr, sizeof(_peakPower));
  SE_swap_array_32((void *)&_peakPower, sizeof(_peakPower));
  buffer_ptr += sizeof(_peakPower);

  memcpy(&_noiseFigure, buffer_ptr, sizeof(_noiseFigure));
  SE_swap_array_32((void *)&_noiseFigure, sizeof(_noiseFigure));
  buffer_ptr += sizeof(_noiseFigure);

  memcpy(&_noisePower, buffer_ptr, sizeof(_noisePower));
  SE_swap_array_32((void *)&_noisePower, sizeof(_noisePower));
  buffer_ptr += sizeof(_noisePower);

  memcpy(&_receiverGain, buffer_ptr, sizeof(_receiverGain));
  SE_swap_array_32((void *)&_receiverGain, sizeof(_receiverGain));
  buffer_ptr += sizeof(_receiverGain);

  memcpy(&_dataSysSat, buffer_ptr, sizeof(_dataSysSat));
  SE_swap_array_32((void *)&_dataSysSat, sizeof(_dataSysSat));
  buffer_ptr += sizeof(_dataSysSat);

  memcpy(&_antennaGain, buffer_ptr, sizeof(_antennaGain));
  SE_swap_array_32((void *)&_antennaGain, sizeof(_antennaGain));
  buffer_ptr += sizeof(_antennaGain);

  memcpy(&_horizBeamWidth, buffer_ptr, sizeof(_horizBeamWidth));
  SE_swap_array_32((void *)&_horizBeamWidth, sizeof(_horizBeamWidth));
  buffer_ptr += sizeof(_horizBeamWidth);

  memcpy(&_vertBeamWidth, buffer_ptr, sizeof(_vertBeamWidth));
  SE_swap_array_32((void *)&_vertBeamWidth, sizeof(_vertBeamWidth));
  buffer_ptr += sizeof(_vertBeamWidth);

  memcpy(&_xmitPulseWidth, buffer_ptr, sizeof(_xmitPulseWidth));
  SE_swap_array_32((void *)&_xmitPulseWidth, sizeof(_xmitPulseWidth));
  buffer_ptr += sizeof(_xmitPulseWidth);

  memcpy(&_radarConstant, buffer_ptr, sizeof(_radarConstant));
  SE_swap_array_32((void *)&_radarConstant, sizeof(_radarConstant));
  buffer_ptr += sizeof(_radarConstant);

  memcpy(&_phaseOffset, buffer_ptr, sizeof(_phaseOffset));
  SE_swap_array_32((void *)&_phaseOffset, sizeof(_phaseOffset));
  buffer_ptr += sizeof(_phaseOffset);

  memcpy(&_vertReceiverGain, buffer_ptr, sizeof(_vertReceiverGain));
  SE_swap_array_32((void *)&_vertReceiverGain, sizeof(_vertReceiverGain));
  buffer_ptr += sizeof(_vertReceiverGain);

  memcpy(&_vertTestPulsePower, buffer_ptr, sizeof(_vertTestPulsePower));
  SE_swap_array_32((void *)&_vertTestPulsePower, sizeof(_vertTestPulsePower));
  buffer_ptr += sizeof(_vertTestPulsePower);

  memcpy(&_vertAntennaGain, buffer_ptr, sizeof(_vertAntennaGain));
  SE_swap_array_32((void *)&_vertAntennaGain, sizeof(_vertAntennaGain));
  buffer_ptr += sizeof(_vertAntennaGain);

  memcpy(_misc, buffer_ptr, sizeof(_misc));
  SE_swap_array_32((void *)_misc, sizeof(_misc));
  buffer_ptr += sizeof(_misc);

  memcpy(_text, buffer_ptr, sizeof(_text));
  buffer_ptr += sizeof(_text);

  return true;
}


void BinetRadarMsg::print(ostream &stream) const
{
  stream << "Radar Header" << endl;
  stream << "------------" << endl;
  stream << "desc = <";
  for (int i = 0; i < 4; ++i)
  {
    if (isprint(_description[i]))
      stream << _description[i];
    else
      stream << "*";
  }
  stream << ">" << endl;
  stream << "record_len = " << _recordLen << endl;
  stream << "rev = " << _revision << endl;
  stream << "year = " << _year << endl;
  stream << "radar_name = <";
  for (int i = 0; i < 8; ++i)
  {
    if (_radarName[i] == '\0')
      break;
    if (isprint(_radarName[i]))
      stream << _radarName[i];
    else
      stream << "*";
  }
  stream << ">" << endl;
  stream << "polarization = " << _polarization << endl;
  stream << "test_pulse_pwr = " << _testPulsePower << endl;
  stream << "test_pulse_frq = " << _testPulseFreq << endl;
  stream << "frequency = " << _frequency << endl;
  stream << "peak_power = " << _peakPower << endl;
  stream << "noise_figure = " << _noiseFigure << endl;
  stream << "noise_power = " << _noisePower << endl;
  stream << "receiver_gain = " << _receiverGain << endl;
  stream << "data_sys_sat = " << _dataSysSat << endl;
  stream << "antenna_gain = " << _antennaGain << endl;
  stream << "horz_beam_width = " << _horizBeamWidth << endl;
  stream << "_vertBeamWidth = " << _vertBeamWidth << endl;
  stream << "xmit_pulsewidth = " << _xmitPulseWidth << endl;
  stream << "rconst= " << _radarConstant << endl;
  stream << "phaseoffset = " << _phaseOffset << endl;
  stream << "vreceiver_gain = " << _vertReceiverGain << endl;
  stream << "vtest_pulse_pwr = " << _vertTestPulsePower << endl;
  stream << "vantenna_gain = " << _vertAntennaGain << endl;
  for (int i = 0; i < 6; ++i)
    stream << "misc[" << i << "] = " << _misc[i] << endl;
  stream << "text = <";
  for (int i = 0; i < 960; ++i)
  {
    if (_text[i] == '\0')
      break;
    if (isprint(_text[i]))
      stream << _text[i];
    else
      stream << "*";
  }
  stream << ">" << endl;
  stream << endl;
}
