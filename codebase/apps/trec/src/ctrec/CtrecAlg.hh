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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/06 23:28:57 $
 *   $Id: CtrecAlg.hh,v 1.17 2016/03/06 23:28:57 dixon Exp $
 *   $Revision: 1.17 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * CtrecAlg.hh : header file for the CtrecAlg class.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef CtrecAlg_HH
#define CtrecAlg_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <vector>

#include <dataport/port_types.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>

#include "GridSearcher.hh"
#include "NoiseGenerator.hh"
#include "SimpleGrid.hh"
using namespace std;


class CtrecAlg
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const double BAD_OUTPUT_VALUE;
  static const ui08 BAD_COUNT_VALUE;
  

  //////////////////
  // Public types //
  //////////////////

  typedef struct
  {
    double x;
    double y;
  } grid_pt_t;
  
  
  //////////////////////////////
  // Constructors/destructors //
  //////////////////////////////

  // Constructor

  CtrecAlg(const double min_echo,
	   const double max_echo,
	   const bool track_top_percentage_flag,
	   const double track_top_percentage,
	   const bool track_top_percentage_increasing,
	   const double max_spd_echo,
	   const double cbox_fract,
	   const double cbox_size,
	   const double cbox_space,
	   const double thr_cor,
	   const GridSearcher<fl32> cormax_searcher,
	   NoiseGenerator *noise_generator,
	   const bool thresh_vectors_flag,
	   const double rad_mean,
	   const double thr_vec,
	   const double thr_diff,
	   const bool fill_vectors_flag,
	   const double rad_vec,
	   const int num_quad_vec,
	   const double min_vec_pts,
	   vector< grid_pt_t > correlation_loc_list,
	   const bool debug_flag = false,
	   const bool print_global_mean = false,
	   const bool output_correlation_grid = false,
	   const bool output_cormax_count_grid = false,
	   const bool output_original_vectors = false,
	   const bool output_local_mean_grids = false,
	   const bool output_local_mean_thresh_vectors = false,
	   const bool output_global_mean_thresh_vectors = false);
  
  // Destructor

  ~CtrecAlg(void);
  
  // Run the algorithm on the given grids.
  //
  // Note that if this class is moved to a library, the run() method
  // will have to check to make sure the grids used by the two fields
  // match.  It will also have to call _setGrid() whenever the grid
  // changes.  The Ctrec class makes sure the grids for all images
  // match exactly.
  //
  // Returns true if the algorithm was successful, false otherwise.

  bool run(const MdvxField &prev_field,
	   const MdvxField &curr_field,
	   const int image_delta_secs,
	   DsMdvx &output_mdv_file);
  
  ////////////////////
  // Access methods //
  ////////////////////

  // Retrieve the U vector grid.  Returns a pointer to internal object
  // memory that should NOT be freed by the caller.

  const fl32 *getUGrid(void)
  {
    return _uGrid.getGrid();
  }
  
  // Retrieve the V vector grid.  Returns a pointer to internal object
  // memory that should NOT be freed by the caller.

  const fl32 *getVGrid(void)
  {
    return _vGrid.getGrid();
  }
  
 private:

  /////////////////////
  // Private members //
  /////////////////////

  static const double EPSILON;
  
  // Algorithm parameters

  double _minEcho;
  double _maxEcho;
  bool _trackTopPercentageFlag;
  double _trackTopPercentage;
  bool _trackTopPercentageIncreasing;
  double _maxSpdEcho;
  double _cboxFract;
  double _cboxSize;
  double _cboxSpace;
  double _thrCor;
  GridSearcher<fl32> _cormaxSearcher;
  NoiseGenerator *_noiseGenerator;
  bool _thrvecFlg;
  double _radMean;
  double _thrVec;
  double _thrDiff;
  bool _fillvecFlg;
  double _radVec;
  int _nquadVec;
  double _minVecPts;
  
  vector< grid_pt_t > _correlationPtList;
  
  // Debugging flags

  bool _debugFlag;
  
  bool _printGlobalMeanFlag;
  
  // Flags for gridded debugging output.

  bool _outputCorrelationGrid;
  bool _outputCormaxCountGrid;
  bool _outputOriginalVectors;
  bool _outputLocalMeanGrids;
  bool _outputLocalMeanThreshVectors;
  bool _outputGlobalMeanThreshVectors;
  
  // Grid parameters

  double _minX;
  double _minY;
  double _deltaXKm;
  double _deltaYKm;
  int _nx;
  int _ny;
  
  // Box parameters

  int _vectorXStart;
  int _vectorXEnd;
  int _vectorYStart;
  int _vectorYEnd;
  int _vectorSpacing;
  int _boxNx;
  int _boxNy;
  int _boxXRadius;
  int _numPtsInBox;
  
  // Grid movement information

  double _maxDistEchoGrid;
  int _maxSearchGrid;
  
  // Initial grids

  fl32 *_prevImage;
  fl32 *_currImage;
  
  // Calculated grids

  SimpleGrid<fl32> _uGrid;
  SimpleGrid<fl32> _vGrid;

  SimpleGrid<fl32> _base;
  SimpleGrid<fl32> _corCoefGrid;
  SimpleGrid<fl32> _maxCorrGrid;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Disallow the copy constructor and assignment operator

  CtrecAlg(const CtrecAlg&);
  const CtrecAlg& operator=(const CtrecAlg&);
  
  // Add the cormax count grid to the output file.  This is done for
  // debugging purposes.

  void _addCormaxCountGridToOutputFile(DsMdvx &output_mdv_file,
				       const Mdvx::field_header_t data_field_hdr,
				       const Mdvx::vlevel_header_t data_vlevel_hdr,
				       const SimpleGrid<ui08> &count_grid) const;
  
  // Add the correlation grid to the output file after it is calculated.
  // This is done for debugging purposes.

  void _addCorrGridToOutputFile(DsMdvx &output_mdv_file,
				const Mdvx::field_header_t data_field_hdr,
				const Mdvx::vlevel_header_t data_vlevel_hdr) const;
  
  // Add the individual correlation grid to the output file after it is
  // calculated.  This is done for debugging purposes.

  void _addIndCorrGridToOutputFile(DsMdvx &output_mdv_file,
				   const Mdvx::field_header_t data_field_hdr,
				   const Mdvx::vlevel_header_t data_vlevel_hdr,
				   SimpleGrid<fl32> cor_coef_grid,
				   const double x,
				   const double y) const;
  
  // Add the given local mean grid to the output file. This is done for
  // debugging purposes.

  void _addLocalMeanGridToOutputFile(DsMdvx &output_mdv_file,
				     const string field_name,
				     const Mdvx::field_header_t data_field_hdr,
				     const Mdvx::vlevel_header_t data_vlevel_hdr,
				     const SimpleGrid<fl32> local_mean_grid) const;
  
  // Add the current U and V fields to the output file.  These are added
  // just for debugging purposes.  The final U and V fields are added
  // to the output file by the calling routine after the run() method is
  // called.

  void _addUVToOutputFile(DsMdvx &output_mdv_file,
			  const Mdvx::field_header_t data_field_hdr,
			  const Mdvx::vlevel_header_t data_vlevel_hdr,
			  const string &field_subname,
			  const string &transform);
  
  // Add the given vector component to the output file for debugging.

  void _addVectorCompToOutputFile(DsMdvx &output_mdv_file,
				  const Mdvx::field_header_t data_field_hdr,
				  const Mdvx::vlevel_header_t data_vlevel_hdr,
				  const int field_code,
				  const string &field_name_long,
				  const string &field_name_short,
				  const string &transform,
				  const SimpleGrid<fl32> &field_data) const;
  

  /**********************************************************************
   * _calcCorrCoef() - Calculate the correlation coefficient between the
   *                   base grid and the test grid.
   *
   * Returns true if successful, false if the correlation coefficient
   * couldn't be calculated.
   *
   * Returns the calculated correlation coefficient in the corr_coef
   * parameter.
   */

  double _calcCorrCoef(const SimpleGrid<fl32> &base_grid,
		       const double sum_base,
		       const double sum_base2,
		       const SimpleGrid<fl32> &test_grid,
		       double &corr_coef) const;
  

  // Calculate the ending position for the current grid point.
  // The calculated position is returned in x_end_grid and y_end_grid.
  // The values are in grid coordinates.

  void _calcEndPos(const int cormax_x, const int cormax_y,
		   const int x_min, const int x_max,
		   const int y_min, const int y_max,
		   double &x_end_grid, double &y_end_grid) const;

  // Calculate the current global mean.
  //
  // Returns true if the calculation was successful, false if it failed.
  // Currently, the calculation will only fail if there are no valid vectors.

  bool _calcGlobalMean(double &avgu,
		       double &avgv,
		       int &nvec) const;
  
  // Calculate and display the histogram statistics.

  void _calcHistogramStats(void) const;
  

  /**********************************************************************
   * _createCorrIndexList() - Create the correlation index list.  This is
   *                          the list of points where the calculated
   *                          correlations will be output in a grid for
   *                          debugging purposes.
   */

  void _createCorrIndexList(const MdvxPjg &projection,
			    vector< GridPoint > &corr_index_list);
  

  // The algorithm calculates motion vectors at the given vector spacing.
  // This method fills in the grid spaces in the entire box with the
  // nearest motion vector.

  void _fillBoxes(SimpleGrid<fl32> &filled_grid,
		  const SimpleGrid<fl32> &grid_orig) const;
  
  void _fillBoxes(SimpleGrid<fl32> &u_grid,
		  SimpleGrid<fl32> &v_grid,
		  const SimpleGrid<fl32> &u_grid_orig,
		  const SimpleGrid<fl32> &v_grid_orig);
  
  // Perform a 2-dimensional data filling of a Cartesian grid using a
  // constrained least-squares data fill.

  void _fillVectors(SimpleGrid<fl32> &data, const double max_grid_echo);
  

  /**********************************************************************
   * _findPercentDataValue() - Sort the values in the given grid and find
   *                           the data value at the given percentage
   *                           position from the maximum data value.
   *
   * Returns the data value at the specified percentage location.
   */

  double _findPercentDataValue(const SimpleGrid<fl32> &data_grid,
			       const double top_percent,
			       const bool track_increasing) const;
  

  /**********************************************************************
   * _initializeSubgrid() - Initialize the given subgrid based on the data
   *                        values in the given full grid.
   *
   *                        If the algorithm is set up to track the top
   *                        percentage of data values, this method will
   *                        fill data values below this percentage in the
   *                        subgrid with noise.  The values in the
   *                        original grid are not affected by this noise.
   */

  void _initializeSubgrid(const fl32 *full_grid,
			  const int full_grid_x,
			  const int full_grid_y,
			  SimpleGrid<fl32> &subgrid) const;
  

  // Initialize the given grid to the given value.

