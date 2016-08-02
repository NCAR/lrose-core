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
// Area.hh
//
// Area class - storm area computations
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Area_HH
#define Area_HH

#include "Params.hh"
#include "Props.hh"
#include <titan/storm.h>
using namespace std;

class StormFile;
class InputMdv;
class Boundary;
class GridClump;

// struct dbz_hist_entry_t;

////////////////////////////////
// Area

class Area {
  
public:

  // constructor

  Area(char *prog_name, Params *params,
       InputMdv *input_mdv, StormFile *storm_file);

  // destructor
  
  virtual ~Area();

  // Initialize some variables for efficiency
  void init(double dbz_scale,
	    double dbz_bias,
	    double z_p_inverse_coeff,
	    double z_p_inverse_exponent);
  
  // compute()
  void compute(GridClump *grid_clump,
	       storm_file_global_props_t *gprops,
	       dbz_hist_entry_t *dbz_hist);

  // Store the projected area runs in the storm file handle

  int storeProjRuns();

  int OK;

protected:
  
private:

  char *_progName;
  Params *_params;
  InputMdv *_inputMdv;
  StormFile *_stormFile;
  GridClump *_gridClump;
  Boundary *_boundary;

  double _dbzScale, _dbzBias;
  double _zPInverseCoeff, _zPInverseExpon;
  double _zFactor;
  storm_file_global_props_t *_gProps;

  int _nPoints, _nX, _nY;
  int _nGridAlloc;
  ui08 *_compGrid;
  ui08 *_precipGrid;

  double *_means, *_eigenvalues;
  double **_eigenvectors;
  double **_areaCoords;
  int _nCoordsAlloc;

  void _ellipseCompute(ui08 *grid,
		       fl32 *area,
		       fl32 *area_centroid_x,
		       fl32 *area_centroid_y,
		       fl32 *area_orientation,
		       fl32 *area_major_radius,
		       fl32 *area_minor_radius);

  void _allocGrids();

  void _allocCoords(const int n_coords);

  void _computeProjPolygon();

  void _precipAndAreaHist(dbz_hist_entry_t *dbz_hist);
 
};

#endif



