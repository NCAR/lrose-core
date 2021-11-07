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
// ClumpingMgr.hh
//
// ClumpingMgr class
//
// Provides services for run identification and clumping.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
// Copied over from apps/titan/src/Titan/Clumping class
//
///////////////////////////////////////////////////////////////

#ifndef ClumpingMgr_HH
#define ClumpingMgr_HH

#include <string>
#include <euclid/clump.h>
#include <dataport/port_types.h>
#include <euclid/PjgGridGeom.hh>
class ClumpingDualThresh;
class ClumpProps;
using namespace std;

////////////////////////////////
// ClumpingMgr

class ClumpingMgr {
  
public:

  // constructor

  ClumpingMgr();

  // destructor
  
  virtual ~ClumpingMgr();

  // Find the run intervals in a 2D data grid
  int findIntervals(int nx, int ny,
		    const unsigned char *data_grid,
		    int byte_threshold);

  // For float data
  int findIntervals(int nx, int ny,
		    const fl32 *data_grid,
		    fl32 threshold);

   // Find the run intervals in a 3D data grid
  int findIntervals3D(int nx, int ny, int nz,
		      const unsigned char *data_grid,
		      int byte_threshold);
    
  // For float data
  int findIntervals3D(int nx, int ny, int nz,
		      const fl32 *data_grid,
		      fl32 byte_threshold);

  // Compute the EDM (Euclidean Distance Measure) of clumps,
  // load grid.
  
  void edm2D(int nx, int ny, unsigned char *edm_grid);
  
  // Erode clumps, put result in eroded grid
  
  void erode(int nx, int ny, unsigned char *eroded_grid,
	     int erosion_threshold);

  // perform clumping

  int performClumping(int nx, int ny, int nz,
		      const unsigned char *data_grid,
		      int min_overlap,
		      int byte_threshold);

  // For float data

  int performClumping(int nx, int ny, int nz,
                      const fl32 *data_grid,
                      int min_overlap,
                      fl32 threshold);

  // get number of clumps, after performClumping()

  int getNClumps() const { return _nClumps; }

  // get clumps after performClumping()
  // turn 1-based into 0-based

  const Clump_order *getClumps() const { return _clumps + 1; }

  // set whether to use dual thresholds

  void setUseDualThresholds(double secondary_threshold,
                            double min_fraction_all_parts,
                            double min_fraction_each_part,
                            double min_size_each_part,
                            double min_clump_volume,
                            double max_clump_volume,
                            bool debug = false);

  // Perform clumping specifying the input geom and input data array.
  // If using dual thresholds, call setUseDualThresholds().
  // Return: fills out clumps vector.

  void loadClumpVector(PjgGridGeom &inputGeom, 
                       const fl32 *inputData,
                       double primary_threshold,
                       int min_grid_overlap,
                       vector<ClumpProps> &clumps,
                       double min_volume_km3 = 0.0);

  // Adjust the clumps for an (x, y) grid offset
  // for example when performing dual thresholding
  // The offsets will be added to the location of the interval
  
  void addXyOffsetToIntervals(int ixOffset, int iyOffset);

  // get debug grids from using dual threshold

  const fl32 *getDualThreshCompFileGrid() const;
  const ui08 *getDualThreshAllFileGrid() const;
  const ui08 *getDualThreshValidFileGrid() const;
  const ui08 *getDualThreshGrownFileGrid() const;

protected:
  
private:

  Clump_order *_clumps;
  int _nClumps;

  Row_hdr *_rowh;
  int _nRowsAlloc;

  Interval *_intervals;
  int _nIntervalsAlloc;
  
  int _nIntOrderAlloc;
  Interval **_intervalOrder;

  PjgGridGeom _gridGeom;
  ClumpingDualThresh *_dualT;

  // allocate row headers
  void _allocRowh(int nrows_per_vol);

};

#endif



