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
// Clumping.hh
//
// Clumping class
//
// Provides services for run identification and clumping.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Clumping_HH
#define Clumping_HH

#include <euclid/clump.h>
#include <string>
#include <dataport/port_types.h>
using namespace std;

////////////////////////////////
// Clumping

class Clumping {
  
public:

  // constructor

  Clumping(const string &prog_name);

  // destructor
  
  virtual ~Clumping();

  int OK;

  Clump_order *clumps;
  int nClumps;

  // Find the run intervals in a 2D data grid
  int findIntervals(int nx, int ny,
		    unsigned char *data_grid,
		    int byte_threshold);

  // For float data
  int findIntervals(int nx, int ny,
		    fl32 *data_grid,
		    fl32 threshold);

   // Find the run intervals in a 3D data grid
  int findIntervals3D(int nx, int ny, int nz,
		      unsigned char *data_grid,
		      int byte_threshold);
    
  // For float data
  int findIntervals3D(int nx, int ny, int nz,
		      fl32 *data_grid,
		      fl32 byte_threshold);

   // Compute the EDM (Euclidean Distance Measure) of clumps,
  // load grid.
  void edm2D(int nx, int ny, unsigned char *edm_grid);
  
  // Erode clumps, put result in eroded grid
  void erode(int nx, int ny, unsigned char *eroded_grid,
	     int erosion_threshold);

  // perform clumping
  int performClumping(int nx, int ny, int nz,
		      unsigned char *data_grid,
		      int min_overlap,
		      int byte_threshold);

  // For float data
  int performClumping(int nx, int ny, int nz,
                      fl32 *data_grid,
                      int min_overlap,
                      fl32 threshold);

protected:
  
private:

  const string &_progName;

  Row_hdr *_rowh;
  int _nRowsAlloc;

  Interval *_intervals;
  int _nIntervalsAlloc;
  
  int _nIntOrderAlloc;
  Interval **_intervalOrder;

  // allocate row headers
  void _allocRowh(int nrows_per_vol);

};

#endif



