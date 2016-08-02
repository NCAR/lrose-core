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
// ChillCal2Xml.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// ChillCal2Xml reads a CHILL cal file, and reformats to
// an XML file in DsRadarCal format
//
////////////////////////////////////////////////////////////////

#ifndef ChillCal2Xml_H
#define ChillCal2Xml_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>

using namespace std;

////////////////////////
// This class

class ChillCal2Xml {
  
public:

  // constructor

  ChillCal2Xml (int argc, char **argv);

  // destructor
  
  ~ChillCal2Xml();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double piCubed;
  static const double lightSpeed;
  static const double kSquared;

  //////////////
  // data members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  DsInputPath *_reader;

  time_t _calTime;
  double _radarConstH;
  double _radarConstV;

  double _noise_v_rx_1;
  double _noise_h_rx_2;
  double _noise_v_rx_2;
  double _noise_h_rx_1;
  double _noise_v_rx_1_dbm;
  double _noise_h_rx_2_dbm;
  double _noise_v_rx_2_dbm;
  double _noise_h_rx_1_dbm;
  double _zdr_bias_db;
  double _ldr_bias_h_db;
  double _ldr_bias_v_db;
  double _gain_v_rx_1_db;
  double _gain_v_rx_2_db;
  double _gain_h_rx_1_db;
  double _gain_h_rx_2_db;
  double _zdr_cal_base_vhs;
  double _zdr_cal_base_vh;
  double _sun_pwr_v_rx_1_db;
  double _sun_pwr_h_rx_2_db;
  double _sun_pwr_v_rx_2_db;
  double _sun_pwr_h_rx_1_db;

  // methods

  int _processFile(const char* filePath);
  int _readChill(const char* filePath);
  int _writeXml();
  
  double _computeRadarConstant(double xmitPowerDbm,
                               double antennaGainDb,
                               double twoWayWaveguideLossDb,
                               double twoWayRadomeLossDb);

};

#endif
