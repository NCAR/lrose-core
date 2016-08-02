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
// Mdv2Vil.hh
//
// Mdv2Vil object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Terri Betancourt
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Mdv2Vil_HH
#define Mdv2Vil_HH

#include "Args.hh"
#include "Params.hh"
#include <physics/vil.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <cmath>
using namespace std;

class DsInputPath;

class Mdv2Vil {
  
public:

  // constructor

  Mdv2Vil (int argc, char **argv);

  // destructor
  
  ~Mdv2Vil();

  // run 

  int Run();

  // data members

  bool isOK;
  
protected:
  
private:
  
  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  DsInputPath *_input;

  // fields

  int _nfieldsOut;
  bool _calcTotalVil;
  bool _calcDiffVil;
  bool _calcSSSIndex;

  // Parameters used in calculating the SSS values

  double _sssWeakDbzMax;
  double _sssSevereDbzMax;
  double _sssStdDeviationLimit;
  double _sssBaseCenterMassMax;
  double _sssTopCenterMassMin;
  double _sssBaseHeightMax;
  double _sssTopHeightMin;

  static const int WB = 1;   /* Weak Base 30-45 dBz*/
  static const int WV = 2;   /* Weak volume 30-45 dBz*/
  static const int WT = 3;   /* Weak Top  30-45 dBz*/
  static const int SB = 4;   /* Severe Base  45 -55 dBz*/
  static const int SV = 5;   /* Severe Volume 45-55 dBz*/
  static const int ST = 6;   /* Severe TOP  45-55 dBz*/
  static const int VSB = 7;  /* Very Severe Base  55+ dBz*/
  static const int VSV = 8;  /* Very Severe Volume  55+ dBz*/
  static const int VST = 9;  /* Very Severe Top 55+ dBz*/

  static const fl32 _missingFloat;

  int _processFile(const char *file_path);

  inline void MASS_Z(double &mass, double dbz) {
    if ((dbz) > VIL_THRESHOLD) {
      mass = (VILCONST * pow(10.0, (dbz) * FOURBYSEVENTY) * 1000.0);
    }
  }

  void _setupRead(DsMdvx &mdvx);
  void _computeVil(DsMdvx &inMdvx, DsMdvx &outMdvx);

  double _getHeight(int z, const MdvxField &field);

  double _getDeltaHt(int z, const MdvxField &field);

  int _computeSSS(double stdDeviation, double centerMass,
                  double maxDbzHeight, double maxDbz);
    
};

#endif
