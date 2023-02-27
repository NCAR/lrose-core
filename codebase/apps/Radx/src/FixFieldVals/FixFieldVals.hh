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
// FixFieldVals.hh
//
// FixFieldVals object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#ifndef FixFieldVals_HH
#define FixFieldVals_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <set>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
class RadxTime;
using namespace std;

class FixFieldVals {
  
public:

  // constructor
  
  FixFieldVals (int argc, char **argv);

  // destructor
  
  ~FixFieldVals();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  set<string> _readPaths;

  int _nWarnCensorPrint;

  // inner class for field diffs
  
  class FieldDiff {
  public:
    string corrName;
    string truthName;
    double sumCorr;
    double sumTruth;
    double nPts;
    double meanDiff;
    FieldDiff(const string &corrFieldName,
              const string &truthFieldName) {
      corrName = corrFieldName;
      truthName = truthFieldName;
      init();
    }
    void init() {
      sumCorr = 0.0;
      sumTruth = 0.0;
      nPts = 0.0;
      meanDiff = 0.0;
    }
    void computeMeanDiff() {
      if (nPts > 0) {
        double sumDiff = sumTruth - sumCorr;
        meanDiff = sumDiff / nPts;
      }
    }
  };
  vector<FieldDiff> _fieldDiffs;
  
  int _analyze(const vector<string> &inputPaths);
  int _correct(const vector<string> &inputPaths);
  int _readFile(const string &filePath,
                RadxVol &vol);
  int _analyzeVol(RadxVol &corr);
  int _computeFieldDiffs(RadxVol &corrVol, RadxVol &truthVol);
  void _finalizeVol(RadxVol &vol);
  void _setupCorrectionRead(RadxFile &file);
  void _setupTruthRead(RadxFile &file);
  void _applyLinearTransform(RadxVol &vol);
  int _readBiasFromSpdb(const RadxTime &volStartTime,
                        const string &fieldName, double &bias);
  void _convertFields(RadxVol &vol);
  void _convertAllFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);
  void _censorFields(RadxVol &vol);
  void _censorRay(RadxRay *ray);
  bool _checkFieldForCensoring(const RadxField *field);

};

#endif
