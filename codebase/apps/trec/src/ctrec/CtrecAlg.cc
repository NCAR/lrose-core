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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:28:57 $
//   $Id: CtrecAlg.cc,v 1.26 2016/03/06 23:28:57 dixon Exp $
//   $Revision: 1.26 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CtrecAlg.cc: class implementing the Cartesian TREC algorithm.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <math.h>

#include <toolsa/os_config.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <dataport/port_types.h>
#include <euclid/GridPoint.hh>
#include <rapmath/math_macros.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>

#include "CtrecAlg.hh"
#include "GridSearcher.hh"
#include "SimpleGrid.hh"
using namespace std;


/**********************************************************************
 * Assign static constant values.
 */

const double CtrecAlg::BAD_OUTPUT_VALUE = -999.0;
const ui08 CtrecAlg::BAD_COUNT_VALUE = 255;
  
const double CtrecAlg::EPSILON = 0.0001;
  

/**********************************************************************
 * Constructor
 */

CtrecAlg::CtrecAlg(const double min_echo,
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
		   const bool debug_flag,
		   const bool print_global_mean,
		   const bool output_correlation_grid,
		   const bool output_cormax_count_grid,
		   const bool output_original_vectors,
		   const bool output_local_mean_grids,
		   const bool output_local_mean_thresh_vectors,
		   const bool output_global_mean_thresh_vectors) :
  _minEcho(min_echo),
  _maxEcho(max_echo),
  _trackTopPercentageFlag(track_top_percentage_flag),
  _trackTopPercentage(track_top_percentage),
  _trackTopPercentageIncreasing(track_top_percentage_increasing),
  _maxSpdEcho(max_spd_echo),
  _cboxFract(cbox_fract),
  _cboxSize(cbox_size),
  _cboxSpace(cbox_space),
  _thrCor(thr_cor * 100.0),
  _cormaxSearcher(cormax_searcher),
  _noiseGenerator(noise_generator),
  _thrvecFlg(thresh_vectors_flag),
  _radMean(rad_mean),
  _thrVec(thr_vec),
  _thrDiff(thr_diff),
  _fillvecFlg(fill_vectors_flag),
  _radVec(rad_vec),
  _nquadVec(num_quad_vec),
  _minVecPts(min_vec_pts),

  _correlationPtList(correlation_loc_list),

  _debugFlag(debug_flag),
  _printGlobalMeanFlag(print_global_mean),

  _outputCorrelationGrid(output_correlation_grid),
  _outputCormaxCountGrid(output_cormax_count_grid),
  _outputOriginalVectors(output_original_vectors),
  _outputLocalMeanGrids(output_local_mean_grids),
  _outputLocalMeanThreshVectors(output_local_mean_thresh_vectors),
  _outputGlobalMeanThreshVectors(output_global_mean_thresh_vectors),

  _vectorXStart(0),
  _vectorXEnd(0),
  _vectorYStart(0),
  _vectorYEnd(0),
  _vectorSpacing(1),

  _maxDistEchoGrid(0.0),
  _maxSearchGrid(0),

  _prevImage(0),
  _currImage(0),

  _uGrid(1, 1),
  _vGrid(1, 1),
  _base(1, 1),
  _corCoefGrid(1, 1),
  _maxCorrGrid(1, 1)
{
  assert(noise_generator != 0);
}


/**********************************************************************
 * Destructor
 */

CtrecAlg::~CtrecAlg(void)
{
  // Do nothing
}
  

/**********************************************************************
 * run() - Run the algorithm on the given grids.
 *
 * Note that if this class is moved to a library, the run() method
 * will have to check to make sure the grids used by the two fields
 * match.  It will also have to call _setGrid() whenever the grid
 * changes.  The Ctrec class makes sure the grids for all images
 * match exactly.
 *
 * Returns true if the algorithm was successful, false otherwise.
 */

bool CtrecAlg::run(const MdvxField &prev_field,
		   const MdvxField &curr_field,
		   const int image_delta_secs,
		   DsMdvx &output_mdv_file)
{
  static const string method_name = "CtrecAlg::run()";
  
  // Make sure the data is in floats

  if (prev_field.getFieldHeader().encoding_type != Mdvx::ENCODING_FLOAT32 ||
      curr_field.getFieldHeader().encoding_type != Mdvx::ENCODING_FLOAT32)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Encoding type for both fields must be ENCODING_FLOAT32" << endl;
    
    return false;
  }
  
  // Make sure the setGrid() method was called

  _setGrid(curr_field, image_delta_secs);
  
  // Print out some values for debugging

  if (_debugFlag)
  {
    cout << "nx,ny are: " << _nx << " " << _ny << endl;
    cout << " thrvec_flg " << _thrvecFlg << endl;
    cout << " fillvec_flg " << _fillvecFlg << endl;
    cout << " nquad_vec " << _nquadVec << endl;
    cout << " min_echo " << _minEcho << endl;
    cout << " cbox_size " << _cboxSize << endl;
    cout << " cbox_space " << _cboxSpace << endl;
    cout << " max_spd_echo " << _maxSpdEcho << endl;
    cout << " cbox_fract" << _cboxFract << endl;
    cout << " thr_cor " << _thrCor << endl;
    cout << " thr_diff " << _thrDiff << endl;
    cout << " rad_mean " << _radMean << endl;
    cout << " thr_vec " << _thrVec << endl;
    cout << " rad_vec " << _radVec << endl;
    cout << " min_vec_pts " << _minVecPts << endl;
    cout << " debug is  T" << endl;
  }
  
  if (_debugFlag)
    cout << "grid minimums are: " << _minX << " " <<
      _minY << endl;
    
  // Do the correlation analysis

  PMU_auto_register("Tracking echos");
  
  _trackEchoes(prev_field, curr_field, image_delta_secs,
	       output_mdv_file);
  
  // Print the global mean if requested

  if (_printGlobalMeanFlag)
    _printGlobalMean();
  
  // Throw away vectors that differ too much from the local mean

  PMU_auto_register("Thresholding based on local means");
  
  if (_thrvecFlg)
  {
    _thresholdLocalMean(_uGrid,
			output_mdv_file,
			"U",
			curr_field.getFieldHeader(),
			curr_field.getVlevelHeader());

    _thresholdLocalMean(_vGrid,
			output_mdv_file,
			"V",
			curr_field.getFieldHeader(),
			curr_field.getVlevelHeader());

    if (_outputLocalMeanThreshVectors)
      _addUVToOutputFile(output_mdv_file,
			 curr_field.getFieldHeader(),
			 curr_field.getVlevelHeader(),
			 "thr local",
			 "After local mean thresholding");
    
  }
  
  // Throw away vectors that differ too much from the global mean

  PMU_auto_register("Thresholding based on global mean");
  
  _thresholdGlobalMean();
  
  if (_outputGlobalMeanThreshVectors)
    _addUVToOutputFile(output_mdv_file,
		       curr_field.getFieldHeader(),
		       curr_field.getVlevelHeader(),
		       "thr global",
		       "After global mean thresholding");
  
  // Calculate histogram statistics

  PMU_auto_register("Calculating historgram statistics");
  
  _calcHistogramStats();
  
  // Fill in vector fields if requested

  PMU_auto_register("Filling vectors");
  
  if (_fillvecFlg)
  {
    if (_debugFlag)
      cout << "filling vectors... " << endl;

    _fillVectors(_uGrid, _maxSearchGrid);
    _fillVectors(_vGrid, _maxSearchGrid);
  }
  
  if (_debugFlag)
   cout << "trecnids: done with tkcomp" << endl;
  
  // The algorithm calculates motion vectors at the given vector spacing.
  // Fill in the grid spaces in the entire box with the average motion vector.

  SimpleGrid<fl32> u_grid_copy(_uGrid);
  SimpleGrid<fl32> v_grid_copy(_vGrid);
  
  _fillBoxes(_uGrid, _vGrid,
	     u_grid_copy, v_grid_copy);
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addCormaxCountGridToOutputFile() - Add the cormax count grid to the
 *                                     output file.  This is done for
 *                                     debugging purposes.
 */

