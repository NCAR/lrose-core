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
// RadxDiffFields.hh
//
// RadxDiffFields object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2016
//
///////////////////////////////////////////////////////////////
//
// RadxDiffFields computes statistics about the difference
// between fields in Radx files.
// The fields can be in the same file, or in different files.
// The results are written to SPDB as XML.
//
///////////////////////////////////////////////////////////////

#ifndef RadxDiffFields_H
#define RadxDiffFields_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <deque>
#include <set>
#include <radar/RadarMoments.hh>
#include <radar/AlternatingVelocity.hh>
#include <radar/InterestMap.hh>
#include <radar/IwrfCalib.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxRcalib.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxRcalib;
class RadxField;
class IwrfCalib;
using namespace std;

class RadxDiffFields {
  
public:

  // constructor
  
  RadxDiffFields (int argc, char **argv);

  // destructor
  
  ~RadxDiffFields();

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
  vector<string> _readPaths;

  class FieldDiff {
  public:
    FieldDiff(const Params::field_pair_t &pair) :
            param(pair) {
      primaryName = pair.primary_field_name;
      if (!pair.fields_are_in_same_file) {
        secondaryName += "sec_";
      }
      secondaryName += pair.secondary_field_name;
    }
    void clear() {
      diffs.clear();
      diffMean = NAN;
      diffPercentiles.clear();
    }
    const Params::field_pair_t &param;
    string primaryName;
    string secondaryName;
    vector<double> diffs;
    double diffMean;
    vector<double> diffPercentiles;
  };
  vector<FieldDiff *> _fields;
  
  // private methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();

  void _setupPrimaryRead(RadxFile &file);
  void _setupSecondaryRead(RadxFile &file,
                           const string &secondaryDir);

  int _processPrimaryFile(const string &filePath);
  
  int _addSecondaryFields(RadxVol &primaryVol,
                          const string &secondaryDir);

  int _mergeVol(const RadxVol &primaryVol, RadxVol &secondaryVol);
  void _mergeRay(RadxRay &primaryRay, const RadxRay &secondaryRay);
  
  int _loadDiffs(RadxVol &vol);
  int _loadDiffs(const RadxRay *ray);
  void _loadDiffs(FieldDiff &diff,
                  const RadxField &pField,
                  const RadxField &sField,
                  const RadxField *cField);

  void _computeDiffStats();
  double _computePerc(FieldDiff &result,
                      double percent);

  int _writeResults(const RadxVol &vol);

};

#endif
