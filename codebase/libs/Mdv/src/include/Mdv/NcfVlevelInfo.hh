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
// NcfVlevelInfo.hh
//
// Vertical level information object for gridded dataset
// 
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008 
//
///////////////////////////////////////////////////////////////
//
//  NcfVlevelInfo object will bridge the Mdv vlevel info with the
//  netCDF dimensions and vertical coordinate variables. 
//
///////////////////////////////////////////////////////////////////////

#ifndef VLEVEL_INFO_H
#define VLEVEL_INFO_H

#include <string>
#include <math.h>
#include <Ncxx/Nc3xFile.hh>
#include <Mdv/MdvxField.hh> 
#include <Mdv/DsMdvx.hh>
#include <euclid/Pjg.hh>
#include <toolsa/pmu.h>

using namespace std;

#define VLEVEL_EPSILON .0000001

////////////////////////
// This class

class NcfVlevelInfo {
  
public:

  /// constructor

  NcfVlevelInfo (Mdvx::vlevel_header_t &vlevHdr, int vlevelType, int nz);

  /// destructor
  
  ~NcfVlevelInfo();
  
  /// add dimension for this object

  int addDim(int vlevelNum, Nc3File *ncFile, string &errStr);

  /// add vert level variable to NcFile object
  
  int addVlevelVar(int vlevelNum, Nc3File *ncFile, string &errStr);

  /// Write the vlevel data
  /// Returns 0 on success, -1 on error

  int writeDataToFile(Nc3File *ncFile, string &errStr);

  /// Get methods

  Nc3Dim *getNcZdim() const { return _zDim; }
  Nc3Var *getNcZvar() const { return _zVar; }

  /// equality

  bool operator==(const NcfVlevelInfo &other);
  
protected:
  
private:
  
  Mdvx::vlevel_header_t _vlevHdr;

  string _units;

  string _longName;

  string _standardName;

  string _positive;

  int _vlevelType;

  int _nz;

  Nc3Dim *_zDim;

  Nc3Var *_zVar;

  float _zData[MDV_MAX_VLEVELS];

};

#endif