void CtrecAlg::_addCormaxCountGridToOutputFile(DsMdvx &output_mdv_file,
					       const Mdvx::field_header_t data_field_hdr,
					       const Mdvx::vlevel_header_t data_vlevel_hdr,
					       const SimpleGrid<ui08> &count_grid) const
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.nx = data_field_hdr.nx;
  field_hdr.ny = data_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = data_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_INT8;
  field_hdr.data_element_nbytes = 1;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.dz_constant = data_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = data_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = data_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    field_hdr.proj_param[i] = data_field_hdr.proj_param[i];
  field_hdr.vert_reference = data_field_hdr.vert_reference;
  field_hdr.grid_dx = data_field_hdr.grid_dx;
  field_hdr.grid_dy = data_field_hdr.grid_dy;
  field_hdr.grid_dz = data_field_hdr.grid_dz;
  field_hdr.grid_minx = data_field_hdr.grid_minx;
  field_hdr.grid_miny = data_field_hdr.grid_miny;
  field_hdr.grid_minz = data_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = BAD_COUNT_VALUE;
  field_hdr.missing_data_value = BAD_COUNT_VALUE;
  field_hdr.proj_rotation = data_field_hdr.proj_rotation;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  
  STRcopy(field_hdr.field_name_long, "cormax count",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "cormax count",
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "number", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the new field.

  MdvxField *count_field = new MdvxField(field_hdr,
					 data_vlevel_hdr,
					 count_grid.getGrid());
  
  // Compress the data.

  count_field->convertType(Mdvx::ENCODING_INT8,
			   Mdvx::COMPRESSION_GZIP);
  
  // Add the field to the output file

  output_mdv_file.addField(count_field);
  
}


/**********************************************************************
 * _addCorrGridToOutputFile() - Add the correlation grid to the output
 *                              file after it is calculated.  This is
 *                              done for debugging purposes.
 */

void CtrecAlg::_addCorrGridToOutputFile(DsMdvx &output_mdv_file,
					const Mdvx::field_header_t data_field_hdr,
					const Mdvx::vlevel_header_t data_vlevel_hdr) const
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.nx = data_field_hdr.nx;
  field_hdr.ny = data_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = data_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.dz_constant = data_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = data_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = data_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    field_hdr.proj_param[i] = data_field_hdr.proj_param[i];
  field_hdr.vert_reference = data_field_hdr.vert_reference;
  field_hdr.grid_dx = data_field_hdr.grid_dx;
  field_hdr.grid_dy = data_field_hdr.grid_dy;
  field_hdr.grid_dz = data_field_hdr.grid_dz;
  field_hdr.grid_minx = data_field_hdr.grid_minx;
  field_hdr.grid_miny = data_field_hdr.grid_miny;
  field_hdr.grid_minz = data_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = BAD_OUTPUT_VALUE;
  field_hdr.missing_data_value = BAD_OUTPUT_VALUE;
  field_hdr.proj_rotation = data_field_hdr.proj_rotation;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  
  STRcopy(field_hdr.field_name_long, "Max correlation coefficients",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "max corr",
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "percent", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the new field.

  MdvxField *corr_field = new MdvxField(field_hdr,
					data_vlevel_hdr,
					_maxCorrGrid.getGrid());
  
  // Compress the data.

  corr_field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP);
  
  // Add the field to the output file

  output_mdv_file.addField(corr_field);
  
}


/**********************************************************************
 * _addIndCorrGridToOutputFile() - Add the individual correlation grid
 *                                 to the output file after it is
 *                                 calculated.  This is done for debugging
 *                                 purposes.
 */

void CtrecAlg::_addIndCorrGridToOutputFile(DsMdvx &output_mdv_file,
					   const Mdvx::field_header_t data_field_hdr,
					   const Mdvx::vlevel_header_t data_vlevel_hdr,
					   SimpleGrid<fl32> cor_coef_grid,
					   const double x,
					   const double y) const
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.nx = data_field_hdr.nx;
  field_hdr.ny = data_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = data_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.dz_constant = data_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = data_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = data_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    field_hdr.proj_param[i] = data_field_hdr.proj_param[i];
  field_hdr.vert_reference = data_field_hdr.vert_reference;
  field_hdr.grid_dx = data_field_hdr.grid_dx;
  field_hdr.grid_dy = data_field_hdr.grid_dy;
  field_hdr.grid_dz = data_field_hdr.grid_dz;
  field_hdr.grid_minx = data_field_hdr.grid_minx;
  field_hdr.grid_miny = data_field_hdr.grid_miny;
  field_hdr.grid_minz = data_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = BAD_OUTPUT_VALUE;
  field_hdr.missing_data_value = BAD_OUTPUT_VALUE;
  field_hdr.proj_rotation = data_field_hdr.proj_rotation;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  
  sprintf(field_hdr.field_name_long, "%7.2f, %7.2f corr coeffs", x, y);
  STRcopy(field_hdr.field_name, "Corr Coefs",
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "percent", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the new field.

  MdvxField *corr_field = new MdvxField(field_hdr,
					data_vlevel_hdr,
					cor_coef_grid.getGrid());
  
  // Compress the data.

  corr_field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP);
  
  // Add the field to the output file

  output_mdv_file.addField(corr_field);
  
}


/**********************************************************************
 * _addLocalMeanGridToOutputFile() - Add the given local mean grid to
 *                                   the output file. This is done for
 *                                   debugging purposes.
 */

