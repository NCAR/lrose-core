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
// Props.hh
//
// Props class - storm properties computations
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Props_HH
#define Props_HH

#include <physics/ZxRelation.hh>
#include <titan/storm.h>
#include <titan/TitanStormFile.hh>
#include "Worker.hh"
#include "Area.hh"
using namespace std;

class InputMdv;
class GridClump;
class Verify;

// layer statistics

typedef struct {
       
  int n;
  int n_vorticity;
  int sum_x;
  int sum_y;
       
  double sum_refl_x;
  double sum_refl_y;
  double sum_refl;
  double sum_mass;
  double sum_vel;
  double sum_vel2;
  double sum_vorticity;
  
  double area;
  double vol_centroid_x;
  double vol_centroid_y;
  double refl_centroid_x;
  double refl_centroid_y;
  double mass;
  double dbz_mean;
  double dbz_max;
  double vel_mean;
  double vel_sd;
  
  double vol_centroid_az;
  double vol_centroid_range;
  double vorticity;

  double htKm;
  
} layer_stats_t;

// struct for those variables which are summed

typedef struct {
  int n, n_vorticity;
  int x, y, z;
  double refl, refl_x, refl_y, refl_z;
  double precip, mass;
  double vel, vel2, vorticity;
} sum_stats_t;

#define MISSING_VAL -9999.0

////////////////////////////////
// Props

class Props : public Worker {
  
public:

  // constructor

  Props(const string &prog_name, const Params &params,
	const InputMdv &input_mdv, TitanStormFile &storm_file,
	Verify *verify);

  // destructor
  
  virtual ~Props();

  // initialize for latest MDV input file
  void init();

  // compute properties for clump
  int compute(const GridClump &grid_clump, int storm_num);

  // get methods
  double getMinValidZ() const { return _minValidZ; }

protected:
  
private:

  const InputMdv &_inputMdv;
  TitanStormFile &_sfile;
  Verify *_verify;
  Area _area;

  int _rangeLimited;
  int _topMissing;
  int _hailPresent;
  int _secondTrip;

  int _nzValid;
  double _minValidZ;

  int _topLayer, _baseLayer;
  int _nDbzIntvls;
  int _nLayers;

  double _testElevRad;
  double _testRangeKm;
  double _ZPInverseCoeff, _ZPInverseExpon;
  double _ZMInverseCoeff, _ZMInverseExpon;
  double _vortHemisphereFactor;
  double _minVortDist;
  
  int _nDbzHistIntervals;
  storm_file_global_props_t _gprops;
  sum_stats_t _sum;

  double *_means, *_eigenvalues;
  double **_eigenvectors;
  double **_tiltData;
  double **_dbzGradientData;
  layer_stats_t *_layer;
  dbz_hist_entry_t *_dbzHist;
  int _nZAlloc;
  int _nHistAlloc;

  // hail mass relationship

  ZxRelation  _hailZM;

  // Waldvogel and Federer probability of hail as a function of
  // the height of the 45 dBZ contour above the 0C isotherm

  typedef struct {
    double height;
    double probability;
  } heightProb_t;

  static const heightProb_t HEIGHT_PROB[];

  // temperature profile

  double _freezingLevel;
  double _htMinus20;
  double _ht45AboveFreezing;

  // methods

  void _alloc(int nz, int nhist);
  int _computeFirstPass(const GridClump &grid_clump);
  void _computeSecondPass(const GridClump &grid_clump);
  int _storeRuns(const GridClump &grid_clump);
  void _tiltCompute();
  void _dbzGradientCompute();
  int _checkSecondTrip();
     
  void _loadGprops(storm_file_global_props_t *gprops,
		   int storm_num,
		   int n_layers,
		   int base_layer,
		   int n_dbz_intvls,
		   int range_limited,
		   int top_missing,
		   int hail_present,
		   int second_trip);

  void _loadLprops(layer_stats_t *layer,
		   storm_file_layer_props_t *lprops);

  void _loadDbzHist(dbz_hist_entry_t *dbz_hist,
		    storm_file_dbz_hist_t *hist);

  double _topOfDbz(double dbz, const GridClump &grid_clump);

  // hail metrics
  
  void  _computeHailMetrics(const GridClump &grid_clump);
  int  _getFokrCategory(const GridClump &grid_clump);
  double _getWaldvogelProbability(const GridClump &grid_clump);

  // nexrad hail detection algorithm

  void _computeNexradHda(const GridClump &grid_clump,
                         double &poh, double &shi,
                         double &posh, double &mehs);
  
};

#endif



