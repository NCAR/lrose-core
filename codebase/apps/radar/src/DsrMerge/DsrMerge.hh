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
// DsrMerge.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////
//
// DsrMerge reads moments data from 2 Dsr file message queues,
// which contain data from 2 channels of the same radar system,
// in which there are differences. For example, there may be
// 2 transmitters operating at different frequencies, each of
// which has a separate moments data set. DsrMerge merges 
// these two data streams, and produces a single combined 
// data stream. In doing so, some fields are copied unchanged
// into the output queue. Other fields may be combined using 
// the mean of the two incoming fields.
//
////////////////////////////////////////////////////////////////

#ifndef DsrMerge_H
#define DsrMerge_H

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsRadarParams.hh>
#include <Fmq/DsRadarQueue.hh>
class Field;

using namespace std;

////////////////////////
// This class

class DsrMerge {
  
public:

  // constructor

  DsrMerge(int argc, char **argv);

  // destructor
  
  ~DsrMerge();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // input queues

  DsRadarQueue _inputQueue1;
  DsRadarQueue _inputQueue2;
  DsRadarMsg _inputMsg1;
  DsRadarMsg _inputMsg2;
  int _inputContents1;
  int _inputContents2;
 
  // output queue

  DsRadarQueue _outputQueue;
  DsRadarMsg _outputMsg;
  MemBuf _outBuf;

  // store params for output

  bool _paramsFoundSinceLastBeam;
  DsRadarParams _radarParams;

  // field list

  vector<Field *> _fields;
  
  // functions
  
  int _run();
  int _init();
  void _compileFieldList();
  void _clearFields();
  bool _beamsMatch();
  int _readQueue1();
  int _readQueue2();
  bool _queue1IsBehind();
  int _mergeAndWrite();
  int _writeForChan1Only();
  int _writeForChan2Only();
  int _performMerge();
  void _loadOutputBuffer(const DsRadarBeam &beam);
  int _openOutputFmq();
  int _writeBeam();
  int _writeParams();
  int _writeCalib(const DsRadarCalib &calib);
  int _writeFlags(const DsRadarFlags &flags);

};


#endif