void CtrecAlg::_addLocalMeanGridToOutputFile(DsMdvx &output_mdv_file,
					     const string field_name,
					     const Mdvx::field_header_t data_field_hdr,
					     const Mdvx::vlevel_header_t data_vlevel_hdr,
					     const SimpleGrid<fl32> local_mean_grid) const
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.nx = data_field_hdr.nx;
  field_hdr.ny = data_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = data_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.dz_constant = data_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = data_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = data_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    field_hdr.proj_param[i] = data_field_hdr.proj_param[i];
  field_hdr.vert_reference = data_field_hdr.vert_reference;
  field_hdr.grid_dx = data_field_hdr.grid_dx;
  field_hdr.grid_dy = data_field_hdr.grid_dy;
  field_hdr.grid_dz = data_field_hdr.grid_dz;
  field_hdr.grid_minx = data_field_hdr.grid_minx;
  field_hdr.grid_miny = data_field_hdr.grid_miny;
  field_hdr.grid_minz = data_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = BAD_OUTPUT_VALUE;
  field_hdr.missing_data_value = BAD_OUTPUT_VALUE;
  field_hdr.proj_rotation = data_field_hdr.proj_rotation;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  
  string field_name_long = field_name + " local means";
  
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, data_field_hdr.units, MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the new field.

  SimpleGrid<fl32> filled_local_mean_grid(local_mean_grid);

  _fillBoxes(filled_local_mean_grid, local_mean_grid);
  
  MdvxField *local_mean_field = new MdvxField(field_hdr,
					      data_vlevel_hdr,
					      filled_local_mean_grid.getGrid());
  
  // Compress the data.

  local_mean_field->convertType(Mdvx::ENCODING_INT8,
				Mdvx::COMPRESSION_GZIP);
  
  // Add the field to the output file

  output_mdv_file.addField(local_mean_field);
  
}


/**********************************************************************
 * _addUVToOutputFile() - Add the current U and V fields to the output
 *                        file.  These are added just for debugging
 *                        purposes.  The final U and V fields are added
 *                        to the output file by the calling routine
 *                        after the run() method is called.
 */

void CtrecAlg::_addUVToOutputFile(DsMdvx &output_mdv_file,
				  const Mdvx::field_header_t data_field_hdr,
				  const Mdvx::vlevel_header_t data_vlevel_hdr,
				  const string &field_subname,
				  const string &transform)
{
  // Copy the data so we can fill in the boxes

  SimpleGrid<fl32> u_data(_uGrid);
  SimpleGrid<fl32> v_data(_vGrid);
  
  // Fill in the boxes

  _fillBoxes(u_data, v_data,
	     _uGrid, _vGrid);
  
  // Output the fields

  _addVectorCompToOutputFile(output_mdv_file,
			     data_field_hdr,
			     data_vlevel_hdr,
			     33,
			     "U wind comp " + field_subname,
			     "U " + field_subname,
			     transform,
			     u_data);
  
  _addVectorCompToOutputFile(output_mdv_file,
			     data_field_hdr,
			     data_vlevel_hdr,
			     34,
			     "V wind comp " + field_subname,
			     "V " + field_subname,
			     transform,
			     v_data);
  
}


/**********************************************************************
 * _addVectorCompToOutputFile() - Add the given vector component to the
 *                                output file for debugging.
 */

void CtrecAlg::_addVectorCompToOutputFile(DsMdvx &output_mdv_file,
					  const Mdvx::field_header_t data_field_hdr,
					  const Mdvx::vlevel_header_t data_vlevel_hdr,
					  const int field_code,
					  const string &field_name_long,
					  const string &field_name_short,
					  const string &transform,
					  const SimpleGrid<float> &field_data) const
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = field_code;
  field_hdr.nx = data_field_hdr.nx;
  field_hdr.ny = data_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = data_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.vlevel_type = data_field_hdr.vlevel_type;
  field_hdr.dz_constant = data_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = data_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = data_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    field_hdr.proj_param[i] = data_field_hdr.proj_param[i];
  field_hdr.vert_reference = data_field_hdr.vert_reference;
  field_hdr.grid_dx = data_field_hdr.grid_dx;
  field_hdr.grid_dy = data_field_hdr.grid_dy;
  field_hdr.grid_dz = data_field_hdr.grid_dz;
  field_hdr.grid_minx = data_field_hdr.grid_minx;
  field_hdr.grid_miny = data_field_hdr.grid_miny;
  field_hdr.grid_minz = data_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = BAD_OUTPUT_VALUE;
  field_hdr.missing_data_value = BAD_OUTPUT_VALUE;
  field_hdr.proj_rotation = data_field_hdr.proj_rotation;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name_short.c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "m/sec", MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, transform.c_str(), MDV_TRANSFORM_LEN);
  
  // Create the new field.

  MdvxField *vector_field = new MdvxField(field_hdr,
					  data_vlevel_hdr,
					  field_data.getGrid());
  
  // Compress the data.

  vector_field->convertType(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_GZIP);
  
  // Add the field to the output file

  output_mdv_file.addField(vector_field);
  
}


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

double CtrecAlg::_calcCorrCoef(const SimpleGrid<fl32> &base_grid,
			       const double sum_base,
			       const double sum_base2,
			       const SimpleGrid<fl32> &test_grid,
			       double &corr_coef) const
{
  // Calculate the sums needed for the correlation coefficient
  // calculation.

  double sum_test = 0.0;
  double sum_test2 = 0.0;
  double sum_base_test = 0.0;
	  
  for (int i = 0; i < _numPtsInBox; ++i)
  {
    double test_value = test_grid.get(i);
    
    sum_test += test_value;
    sum_test2 += (test_value * test_value);
    sum_base_test +=
      base_grid.get(i) * test_value;
  } /* endfor - x */
	  
  // Calculate the correlation coefficient.  Note that the variance
  // and covariance values are the true values multiplied by
  // _numPtsInBox to reduce the calculations needed for each.
  // Mathematically, base_variance should actually be:
  //
  //     sum_base2/_numPtsInBox - 
  //            (sum_base*sum_base)/(_numPtsInBox*_numPtsInBox)
  //
  // When we get to the correlation coefficient calculation, the
  // _numPtsInBox values in the numerator and denominator cancel
  // each other out.

  double base_variance = sum_base2 -
    (sum_base * sum_base / (double)_numPtsInBox);
  double test_variance = sum_test2 -
    (sum_test * sum_test / (double)_numPtsInBox);
  double covariance = sum_base_test -
    (sum_base * sum_test / (double)_numPtsInBox);
	  
  if (base_variance <= 0.001 || test_variance <= 0.001)
    return false;
  
  corr_coef = covariance / sqrt(base_variance * test_variance);
  
  return true;
}


/**********************************************************************
 * _calcEndPos() - Calculate the ending position for the current grid
 *                 point.  The calculated position is returned in
 *                 x_end_grid and y_end_grid.  The values are in grid
 *                 coordinates.
 */

