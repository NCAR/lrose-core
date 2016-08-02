// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// TsGrabber.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// TsGrabber reads time-series data from RVP8,
// and processes it in various ways
//
////////////////////////////////////////////////////////////////

#ifndef TsGrabber_H
#define TsGrabber_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Input.hh"
#include "Stats.hh"
#include <rvp8_rap/RapComplex.hh>
#include <rvp8_rap/Socket.hh>

using namespace std;

////////////////////////
// This class

class TsGrabber {
  
public:

  // constructor

  TsGrabber (int argc, char **argv);

  // destructor
  
  ~TsGrabber();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  Input *_input;

  int _nSamples;
  int _nGates;
  int _startGate;
  bool _dualChannel;
  bool _fastAlternating;
  bool _onceOnly;
  bool _labviewRequest;

  int _totalPulseCount;
  int _pulseCount;
  int _printCount;
  
  double _midTime, _midPrt;
  double _midAz, _midEl;
  int _midAzBits, _midElBits;

  Stats _stats;
  vector<Stats> _aStats;
  int _nGatesAscope;

  int _runPrintMode();
  int _runCalMode();
  int _runServerMode();
  int _runAscopeMode();
  
  // process one pulse

  // compute stats

  void _addToSummary(const TsPulse &pulse);
  void _computeSummary();

  void _addToAlternating(const TsPulse &pulse);
  void _computeAlternating();

  void _addToAscope(const TsPulse &pulse);

  void _clearStats();

  // server mode
  
  int _handleClient(Socket *sock);
  void _reapChildren();
  int _readCommands(Socket *sock, string &xmlBuf);
  int _decodeCommands(string &xmlBuf);
  void _prepareXmlResponse(int iret, string &xmlResults);
  void _prepareLabviewResponse(int iret, string &xmlResults);
  int _writeResponse(Socket *sock, string &xmlBuf);

  // printing

  void _printRunDetails(ostream &out);
  void _printOpsInfo(ostream &out);
  
  void _printPulseLabels(ostream &out);
  string _pulseString(const TsPulse &pulse);

  void _printSummaryHeading(ostream &out);
  void _printAlternatingHeading(ostream &out);
  
  void _printSummaryLabels(ostream &out);
  void _printSummaryData(FILE *out);
  void _printAlternatingLabels(ostream &out);
  void _printAlternatingData(FILE *out, const TsPulse *pulse = NULL);
  
  string _majorMode2String(int majorMode);
  string _prfMode2String(int prfMode);
  string _phaseSeq2String(int phaseSeq);
  string _polarization2String(int polarization);
  string _iangle2BitStr(int iang);
  
};

#endif
