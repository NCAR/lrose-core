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
/*********************************************************************
 * ConvPartition.cc: class implementing the convective partition
 *                   described by Steiner, etal in "Climatological
 *                   Characterization of Three-Dimensional Storm
 *                   Structure from Operational Radar and Rain Gauge
 *                   Data" in the Journal of Applied Meteorology,
 *                   Sept. 1995, vol. 34, pp. 1983-1990.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>

#include <stdio.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <euclid/CircularTemplate.hh>
#include <euclid/CircularTemplateList.hh>
#include <euclid/GridPoint.hh>
#include <euclid/SimpleGrid.hh>
#include <Mdv/MdvxField.hh>
#include <rapmath/PtFunction.hh>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>

#include "ConvPartition.hh"


/**********************************************************************
 * Constructor
 */

ConvPartition::ConvPartition(PtFunction *value_diff_function,
			     PtFunction *conv_radius_function,
			     double conv_center_min,
			     double background_min_dbz,
			     double background_radius,
			     const bool data_increasing,
			     const bool debug) :
  _debug(debug),
  _dataIncreasing(data_increasing),
  _backgroundMinDbz(background_min_dbz),
  _convCenterMin(conv_center_min),
  _valueDiffFunc(value_diff_function),
  _convRadiusFunc(conv_radius_function),
  _meanData(1, 1),
  _partition(1, 1),
  _excess(1, 1),
  _threshold(1, 1)
{
  // Create the background template

  _backgroundTemplate = new CircularTemplate(background_radius);
  
  // Create the convective radius templates

  map< double, double, less<double> >::iterator conv_radius_iterator;
  
  for (conv_radius_iterator = _convRadiusFunc->begin();
       conv_radius_iterator != _convRadiusFunc->end();
       conv_radius_iterator++)
  {
    _convRadiusTemplates.addTemplate((*conv_radius_iterator).second);
  }
  
}


/**********************************************************************
 * Destructor
 */

ConvPartition::~ConvPartition()
{
  delete _valueDiffFunc;
  delete _convRadiusFunc;
  delete _backgroundTemplate;
}
  

/**********************************************************************
 * run() - Run the algorithm.
 */