//  template<class T> void _initializeGrid(SimpleGrid<T> &grid,
//					 const int nx, const int ny,
//					 const T data_value)
  void _initializeGrid(SimpleGrid<fl32> &grid,
		       const int nx, const int ny,
		       const fl32 data_value)
  {
    for (int i = 0; i < nx * ny; ++i)
    {
      grid.set(i, data_value);
    }
  }


  // Print the global mean values.

  void _printGlobalMean(void) const;
  
  // Set the grid parameters for the algorithm.

  void _setGrid(const MdvxField &field,
		const int image_delta_secs);
  
  // Solve 3 simultaneous linear equations of the form:
  //         r = a0 + a1*x + a2*x*x
  // The caller passes in the r's and the associated x's.
  //
  // Returns the maximum or minimum r value.  Returns the associated
  // x value in xp.

  double _solveLinEq(const double r1, const double r2, const double r3,
		     const double x1, const double x2, const double x3,
		     double &xp) const;                     // output

  // Discard vectors that differ too greatly from the global mean.

  void _thresholdGlobalMean(void);
  
  // Eliminate outlying computed echo motion vectors by comparing them
  //to a local mean.
  
  void _thresholdLocalMean(SimpleGrid<fl32> &data,
			   DsMdvx &output_mdv_file,
			   const string field_name,
			   const Mdvx::field_header_t data_field_hdr,
			   const Mdvx::vlevel_header_t data_vlevel_hdr);
  
  // Do the echo tracking by finding local maxima in the correlation
  // function.

  void _trackEchoes(const MdvxField &prev_field,
		    const MdvxField &curr_field,
		    const int image_delta_secs,
		    DsMdvx &output_mdv_file);
  
};


#endif