void CtrecAlg::_calcEndPos(const int cormax_x, const int cormax_y,
			   const int x_min, const int x_max,
			   const int y_min, const int y_max,
			   double &x_end_grid, double &y_end_grid) const
{
  double r[3];
  double x[3];
  double rmax;
  double xp;

  // Initialize the return values

  x_end_grid = cormax_x;
  y_end_grid = cormax_y;
  
  // Estimate the X value of the endpoint

  if (cormax_x > x_min && cormax_x < x_max)
  {
    r[0] = _corCoefGrid.get(cormax_x - 1, cormax_y);
    r[1] = _corCoefGrid.get(cormax_x, cormax_y);
    r[2] = _corCoefGrid.get(cormax_x + 1, cormax_y);
      
    x[0] = cormax_x - 1;
    x[1] = cormax_x;
    x[2] = cormax_x + 1;

    if (r[0] != BAD_OUTPUT_VALUE &&
	r[1] != BAD_OUTPUT_VALUE &&
	r[2] != BAD_OUTPUT_VALUE)
    {
      rmax = _solveLinEq(r[0], r[1], r[2], x[0], x[1], x[2], xp);
  
      if (rmax > _corCoefGrid.get(cormax_x, cormax_y) &&
	  xp > x[0] && xp < x[2])
	x_end_grid = xp;
    }
  }
  
  // Estimate the Y value of the endpoint

  if (cormax_y > y_min && cormax_y < y_max)
  {
    r[0] = _corCoefGrid.get(cormax_x, cormax_y - 1);
    r[1] = _corCoefGrid.get(cormax_x, cormax_y);
    r[2] = _corCoefGrid.get(cormax_x, cormax_y + 1);
      
    x[0] = cormax_y - 1;
    x[1] = cormax_y;
    x[2] = cormax_y + 1;

    if (r[0] != BAD_OUTPUT_VALUE &&
	r[1] != BAD_OUTPUT_VALUE &&
	r[2] != BAD_OUTPUT_VALUE)
    {
      rmax = _solveLinEq(r[0], r[1], r[2], x[0], x[1], x[2], xp);
  
      if (rmax > _corCoefGrid.get(cormax_x, cormax_y) &&
	  xp > x[0] && xp < x[2])
	y_end_grid = xp;
    }
  }
  
}


/**********************************************************************
 * _calcGlobalMean() - Calculate the current global mean.
 *
 * Returns true if the calculation was successful, false if it failed.
 * Currently, the calculation will only fail if there are no valid vectors.
 */

bool CtrecAlg::_calcGlobalMean(double &avgu,
			       double &avgv,
			       int &nvec) const
{
  double sumu = 0.0;
  double sumv = 0.0;
  nvec = 0;
    
  for (int index = 0; index < _nx * _ny; ++index)
  {
    if (_uGrid.get(index) == BAD_OUTPUT_VALUE ||
	_vGrid.get(index) == BAD_OUTPUT_VALUE)
      continue;
      
    if (fabs(_uGrid.get(index)) > EPSILON ||
	fabs(_vGrid.get(index)) > EPSILON)
    {
      sumu += _uGrid.get(index);
      sumv += _vGrid.get(index);
      nvec++;
    }
      
  }
    
  if (nvec <= 0)
    return false;
  
  avgu = sumu / (double)nvec;
  avgv = sumv / (double)nvec;
  
  return true;
}


/**********************************************************************
 * _calcHistogramStats() - Calculate and display the histogram statistics.
 */

void CtrecAlg::_calcHistogramStats(void) const
{
  // Only do this if we are in debug mode

  if (_debugFlag)
  {
    // Calculate the histogram statistics

    double avgu, avgv;
    int nvec;
    
    if (!_calcGlobalMean(avgu, avgv, nvec))
      return;
    
    cout << "Histogram: n, avgu, avgv = " << nvec << " " <<
      avgu << " " << avgv << endl;
  }
  
}


/**********************************************************************
 * _createCorrIndexList() - Create the correlation index list.  This is
 *                          the list of points where the calculated
 *                          correlations will be output in a grid for
 *                          debugging purposes.
 */

void CtrecAlg::_createCorrIndexList(const MdvxPjg &projection,
				    vector< GridPoint > &corr_index_list)
{

  vector< grid_pt_t >::iterator grid_iter;
  
  for (grid_iter = _correlationPtList.begin();
       grid_iter != _correlationPtList.end(); ++grid_iter)
  {
    int x_index, y_index;
    
    if (projection.xy2xyIndex(grid_iter->x, grid_iter->y,
			      x_index, y_index) != 0)
    {
      cerr << "*** Point outside of grid: " << grid_iter->x <<
	", " << grid_iter->y << endl;
      continue;
    }
    
    GridPoint index_pt(x_index, y_index);

    cerr << "*** Point " << grid_iter->x << ", " << grid_iter->y <<
      " is at index " << index_pt.x << ", " << index_pt.y << endl;
    
    corr_index_list.push_back(index_pt);
  } /* endfor - grid_iter */
  
}


/**********************************************************************
 * _fillBoxes() - The algorithm calculates motion vectors at the given
 *                vector spacing.  This method fills in the grid spaces
 *                in the entire box with the nearest motion vector.
 */

void CtrecAlg::_fillBoxes(SimpleGrid<fl32> &filled_grid,
			  const SimpleGrid<fl32> &grid_orig) const
{
  int fill_range = _vectorSpacing / 2;
  
  if (_debugFlag)
    cout << "fill_range= " << fill_range << endl;
  
  for (int x = fill_range; x < _nx - fill_range; x++)
  {
    for (int y = fill_range; y < _ny - fill_range; y++)
    {
      if (grid_orig.get(x, y) != BAD_OUTPUT_VALUE)
      {
	for (int xrng = -fill_range; xrng <= fill_range; xrng++)
	{
	  for (int yrng = -fill_range; yrng <= fill_range; yrng++)
	    filled_grid.set(x + xrng, y + yrng,
			    grid_orig.get(x, y));


	} /* endfor - xrng */

      } /* endif - valid data values */

    } /* endfor - y */
  } /* endfor - x */
}


void CtrecAlg::_fillBoxes(SimpleGrid<fl32> &u_grid, SimpleGrid<fl32> &v_grid,
			  const SimpleGrid<fl32> &u_grid_orig,
			  const SimpleGrid<fl32> &v_grid_orig)
{
  int fill_range = _vectorSpacing / 2;
  
  if (_debugFlag)
    cout << "fill_range= " << fill_range << endl;
  
  for (int x = fill_range; x < _nx - fill_range; x++)
  {
    for (int y = fill_range; y < _ny - fill_range; y++)
    {
      if (u_grid_orig.get(x + 1, y + 1) != BAD_OUTPUT_VALUE &&
	  v_grid_orig.get(x + 1, y + 1) != BAD_OUTPUT_VALUE)
      {
	for (int xrng = -fill_range; xrng <= fill_range; xrng++)
	{
	  for (int yrng = -fill_range; yrng <= fill_range; yrng++)
	  {
	    u_grid.set(x + xrng, y + yrng,
		       u_grid_orig.get(x + 1, y + 1));
	    v_grid.set(x + xrng, y + yrng,
		       v_grid_orig.get(x + 1, y + 1));

	  } /* endfor - yrng */
	} /* endfor - xrng */

      } /* endif - valid data values */

    } /* endfor - y */
  } /* endfor - x */
}


