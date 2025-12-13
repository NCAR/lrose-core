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

  ///////////////////////////////////////////////
  // constructor which sets target coordinates
  
  MdvxRemapInterp(const MdvxProj &projTgt,
                  const vector<double> &vlevelsTgt);
  
  ///////////////////////
  // destructor
  
  virtual ~MdvxRemapInterp();

  ///////////////////////////////////////////////////////////
  // interpolate a field, remapping to target projection
  //
  // Creates a field, returns pointer to that field.
  // Memory ownership passes back to caller.
  // The returned field must be freed by the caller.
  //
  // Side effect - the source field is converted to FLOAT32
  // and uncompressed
  
  MdvxField *interpField(MdvxField &sourceFld);
  
  // access to members

  const MdvxProj &getProjSource() const { return _projSource; }
  const MdvxProj &getProjTarget() const { return _projTarget; }
  
protected:
  
  MdvxProj _projSource;
  MdvxProj _projTarget;
  
  const Mdvx::coord_t *_coordSource;
  const Mdvx::coord_t *_coordTarget;

  vector<double> _vlevelsSource;
  vector<double> _vlevelsTarget;
  
  bool _lutComputed;

  typedef struct {
    double xx, yy;
    double wtx, wty;
    size_t sourceIndex;
  } xy_pt_t;

  typedef struct {
    xy_pt_t pt_ul;
    xy_pt_t pt_ur;
    xy_pt_t pt_ll;
    xy_pt_t pt_lr;
    size_t targetIndex;
  } xy_lut_t;

  vector<xy_lut_t> _xyLut;
  
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

  // compute lookup table
  
  void _computeLut(const MdvxProj &projSrc,
                   const vector<double> &vlevelsSrc);
  

  // Compute xy interpolation lookup table
  
  void _computeXyLookup();
  
  // Compute z interpolation lookup table
  
  void _computeZLookup();
  
private:

};

#endif


