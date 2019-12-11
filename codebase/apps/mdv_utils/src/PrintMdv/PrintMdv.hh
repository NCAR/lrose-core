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
// PrintMdv.hh
//
// PrintMdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#ifndef PrintMdv_H
#define PrintMdv_H

#include <Mdv/DsMdvxThreaded.hh>
#include "Args.hh"
#include "Params.hh"
#include <string>
using namespace std;

class PrintMdv {
  
public:

  // constructor

  PrintMdv (int argc, char **argv);

  // destructor
  
  ~PrintMdv();

  // run 

  int Run();

  // data members

  int OK;

protected:

  time_t _readSearchTime;
  time_t _latestValidModTime;
  time_t _timeListStartTime;
  time_t _timeListEndTime;
  time_t _timeListGenTime;
  time_t _timeListSearchTime;
  
  void _setupRead(DsMdvx *mdvx);

  int _handleVolume(DsMdvx *mdvx);
  bool _needData();
  int _handleVsection(DsMdvx *mdvx);
  int _handleAllHeaders(DsMdvx *mdvx);
  int _handleTimeList(DsMdvx *mdvx);
  int _handleTimeHeight(DsMdvx *mdvx);

  int _getVolume(DsMdvx *mdvx);
  int _getAllHeaders(DsMdvx *mdvx);
  int _getVsection(DsMdvx *mdvx);
  int _getTimeList(DsMdvx *mdvx);
  int _getTimeHeight(DsMdvx *mdvx);
  
  void _setTimeListMode(DsMdvx *mdvx);

  void _printVolume(const DsMdvx *mdvx) const;
  void _printVsection(const DsMdvx *mdvx) const;
  void _doPrintVol(const DsMdvx *mdvx) const;

  int _doTest(DsMdvx *mdvx);
  void _printSizes(ostream &out);

  MdvxTimeList::time_list_mode_t getTimeListMode(Params::time_list_mode_t mode);

private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

};

#endif