/**********************************************************************
 * _fillVectors() - Perform a 2-dimensional data filling of a Cartesian
 *                  grid using a constrained least-squares data fill.
 */

void CtrecAlg::_fillVectors(SimpleGrid<fl32> &data,
			    const double max_grid_echo)
{
  bool quad_filled[4];
  int num_quads_filled;
  
  int max_box_radius;   // Maximum number of grid points to search
                        // outward from a missing data point to determine
                        // if it is bounded and if _minDbzPts has been
                        // satisfied.

  int box_minx, box_maxx;
  int box_miny, box_maxy;
  
  int x_dist, y_dist;
  
  int num_pts;
  double sum_x, sum_y, sum_x2, sum_y2, sum_xy;
  double sum_data, sum_data_x, sum_data_y;
  double t1, t2, t3;
  double denom, numerator;
  
  if (_debugFlag)
    cout << "...........filling data.............." << endl;
  
  max_box_radius = (int)(_radVec / _deltaXKm);
  if (max_box_radius <= 0)
    max_box_radius = 1;
  
  // Loop over grid points.  If grid point value is missing, fill it.
  // Otherwise, do nothing.

  for (int x = (int)(max_grid_echo + _boxXRadius);
       x <= (int)(_nx - max_grid_echo - _boxXRadius - 1); x += _vectorSpacing)
  {
    for (int y = (int)(max_grid_echo + _boxXRadius);
	 y <= (int)(_ny - max_grid_echo - _boxXRadius - 1);
	 y += _vectorSpacing)
    {
      if (data.get(x, y) != BAD_OUTPUT_VALUE)
	continue;
      
      for (int quad_num = 0; quad_num < 4; ++quad_num)
	quad_filled[quad_num] = false;
      
      // Start searching outward from grid point until the _nquadVec
      // and _minDbzPts conditions are met.

      for (int box_radius = 1; box_radius <= max_box_radius; ++box_radius)
      {
	box_miny = MAX(0, y - box_radius);
	box_maxy = MIN(_ny - 1, y + box_radius);
	box_minx = MAX(0, x - box_radius);
	box_maxx = MIN(_nx - 1, x + box_radius);
	
	// Initialize summation terms used in the least-squares fit

	num_pts = 0;
	sum_x = 0.0;
	sum_y = 0.0;
	sum_x2 = 0.0;
	sum_y2 = 0.0;
	sum_xy = 0.0;
	sum_data_x = 0.0;
	sum_data_y = 0.0;
	sum_data = 0.0;
	
	for (int xx = box_minx; xx <= box_maxx; ++xx)
	{
	  x_dist = xx - x;
	    
	  for (int yy = box_miny; yy <= box_maxy; ++yy)
	  {
	    y_dist = yy - y;
	  
	    if (data.get(xx, yy) == BAD_OUTPUT_VALUE)
	      continue;
	    
	    if (x_dist >= 0 && y_dist > 0)
	      quad_filled[0] = true;
	    else if (x_dist > 0 && y_dist <= 0)
	      quad_filled[1] = true;
	    else if (x_dist <= 0 && y_dist < 0)
	      quad_filled[2] = true;
	    else if (x_dist < 0 && y_dist >= 0)
	      quad_filled[3] = true;
	    
	    num_pts++;
	    sum_x += x_dist;
	    sum_y += y_dist;
	    sum_x2 += (x_dist * x_dist);
	    sum_y2 += (y_dist * y_dist);
	    sum_xy += (x_dist * y_dist);
	    sum_data += data.get(xx, yy);
	    sum_data_x += (x_dist * data.get(xx, yy));
	    sum_data_y += (y_dist * data.get(xx, yy));
	    
	  } /* endfor - yy */
	} /* endfor - xx */
	
	num_quads_filled = 0;
	for (int quad_num = 0; quad_num < 4; quad_num++)
	  if (quad_filled[quad_num])
	    num_quads_filled ++;
	
	if (num_quads_filled < _nquadVec)
	  continue;
	
	if (num_pts < _minVecPts)
	  continue;
	
	t1 = (sum_x2 * sum_y2) - (sum_xy * sum_xy);
	t2 = (sum_x * sum_y2) - (sum_xy * sum_y);
	t3 = (sum_x * sum_xy) - (sum_x2 * sum_y);
	
	denom = ((double)num_pts * t1) - (sum_x * t2) + (sum_y * t3);
	
	if (fabs(denom) <= EPSILON)
	{
	  cout << "denom < EPS, i,j,kq,denom: " <<
	    x << " " << y << " " << num_quads_filled << " " << denom << endl;
	  continue;
	}
	
	numerator = (sum_data * t1) - (sum_data_x * t2) + (sum_data_y * t3);
	
	data.set(x, y, numerator / denom);
	
	break;
	
      } /* endfor - box_radius */
      
    } /* endfor - y */
  } /* endfor - x */
    
}


/**********************************************************************
 * _findPercentDataValue() - Sort the values in the given grid and find
 *                           the data value at the given percentage
 *                           position from the maximum data value.
 *
 * Returns the data value at the specified percentage location.
 */

double CtrecAlg::_findPercentDataValue(const SimpleGrid<fl32> &data_grid,
				       const double top_percent,
				       const bool track_increasing) const
{
  // First sort the data values

  vector< double > data_values;
  int num_data_values = data_grid.getNx() * data_grid.getNy();
  int num_good_values = 0;
  
  for (int i = 0; i < num_data_values; ++i)
  {
    double data_value = data_grid.get(i);
    
    if (data_value >= _minEcho &&
	data_value <= _maxEcho)
    {
      data_values.push_back(data_value);
      ++num_good_values;
    }
  }
  
  sort(data_values.begin(), data_values.end());
  
  // Now find the index for the desired data value

  if (num_good_values <= 0)
    return _minEcho;
  
  int data_index;
  
  if (track_increasing)
    data_index = num_good_values -
      (int)((double)num_good_values * top_percent);
  else
    data_index = (int)((double)num_good_values * top_percent);
  
  if (data_index < 0)
    data_index = 0;
  
  if (data_index >= num_good_values)
    data_index = num_good_values - 1;
  
  return data_values[data_index];
}


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

