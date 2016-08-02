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
// IntfRemove.hh
//
// IntfRemove object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2008
//
///////////////////////////////////////////////////////////////
//
// IntfRemove reads DBZ and SNR data in an input DsRadar FMQ,
// identifies interference and removes the interference power
// from the power fields, and writes the cleaned up data out to
// a DsRadar queue
//
///////////////////////////////////////////////////////////////////////

#ifndef IntfRemove_HH
#define IntfRemove_HH

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class IntfRemove {
  
public:

  // constructor

  IntfRemove (int argc, char **argv);

  // destructor
  
  ~IntfRemove();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double _missingDouble;
  static const double _missingTest;

  // members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  bool _debugPrintNeedsNewline;

  // FMQ input
  
  DsRadarQueue *_inputQueue;
  DsRadarMsg _inputMsg;
  int _inputContents;
  int _inputNFail;
  int _nFieldsIn;
  int _nFieldsOut;
  int _nGates;

  DsRadarParams _inputRadarParams;
  vector<DsFieldParams*> _inputFieldParams;
  DsRadarCalib _inputRadarCalib;

  // FMQ output
  
  DsRadarQueue *_outputQueue;
  DsRadarMsg _outputMsg;

  // data arrays

  int _nGatesAlloc;
  TaArray<double> _snr_, _snrMedian_, _dbz_;
  double *_snr, *_snrMedian, *_dbz;

  // DBZ and SNR field information
  
  int _snrIndex;
  int _dbzIndex;
  DsFieldParams _snrFieldParams;
  DsFieldParams _dbzFieldParams;

  // interference characteristics

  double _interferenceSnrDb;
  double _backgroundSnrDb;
  double _backgroundSnrSum;
  double _backgroundSnrCount;

  // functions
  
  int _run();

  int _openInputQueue();
  int _openOutputQueue();

  int _processInputMessage();
  
  void _copyFlags();

  int _processBeam();
  
  int _getFieldIndices();
  int _getInputFieldIndex(const string &dsrName);

  void _loadInputField(const DsRadarBeam &beam,
		       int index,
		       const DsFieldParams &fieldParams,
		       double *fldData);

  void _computeSnrFromDbz();

  bool _interferencePresent(int startGate, int endGate);

  void _removeInterference(int startGate, int endGate);

  void _printInputData(ostream &out);

  int _writeRadarParams();
  int _writeFieldParams();
  int _writeRadarCalib();

  int _writeBeam();

  void _loadOutputField(float scale, float bias,
			int missingDataValue, int byteWidth,
			double *fld,
			int outPos, ui08 *outData);

  void _printDebugMsg(const string &msg, bool addNewline = true);
  
  void _allocateArrays(int nGates);

  void _applyNexradSpikeFilter();
  
};

#endif

