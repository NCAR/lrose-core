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
// NcfGridInfo.hh
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008 
//
///////////////////////////////////////////////////////////////
//
//  NcfGridInfo object will bridge the Mdv projection info with the
//  netCDF dimensions, coordinate variables,  and auxilliary variables 
//  (if relevant). 
//
///////////////////////////////////////////////////////////////////////

#ifndef GRID_INFO_H
#define GRID_INFO_H

#include <string>
#include <math.h>
#include <Ncxx/Nc3xFile.hh>
#include <Mdv/MdvxField.hh> 
#include <Mdv/DsMdvx.hh>
#include <euclid/Pjg.hh>
#include <toolsa/pmu.h>

using namespace std;

////////////////////////
// This class

class NcfGridInfo {
  
public:

  /// constructor

  NcfGridInfo (Mdvx::field_header_t fHdr);

  /// destructor
  
  ~NcfGridInfo();
  
  ///  Compute projection x and y arrays. If data is NOT in lat lon projection
  ///  compute the auxiliary 2D coordinate variables. See Section 5 of CF-1.0 
  ///  Coordnate Systems for discussion on auxiliary coordinate variables
  
  int computeCoordinateVars();
  
  /// For an Xsection, set the CoordinateVars from the sample points

  int setCoordinateVarsFromSamplePoints(vector<Mdvx::vsect_samplept_t> pts);

  /// add xy dimension for this grid
  /// returns 0 on success, -1 on failure
  
  int addXyDim(int gridNum, Nc3File *ncFile, string &errStr);

  /// add projection variable for this grid
  
  int addProjVar(int projNum, Nc3File *ncFile, string &errStr);

  /// add coordinate variables for this grid
  
  int addCoordVars(int gridNum, bool outputLatlonArrays,
                   Nc3File *ncFile, string &errStr);

  /// add vert section coordinate variables for this grid
  
  int addVsectCoordVars(int gridNum,
                        Nc3File *ncFile, string &errStr);
    
  /// Write the coordinate data to file
  /// Returns 0 on success, -1 on error
  
  int writeCoordDataToFile(Nc3File *ncFile, string &errStr);
  
  // Get methods
  
  Nc3Dim *getNcXdim() const { return _xDim; }
  Nc3Dim *getNcYdim() const { return _yDim; }
  Nc3Var *getNcXvar() const { return _xVar; }
  Nc3Var *getNcYvar() const { return _yVar; }
  Nc3Var *getNcLatVar() const { return _latVar; }
  Nc3Var *getNcLonVar() const { return _lonVar; }
  
  Mdvx::projection_type_t getProjType() const { return _proj.getProjType();}
  const Nc3Var *getNcProjVar()  const{ return _projVar;}

  bool getOutputLatlonArrays() const { return _outputLatlonArrays; }

  /// equality  -- Assume the objects are equal if their coordinate structures
  /// are equal.  This requires that the structure be initialized to all zeros
  /// before setting any of the structure's fields.

  bool operator==(const NcfGridInfo &other) const;
  
protected:
  
private:
 
  Mdvx::field_header_t _fHdr;
  MdvxProj _proj;
  Mdvx::coord_t _coord;
  Nc3Var *_projVar;
  bool _outputLatlonArrays;
  bool _isXSect;
  
  Nc3Dim *_xDim;
  Nc3Dim *_yDim;

  Nc3Var *_xVar;
  Nc3Var *_yVar;
  Nc3Var *_latVar;
  Nc3Var *_lonVar;
  Nc3Var *_altVar;

  float *_xData;
  float *_yData;
  float *_lonData;
  float *_latData;
  
  void _clear();

};

#endif