void CtrecAlg::_initializeSubgrid(const fl32 *full_grid,
				  const int full_grid_x,
				  const int full_grid_y,
				  SimpleGrid<fl32> &subgrid) const
{
  // First, copy the original data into the subgrid

  for (int x = full_grid_x - _boxXRadius, subgrid_x = 0;
       x <= full_grid_x + _boxXRadius;
       ++x, ++subgrid_x)
  {
    for (int y = full_grid_y - _boxXRadius, subgrid_y = 0;
	 y <= full_grid_y + _boxXRadius;
	 ++y, ++subgrid_y)
    {
      int data_index = x + (_nx * y);
	  
      subgrid.set(subgrid_x, subgrid_y, full_grid[data_index]);
    } /* endfor - y */
  } /* endfor - x */

  // Next, see if we are only tracking the top data values.  If so,
  // find the minimum data value to track and fill in the rest of the
  // data with noise.  Note that data outside of the range from
  // _minEcho to _maxEcho is already filled with noise so we don't
  // have to calculate noise values for these.

  if (_trackTopPercentageFlag)
  {
    double percent_data_value =
      _findPercentDataValue(subgrid,
			    _trackTopPercentage,
			    _trackTopPercentageIncreasing);
    
    if (_trackTopPercentageIncreasing)
    {
      for (int i = 0; i < _numPtsInBox; ++i)
	if (subgrid.get(i) < percent_data_value &&
	    subgrid.get(i) >= _minEcho)
	  subgrid.set(i, _noiseGenerator->getNoiseValue());
    }
    else
    {
      for (int i = 0; i < _numPtsInBox; ++i)
	if (subgrid.get(i) > percent_data_value &&
	    subgrid.get(i) <= _maxEcho)
	  subgrid.set(i, _noiseGenerator->getNoiseValue());
    }
  }
}


/**********************************************************************
 * _printGlobalMean() - Print the global mean values.
 */

void CtrecAlg::_printGlobalMean(void) const
{
  // Calculate the global mean
  
  double avgu;
  double avgv;
  int nvec;
    
  if (!_calcGlobalMean(avgu, avgv, nvec))
  {
    cout << "Cannot print global mean -- no vectors" << endl;
    return;
  }
  
  // Calculate the average direction and average speed.

  double avgdir, avgspd;
  
  if (avgu == 0.0)
  {
    if (avgv == 0.0)
      avgdir = 0.0;
    else if (avgv < 0.0)
      avgdir = -0.5 * PI;
    else
      avgdir = 0.5 * PI;
  }
  else if (avgv == 0.0)
  {
    if (avgu < 0.0)
      avgdir = PI;
    else
      avgdir = 0.0;
  }
  else
  {
    avgdir = atan2(avgv, avgu);
  }
      
  avgdir *= (180.0 / PI);
  avgdir = 90.0 - avgdir;
      
  while (avgdir < 0.0)
    avgdir += 360.0;
      
  avgspd = sqrt((avgu * avgu) + (avgv * avgv));
      
  cout << "Avg U = " << avgu << endl;
  cout << "Avg V = " << avgv << endl;
  cout << "Avg dir = " << avgdir << endl;
  cout << "Avg speed = " << avgspd << endl;
}


/**********************************************************************
 * _setGrid() - Set the grid parameters for the algorithm.
 */

void CtrecAlg::_setGrid(const MdvxField &field,
			const int image_delta_secs)
{
  static const string method_name = "CtrecAlg::_setGrid()";
  
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  
  // Set the grid limits

  _minX = field_hdr.grid_minx;
  _minY = field_hdr.grid_miny;
  _nx = field_hdr.nx;
  _ny = field_hdr.ny;

  // Set up the correlation box stuff

  switch(field_hdr.proj_type)
  {
  case Mdvx::PROJ_FLAT :
    _deltaXKm = field_hdr.grid_dx;
    _deltaYKm = field_hdr.grid_dy;
    break;
    
  case Mdvx::PROJ_LATLON :
    _deltaXKm = field_hdr.grid_dx * KM_PER_DEG_AT_EQ;
    _deltaYKm = field_hdr.grid_dy * KM_PER_DEG_AT_EQ;
    break;
    
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Program can only handle FLAT and LATLON projections" << endl;
    
    exit(-1);
    break;
  }
  
  _boxNx = (int)((_cboxSize / _deltaXKm) + 1.0001);
  if (_boxNx % 2 == 0)
    ++_boxNx;
  
  _boxNy = _boxNx;
  
  _boxXRadius = _boxNx / 2;

  _numPtsInBox = _boxNx * _boxNy;
  
  // Determine the maximum amount that the echo could have moved

  _maxDistEchoGrid =
    (0.001 * _maxSpdEcho * (double)image_delta_secs) / _deltaXKm;
  
  // This max_search_grid determines how far to search, in grid squares,
  // for correlation matches.  It is based on max distance that the echo
  // could have traveled.

  _maxSearchGrid = (int)(_maxDistEchoGrid + 1.0001);
  
  // Calculate the grid used for calculations.  All calculations are
  // done on a sparce grid at the vector spacing interval, and then are
  // filled, if necessary, before being output.

  _vectorXStart = _maxSearchGrid + _boxXRadius;
  _vectorXEnd = _nx - _maxSearchGrid - _boxXRadius;
  
  _vectorYStart = _maxSearchGrid + _boxXRadius;
  _vectorYEnd = _ny - _maxSearchGrid - _boxXRadius;
  
  _vectorSpacing = (int)(_cboxSpace / _deltaXKm);
  if (_vectorSpacing < 1)
    _vectorSpacing = 1;
  
  // Allocate space for the output grids

  _uGrid.realloc(_nx, _ny);
  _vGrid.realloc(_nx, _ny);
    
  _base.realloc(_boxNx, _boxNy);
  _corCoefGrid.realloc(_nx, _ny);
  _maxCorrGrid.realloc(_nx, _ny);
  
}


/**********************************************************************
 * _solveLinEq() - Solve 3 simultaneous linear equations of the form:
 *                   r = a0 + a1*x + a2*x*x
 *                 The caller passes in the r's and the associated x's.
 *
 * Returns the maximum or minimum r value.  Returns the associated
 * x value in xp.
 */

double CtrecAlg::_solveLinEq(const double r1, const double r2, const double r3,
			     const double x1, const double x2, const double x3,
			     double &xp) const                      // output
{
  double a0, a1, a2;
  
//  a2 = (((r2 - r1) * (x3 - x1)) + ((r1 - r3) * (x2 - x1))) /
//    ((x2 - x1) * (x3 - x1) * (x2 - x3));
//  a1 = (r2 - r1 - (a2 * ((x2 * x2) - (x1 * x1)))) / (x2 - x1);
//  a0 = r1 - (x1 * a1) - (x1 * x1 * a2);
  
  a2 = (((r3 -r1) * (x2 - x1)) + ((r2 - r1) * (x1 - x3))) /
    ((x2 - x1) * (((x2 + x1) * (x1 - x3)) + ((x3 - x1) * (x3 + x1))));
  a1 = (r2 - r1 - (a2 * (x2 - x1) * (x2 + x1))) / (x2 - x1);
  a0 = r1 - (a1 * x1) - (a2 * x1 * x1);
  
  xp = -a1 / (a2 + a2);
  
  return a0 + (a1 * xp) + (a2 * xp * xp);
}


/**********************************************************************
 * _thresholdGlobalMean() - Discard vectors that differ too greatly from
 *                          the global mean.
 */

