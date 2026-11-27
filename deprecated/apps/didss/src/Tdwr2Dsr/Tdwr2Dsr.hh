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
// Tdwr2Dsr.hh
//
// Tdwr2Dsr object
//
// Gary BLackburn, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2001
//
///////////////////////////////////////////////////////////////

#ifndef Tdwr2Dsr_HH
#define Tdwr2Dsr_HH

#include <string>
#include <vector>
#include "Args.hh"
#include "Params.hh"
#include "UdpMsg.hh"
#include "TapeMsg.hh"
#include "tdwr.h"
#include "TDWRadial.hh"

#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarParams.hh>
#include <toolsa/MemBuf.hh>
using namespace std;


// class DsInputPath;

////////////////////////
// This class

class Tdwr2Dsr {
  
public:

  // constructor
  Tdwr2Dsr (int argc, char **argv);

  // destructor
  ~Tdwr2Dsr();

  // run 
  int Run();

  bool getTdwrUdpData(unsigned char **radial, int  *rad_len);

  bool okay;

  static const int    MAX_BUF_SIZE;

protected:
  
private:

  char *_progName;
  char *_paramsPath;
  Args *_args;
  Params _params;
  DsRadarQueue _rQueue;     // output queue contains data in Dsr format
  UdpMsg *_udp_input;       // object handling UDP communications for data input
  TapeMsg *_tape_input;  // object handling playback input
  MemBuf *_radial_buffer;   // buffer used to store input radial
  unsigned char *_data_ptr;
  TDWRadial *_tdwr_data; // input radial stored in this object
  TDWRadial::scan_description_t  _tdwrParams;

  // used to convert 16 bit velocities into 8 bit
  unsigned short _minCompressedVelocity;  // min compressed velocity value
  unsigned short _maxCompressedVelocity;  // max compressed velocity value

  int _volNum;
  int _beamCount;
  int _ppi_num;
  int _currentScanType;

  int _nScans;
  int _nFields;

  // TDWR data contains message headers that allow multiple packets
  // to be reassembled to represent a complete radial
  unsigned        _last_frame_seq;
  bool            _in_progress;
  int             _first_time; 

  void _convert2Dsr (unsigned char *tdwr_data, int data_len);

  void _setDsRadarParams(DsRadarParams& rParams);
  void _setDsFieldParams(vector <DsFieldParams*> &fieldParams);

  
  void _writeBeam(unsigned char *radar_data, DsRadarBeam& radarBeam);
  void _applyDataFlags(ui08 *data_ptr, unsigned char flag_byte);

  void _print_debug_tilt_vals ();
  void _print_debug_vol_vals ();

};

#endif
