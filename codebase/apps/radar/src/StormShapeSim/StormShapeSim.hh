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
// StormShapeSim.h
//
// StormShapeSim object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2014
//
///////////////////////////////////////////////////////////////

#ifndef StormShapeSim_H
#define StormShapeSim_H

#include <tdrp/tdrp.h>
#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"

class RadxFile;
class RadxVol;

class StormShapeSim {
  
public:

  // constructor

  StormShapeSim (int argc, char **argv);

  // destructor
  
  ~StormShapeSim();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;

  int _readFile(const string &readPath, RadxVol &vol);
  void _createVol(RadxVol &vol);
  void _addGeomFields(RadxVol &vol);
  void _computeAgeHist(RadxVol &vol, double maxHtKm,
                       double &meanAgeFwd,
                       double &meanAgeRev,
                       vector<double> &cumFreqFwd,
                       vector<double> &cumFreqRev);
  void _writeAgeResults(RadxVol &vol,
                        vector<double> &maxHtKm,
                        vector<double> &meanAgeFwd,
                        vector<double> &meanAgeRev,
                        vector< vector<double> > &cumFreqFwd,
                        vector< vector<double> > &cumFreqRev);
  void _setupWrite(RadxFile &file);
  int _writeVol(RadxVol &vol);
  void _printRangeHeightTable(RadxVol &vol);

};

#endif