void CtrecAlg::_thresholdGlobalMean()
{
  // Calculate the global mean

  double avgu;
  double avgv;
  int nvec;
    
  if (!_calcGlobalMean(avgu, avgv, nvec))
    return;
  
  // Threshold the data

  for (int index = 0; index < _nx * _ny; ++index)
  {
    if (_uGrid.get(index) == BAD_OUTPUT_VALUE ||
	_vGrid.get(index) == BAD_OUTPUT_VALUE)
      continue;
      
    if (fabs(_uGrid.get(index) - avgu) > _thrDiff ||
	fabs(_vGrid.get(index) - avgv) > _thrDiff)
    {
      _uGrid.set(index, BAD_OUTPUT_VALUE);
      _vGrid.set(index, BAD_OUTPUT_VALUE);
    }

  } /* endfor - index */
}


/**********************************************************************
 * _thresholdLocalMean() - Eliminate outlying computed echo motion
 *                         vectors by comparing them to a local mean.
 */

void CtrecAlg::_thresholdLocalMean(SimpleGrid<fl32> &data,
				   DsMdvx &output_mdv_file,
				   const string field_name,
				   const Mdvx::field_header_t data_field_hdr,
				   const Mdvx::vlevel_header_t data_vlevel_hdr)
{
  int box_size;           // Number of grid points in radius

  int box_minx, box_maxx, box_miny, box_maxy;
  
  int num_pts;
  double avg;
  double sum;
  
  // Allocate a temporary local mean grid for debugging purposes

  SimpleGrid<fl32> *local_mean_grid = 0;

  if (_outputLocalMeanGrids)
  {
    local_mean_grid = new SimpleGrid<fl32>(_nx, _ny);
    
    for (int index = 0; index < _nx * _ny; index++)
      local_mean_grid->set(index, BAD_OUTPUT_VALUE);
  }
  
  // Compute the number of grid points in the radius to compute the
  // local mean.

  box_size = (int)(_radMean / _deltaXKm);
  
  if (box_size <= 0)
    box_size = 1;
  
  if (_debugFlag)
  {
    cout << "............decimating data..........." << endl;
    cout << "decimate: igrid, thr_vec,_vectorSpacing= " << box_size << " " <<
      _thrVec << " " << _vectorSpacing << endl;
    cout << "max_search_grid = " << _maxSearchGrid << endl;
    cout << "nx,ny,dx,dy,rad_mean,thr_vec " << _nx << " " << _ny << " " <<
      _deltaXKm << " " << _deltaYKm << " " << _radMean << " " << _thrVec << endl;
  }
  
  // Loop through the image, operating on each motion vector.

  for (int x = _vectorXStart; x < _vectorXEnd; x += _vectorSpacing)
  {
    // Calculate the X index limits for the box

    box_minx = x - box_size;
    box_maxx = x + box_size;
    
    if (box_minx < 0)
      box_minx = 0;
    if (box_maxx >= _nx)
      box_maxx = _nx - 1;
    
    for (int y = _vectorYStart; y < _vectorYEnd; y += _vectorSpacing)
    {
      int index = x + (y * _nx);
      
      if (data.get(x, y) == BAD_OUTPUT_VALUE)
	continue;
      
      // Calculate the Y index limits for the box

      box_miny = y - box_size;
      box_maxy = y + box_size;

      if (box_miny < 0)
	box_miny = 0;

      if (box_maxy >= _ny)
	box_maxy = _ny - 1;
      
      // Sum the data within the box

      sum = 0.0;
      num_pts = 0;
      
      for (int xx = box_minx; xx <= box_maxx; ++xx)
      {
	for (int yy = box_miny; yy <= box_maxy; ++yy)
	{
	  if (data.get(xx, yy) == BAD_OUTPUT_VALUE ||
	      (xx == x && yy == y))
	    continue;
	  
	  num_pts++;
	  sum += data.get(xx, yy);
	  
	} /* endfor - yy */
      } /* endfor - xx */
      
      // Compare data with average, throw out if difference too big or not
      // enough points to compute average

      if (num_pts >= 5)
      {
	avg = sum / (double)num_pts;
	if (fabs(data.get(x, y) - avg) > _thrVec)
	  data.set(x, y, BAD_OUTPUT_VALUE);

	if (local_mean_grid != 0)
	  local_mean_grid->set(index, avg);
	
      }
      else
      {
	data.set(x, y, BAD_OUTPUT_VALUE);
      }
      
    } /* endfor - y */
  } /* endfor - x */
  
  if (local_mean_grid != 0)
  {
    _addLocalMeanGridToOutputFile(output_mdv_file,
				  field_name,
				  data_field_hdr,
				  data_vlevel_hdr,
				  *local_mean_grid);
    
    delete local_mean_grid;
  }
  
}


/**********************************************************************
 * _trackEchoes() - Do the echo tracking by finding local maxima in the
 *                  correlation function.
 */

