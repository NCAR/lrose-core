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
// InputMdv.hh
//
// InputMdv class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef InputMdv_HH
#define InputMdv_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <rapformats/titan_grid.h>
#include "Worker.hh"
#include "ConvectionFinder.hh"
using namespace std;

#define N_BYTE_DATA_VALS 256

////////////////////////////////
// InputMdv

class InputMdv : public Worker {
  
public:

  // constructor

  InputMdv(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~InputMdv();

  // mdvx object

  DsMdvx mdvx;
  Mdvx::coord_t coord;

  MdvxField *dbzField;
  MdvxField *velField;

  MdvxProj proj;

  titan_grid_t grid;

  // public data members

  int nFields;
  int minValidLayer;
  int maxValidLayer;
  int minPrecipLayer;
  int maxPrecipLayer;
  int specifiedPrecipLayer;
  int nPlanes;

  fl32 *dbzVol;
  fl32 *velVol;
  fl32 *compDbz;
  fl32 dbzMiss;

  // read file for given time
  int read(time_t data_time);

protected:
  
private:

  ConvectionFinder _convFinder;

  void _copyCoord2Grid(const Mdvx::coord_t &coord,
		       titan_grid_t &grid);

  void _computeComposite();
  void _removeStratiform();

};

#endif


