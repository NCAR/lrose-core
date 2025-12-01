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
////////////////////////////////////////////////////////////////////
// Mdv/MdvxRemapInterp.hh
//
// An object of this class is used to hold the data for interpolation
// from one MdvxProj grid mapping to another.
//
// This method uses bilinear interpolation for remapping in 3-D.
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 2025
//
////////////////////////////////////////////////////////////////////

#ifndef MdvxRemapInterp_hh
#define MdvxRemapInterp_hh

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <vector>
using namespace std;

class MdvxRemapInterp
{

public:

  ///////////////////////
  // default constructor
  //
  // You need to call computeSamplePts() and
  // computeLut() before using the lookup table.
  
  MdvxRemapInterp();
  
  //////////////////////////////////////////
  // constructor which computes lookup table
  
  MdvxRemapInterp(const MdvxProj &proj_source,
                  const MdvxProj &proj_target);
  
  ///////////////////////
  // destructor
  
  virtual ~MdvxRemapInterp();

  ///////////////////////////////
  // compute lookup table
  
  void computeLut(const MdvxProj &proj_source,
                  const MdvxProj &proj_target);
  
  // access to members

  const MdvxProj &getProjSource() const { return _projSource; }
  const MdvxProj &getProjTarget() const { return _projTarget; }
  
  size_t getNOffsets() const { return _sourceOffsets.size(); }
  const int64_t *getSourceOffsets() const { return _sourceOffsets.data(); }
  const int64_t *getTargetOffsets() const { return _targetOffsets.data(); }
  
protected:
  
  MdvxProj _projSource;
  MdvxProj _projTarget;
  
  Mdvx::vlevel_header_t _vlevelSource;
  Mdvx::vlevel_header_t _vlevelTarget;

  vector<int64_t> _sourceOffsets;
  vector<int64_t> _targetOffsets;

  bool _lutComputed;

  typedef struct {
    double zz;
    int indexLower;
    int indexUpper;
    double zLower;
    double zUpper;
    double wtLower;
    double wtUpper;
  } z_lut_t;

  vector<z_lut_t> _zLut;

  
  // Compute z interpolation lookup table
  
  void _computeZLut();
  
private:

};

#endif