void CtrecAlg::_trackEchoes(const MdvxField &prev_field,
			    const MdvxField &curr_field,
			    const int image_delta_secs,
			    DsMdvx &output_mdv_file)
{
  static const string method_name = "CtrecAlg::_trackEchoes()";
  
  // Initialize local variables

  _prevImage = (fl32 *)prev_field.getVol();
  _currImage = (fl32 *)curr_field.getVol();

  // Create the list of x,y points for outputting the calculated
  // correlation values.

  MdvxPjg projection(curr_field.getFieldHeader());
  vector< GridPoint > corr_index_list;

  _createCorrIndexList(projection, corr_index_list);
  

  if (_debugFlag)
  {
    cout << "array size, spacing, _vectorSpacing = " << _cboxSize << " " <<
      _cboxSpace << " " << _vectorSpacing << endl;
    cout << "max_search_grid is " << _maxSearchGrid << endl;
    cout << "max_dist_echo_km is " << _maxDistEchoGrid * _deltaXKm << endl;
    cout << "nx,ny,_boxXRadius,_vectorSpacing= " << _nx << " " << _ny << " " <<
      _boxXRadius << " " << _vectorSpacing << endl;
  }
  
  // Initialize the output grids to the bad data value

  PMU_auto_register("Initializing grids");
  
  _initializeGrid(_uGrid, _nx, _ny, (fl32)BAD_OUTPUT_VALUE);
  _initializeGrid(_vGrid, _nx, _ny, (fl32)BAD_OUTPUT_VALUE);
  _initializeGrid(_maxCorrGrid, _nx, _ny, (fl32)BAD_OUTPUT_VALUE);
  
  // If outputting a grid containing a count of the number of positions
  // where the max correlation value occurred, initialize that grid.

  SimpleGrid<ui08> *cormax_count_grid = 0;
  if (_outputCormaxCountGrid)
  {
    cormax_count_grid = new SimpleGrid<ui08>(_nx, _ny);

    for (int i = 0; i < _nx * _ny; ++i)
      cormax_count_grid->set(i, BAD_COUNT_VALUE);
  }
  
  // Loop over all possible correlation boxes in first scan

  for (int x1 = _vectorXStart; x1 < _vectorXEnd; x1 += _vectorSpacing)
  {
    PMU_auto_register("Looping over possible correlation boxes");
    
    for (int y1 = _vectorYStart; y1 < _vectorYEnd; y1 += _vectorSpacing)
    {
      double x_begin_grid = (double)x1;
      double y_begin_grid = (double)y1;
      
      double sum_prev = 0.0;
      double sum_prev2 = 0.0;
      
      // Initialize the base grid.  This is the box in the previous
      // data grid that we are trying to match to the current data
      // grid.

      _initializeSubgrid(_prevImage, x1, y1, _base);
      
      // Count the number of data points outside of the defined
      // signal range and don't process this box if there aren't
      // enough points.

      int num_bad_pts = 0;
      
      for (int i = 0; i < _numPtsInBox; ++i)
      {
	if (_base.get(i) < _minEcho ||
	    _base.get(i) > _maxEcho)
	  num_bad_pts++;
      } /* endfor - i */
 
      if (((double)num_bad_pts / _numPtsInBox) > _cboxFract)
	continue;
      
      // Sum the data values in the base grid for use in the correlation
      // calculations.

      for (int base_x = 0; base_x < _boxNx; ++base_x)
      {
	for (int base_y = 0; base_y < _boxNy; ++base_y)
	{
	  double base_data = _base.get(base_x, base_y);
	  
	  sum_prev += base_data;
	  sum_prev2 += (base_data * base_data);
	  
	} /* endfor - base_y */
      } /* endfor - base_x */
      
      // Loop over all possible boxes in the second scan that are within
      // the search radius

      _initializeGrid(_corCoefGrid, _nx, _ny, (fl32)BAD_OUTPUT_VALUE);
      
      for (int x2 = x1 - _maxSearchGrid; x2 <= x1 + _maxSearchGrid; ++x2)
      {
	for (int y2 = y1 - _maxSearchGrid; y2 <= y1 + _maxSearchGrid; ++y2)
	{
	  // See if the box we are checking is too far from the original box

	  double x_dist = (double)x2 - x_begin_grid;
	  double y_dist = (double)y2 - y_begin_grid;
	  double range = sqrt((x_dist * x_dist) + (y_dist * y_dist));

	  if (range > _maxDistEchoGrid)
	    continue;
	  
	  // Create the subgrid of the current data that we are
	  // testing

	  SimpleGrid<fl32> test_subgrid(_boxNx, _boxNy);
	  
	  _initializeSubgrid(_currImage, x2, y2, test_subgrid);
	  
	  // Calculate the correlation coefficient

	  double corcoef;
	  
	  if (!_calcCorrCoef(_base,
			     sum_prev, sum_prev2,
			     test_subgrid,
			     corcoef))
	    continue;
	  
	  _corCoefGrid.set(x2, y2, 100.0 * corcoef);
	  
	} /* endfor - y2 */
      } /* endfor - x2 */
      
      // See if we need to create an output field for these
      // correlation calculations.

      GridPoint index_pt(x1, y1);
      
      if (find(corr_index_list.begin(), corr_index_list.end(),
	       index_pt) != corr_index_list.end())
      {
	cerr << "*** Adding corr grid for point " << x1 <<
	  ", " << y1 << endl;
	
	_addIndCorrGridToOutputFile(output_mdv_file,
				    curr_field.getFieldHeader(),
				    curr_field.getVlevelHeader(),
				    _corCoefGrid,
				    x1, y1);
      }
      
      int cormax_x, cormax_y;
      int cormax_count = 0;
      
      GridPoint cormax_point;
      
      double cormax = _cormaxSearcher.getMaxValue(_corCoefGrid,
						  x1 - _maxSearchGrid,
						  x1 + _maxSearchGrid,
						  y1 - _maxSearchGrid,
						  y1 + _maxSearchGrid,
						  (fl32)BAD_OUTPUT_VALUE,
						  cormax_point,
						  cormax_count);
      
      cormax_x = cormax_point.x;
      cormax_y = cormax_point.y;
      
      if (cormax == BAD_OUTPUT_VALUE)
      {
	if (cormax_count_grid != 0)
	  cormax_count_grid->set(x1, y1, BAD_COUNT_VALUE);
	
	continue;
      }
      
      if (cormax_count_grid != 0)
	cormax_count_grid->set(x1, y1, cormax_count);
      
      // Save the maximum correlation value for debugging.

      _maxCorrGrid.set(x1, y1, cormax);

      // Don't calculate the vector if the correlation is too low

      if (cormax < _thrCor)
	continue;
      
      // Calculate motion for the current grid point

      double x_end_grid, y_end_grid;
      
      _calcEndPos(cormax_x, cormax_y,
		  x1 - _maxSearchGrid, x1 + _maxSearchGrid,
		  y1 - _maxSearchGrid, y1 + _maxSearchGrid,
		  x_end_grid, y_end_grid);
      
      double x_begin_km = _minX + (x_begin_grid * _deltaXKm);
      double y_begin_km = _minY + (y_begin_grid * _deltaYKm);

      double x_end_km = _minX + (x_end_grid * _deltaXKm);
      double y_end_km = _minY + (y_end_grid * _deltaYKm);

//      cout << "x_begin_km = " << x_begin_km << endl;
//      cout << "x_end_km = " << x_end_km << endl;
//      cout << "y_begin_km = " << y_begin_km << endl;
//      cout << "y_end_km = " << y_end_km << endl;
      
      if (fabs(x_begin_km) <= 0.001 && fabs(y_begin_km) <= 0.001)
	continue;
      
      _uGrid.set(x1, y1,
		  1000.0 * (x_end_km - x_begin_km) / image_delta_secs);
      _vGrid.set(x1, y1,
		  1000.0 * (y_end_km - y_begin_km) / image_delta_secs);
      
    } /* endfor - y1 */
  } /* endfor - x1 */
  
  // Output the requested debugging grids

  PMU_auto_register("Outputting requested debugging grids");
  
  if (_outputCorrelationGrid)
    _addCorrGridToOutputFile(output_mdv_file,
			     curr_field.getFieldHeader(),
			     curr_field.getVlevelHeader());
  
  if (cormax_count_grid != 0)
    _addCormaxCountGridToOutputFile(output_mdv_file,
				    curr_field.getFieldHeader(),
				    curr_field.getVlevelHeader(),
				    *cormax_count_grid);
  
  if (_outputOriginalVectors)
    _addUVToOutputFile(output_mdv_file,
		       curr_field.getFieldHeader(),
		       curr_field.getVlevelHeader(),
		       "orig",
		       "original");
  
  // Free temporary grids

  if (cormax_count_grid != 0)
  {
    delete cormax_count_grid;
    cormax_count_grid = 0;
  }
  
}