void ConvPartition::run(MdvxField &field)
{
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  
  // Allocate space for the data grids

  _allocateDataGrids(field_hdr.nx, field_hdr.ny);
  
  // Calculate the internal data grids

  fl32 *data = (fl32 *)field.getVol();
  
  SimpleGrid<fl32> data_grid(field_hdr.nx, field_hdr.ny);
  for (int i = 0; i < field_hdr.nx * field_hdr.ny; ++i)
    data_grid.set(i, data[i]);
  
  if (_debug)
    fprintf(stderr, "    Getting ready to calculate data grids.\n");

  _calculateDataGrids(data_grid,
		      field_hdr.missing_data_value,
		      field_hdr.bad_data_value);
  
  if (_debug)
    fprintf(stderr, "    Data grids calculated.\n");

  // Generate the convective partition

  _generatePartition(data_grid,
		     field_hdr.missing_data_value,
		     field_hdr.bad_data_value);
  
  if (_debug)
    fprintf(stderr, "    Partition generated.\n");

  return;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _allocateDataGrids() - Allocate space for the internal data grids.
 */

void ConvPartition::_allocateDataGrids(int nx, int ny)
{
  // Allocate the space for the grids

  _meanData.realloc(nx, ny);
  _partition.realloc(nx, ny);
  _excess.realloc(nx, ny);
  _threshold.realloc(nx, ny);
  
  // Clear the information in the grids

  _meanData.initializeGrid(0.0);
  _partition.initializeGrid(MISSING_DATA);
  _excess.initializeGrid(-9999.0);
  _threshold.initializeGrid(-9999.0);
  
  return;
}
  

/**********************************************************************
 * _calculateDataGrids() - Calculate the internal data grids needed in
 *                         the algorithm calculations.
 */

void ConvPartition::_calculateDataGrids(const SimpleGrid<fl32> &grid,
					const fl32 missing_data_value,
					const fl32 bad_data_value)
{
  // Calculate the mean background values.

  int nx = grid.getNx();
  int ny = grid.getNy();

  for (int y = 0; y < ny; y++)
  {
    PMU_auto_register("In ConvPartition::_calculateDataGrids");

//    if (_debug)
//      fprintf(stderr, "      Looking at row %d\n", y);

    for (int x = 0; x < nx; x++)
    {
      fl32 data_value = grid.get(x, y);
      
      if (data_value != missing_data_value &&
	  data_value != bad_data_value)
      {
	_meanData.set(x, y, _calculateDataMean(grid,
					       missing_data_value,
					       bad_data_value,
                                               _backgroundMinDbz,
					       _backgroundTemplate,
					       x, y));
      }
      else
	_meanData.set(x, y, missing_data_value);
      
    } /* endfor - x */
  } /* endfor - y */

}


/**********************************************************************
 * _calculateDataMean() - Calculate the mean data value for data within
 *                        the given template centered on the given point.
 */

double ConvPartition::_calculateDataMean(const SimpleGrid<fl32> &data,
					 const fl32 missing_data_value,
					 const fl32 bad_data_value,
                                         double background_min_dbz,
					 CircularTemplate *templ,
					 int x, int y)
{
  double data_total = 0.0;
  int num_data_points = 0;
  
  GridPoint *point;
  
  int nx = data.getNx();
  int ny = data.getNy();
  
  //
  // Niles : added the following call to auto_register.
  // We are having restarts at WSMR on a heavily loaaded machine.
  //
  PMU_auto_register("In ConvPartition::_calculateDataMean");

  for (point = templ->getFirstInGrid(x, y, nx, ny);
       point != (GridPoint *)NULL;
       point = templ->getNextInGrid())
  {
    fl32 data_value = data.get(point->getIndex(nx, ny));
    
    if (data_value != missing_data_value &&
	data_value != bad_data_value &&
        data_value > background_min_dbz)
    {
      data_total += data_value;
      num_data_points++;
    }
    
  } /* endfor - point */
  
  if (num_data_points <= 0)
    return missing_data_value;
  else
    return data_total / num_data_points;
}
  

/**********************************************************************
 * _generatePartition() - Generate the convective partition of the
 *                        data.
 */

void ConvPartition::_generatePartition(const SimpleGrid<fl32> &data_grid,
				       const fl32 missing_data_value,
				       const fl32 bad_data_value)
{
  const string routine_name = "_generatePartition()";

  // Calculate the partition

  for (int y = 0; y < data_grid.getNy(); y++)
  {
    for (int x = 0; x < data_grid.getNx(); x++)
    {
      // Check for missing data

      fl32 data_value = data_grid.get(x, y);
      fl32 mean_data_value = _meanData.get(x, y);
      
      if (data_value == missing_data_value ||
	  data_value == bad_data_value ||
          data_value <= _backgroundMinDbz)
	continue;

      fl32 excess = data_value - mean_data_value;
      fl32 threshold = _valueDiffFunc->computeFunctionValue(mean_data_value);

      _excess.set(x, y, excess);
      _threshold.set(x, y, threshold);
      
      // Set the data value to STRATIFORM initially.  We will reset it to
      // CONVECTIVE is we determine it is convective.

      if (_partition.get(x, y) == MISSING_DATA)
	_partition.set(x, y, STRATIFORM);
      
      // After this, we will set the conv_center flag when we find a center.
      // We do this so we can fill in the entire convective area in the
      // partition as we go along.

      bool conv_center = false;
      
      // Check for criteria 1

      if(_dataIncreasing) 
      {
        
	if (data_value >= _convCenterMin)
	{
	  _partition.set(x, y, CONVECTIVE);
	  conv_center = true;
	}
      
	// Check for criteria 2

	else if (excess > 0 && excess >= threshold)
	{
	  _partition.set(x, y, CONVECTIVE);
	  conv_center = true;
	}
      } /* endif -- _dataIncreasing */
      else 
      {
	if (data_value <= _convCenterMin)
	{
	  _partition.set(x, y, CONVECTIVE);
	  conv_center = true;
	}
      
	// Check for criteria 2

	else if (excess < 0 && excess <= threshold)
	{
	  _partition.set(x, y, CONVECTIVE);
	  conv_center = true;
	}
      }  /* endelse - data decreasing */
	  
      
      // We have performed all of the convective center tests so continue if
      // this isn't a center.

      if (!conv_center)
	continue;
      
      // Now fill in the convective area for the convective center

      double conv_radius =
	_convRadiusFunc->computeFunctionValue(mean_data_value);
      
      CircularTemplate *area_template;
      if ((area_template = _convRadiusTemplates.getTemplate(conv_radius))
	  == (CircularTemplate *)NULL)
      {
	fprintf(stderr,
		"ERROR:  %s::%s\n", _className(), routine_name.c_str());
	fprintf(stderr,
		"Template with radius %f not found in template list.\n",
		conv_radius);
	fprintf(stderr,
		"Convective area for this center will NOT be marked convective.\n");
	
	continue;
      }
      
      GridPoint *point;
      
      for (point = area_template->getFirstInGrid(x, y,
						 _partition.getNx(),
						 _partition.getNy());
	   point != (GridPoint *)NULL;
	   point = area_template->getNextInGrid())
      {
	_partition.set(point->getIndex(_partition.getNx(),
				       _partition.getNy()),
		       CONVECTIVE);
      }
      
    } /* endfor - x */
  } /* endfor - y */
  
  return;
}
