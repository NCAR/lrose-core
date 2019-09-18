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
// WriteToFile.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Resample IWRF time series data,
// convert to APAR time series format,
// and write out to files
//
////////////////////////////////////////////////////////////////

#ifndef WriteToFile_HH
#define WriteToFile_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/apar_ts_data.h>
#include <radar/AparTsInfo.hh>

using namespace std;

////////////////////////
// This class

class WriteToFile {
  
public:

  // constructor
  
  WriteToFile(const string &progName,
              const Params &params,
              vector<string> &inputFileList);

  // destructor
  
  ~WriteToFile();

  // run 

  int Run();
  
protected:
  
private:
  
  string _progName;
  const Params &_params;
  vector<string> _inputFileList;

  // output file

  FILE *_out;

  // APAR-style metadata

  AparTsInfo *_aparTsInfo;
  AparTsDebug_t _aparTsDebug;
  apar_ts_radar_info_t _aparRadarInfo;
  apar_ts_scan_segment_t _aparScanSegment;
  apar_ts_processing_t _aparTsProcessing;
  apar_ts_calibration_t _aparCalibration;

  // pulse details

  ui64 _dwellSeqNum;
  ui64 _pulseSeqNum;
  vector<IwrfTsPulse *> _dwellPulses;
  
  // functions

  int _convertFile(const string &inputPath);
  int _processDwell(vector<IwrfTsPulse *> &dwellPulses);
  int _openOutputFile(const string &inputPath,
                      const IwrfTsPulse &pulse);
  void _closeOutputFile();
  
  void _reformat2Apar(const IwrfTsPulse &pulse);

  void _convertMeta2Apar(const IwrfTsInfo &info);

  void _copyIwrf2Apar(const iwrf_packet_info_t &iwrf,
                      apar_ts_packet_info_t &apar);

  void _copyIwrf2Apar(const iwrf_radar_info_t &iwrf,
                      apar_ts_radar_info_t &apar);

  void _copyIwrf2Apar(const iwrf_scan_segment_t &iwrf,
                      apar_ts_scan_segment_t &apar);

  void _copyIwrf2Apar(const iwrf_ts_processing_t &iwrf,
                      apar_ts_processing_t &apar);

  void _copyIwrf2Apar(const iwrf_calibration_t &iwrf,
                      apar_ts_calibration_t &apar);

  void _copyIwrf2Apar(const iwrf_pulse_header_t &iwrf,
                      apar_ts_pulse_header_t &apar);

  void _copyIwrf2Apar(const iwrf_event_notice_t &iwrf,
                      apar_ts_event_notice_t &apar);

};

#endif
