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
// Mdv/MdvxRemapLut.hh
//
// An object of this class is used to hold the lookup table for
// computing grid remapping.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1999
//
////////////////////////////////////////////////////////////////////

#ifndef MdvxRemapLut_hh
#define MdvxRemapLut_hh

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <vector>
using namespace std;

class MdvxRemapLut
{

public:

  ///////////////////////
  // default constructor
  //
  // You need to call computeSamplePts() and
  // computeLut() before using the lookup table.
  
  MdvxRemapLut();
  
  //////////////////////////////////////////
  // constructor which computes lookup table
  
  MdvxRemapLut(const MdvxProj &proj_source,
	       const MdvxProj &proj_target);
  
  ///////////////////////
  // destructor
  
  virtual ~MdvxRemapLut();

  ///////////////////////////////
  // compute lookup table offsets
  
  void computeOffsets(const MdvxProj &proj_source,
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
  
  vector<int64_t> _sourceOffsets;
  vector<int64_t> _targetOffsets;

  bool _offsetsComputed;

private:

};

#endif


