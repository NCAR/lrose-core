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
//   $Date: 2016/03/07 18:28:25 $
//   $Id: DrawFmqPolygonStats.cc,v 1.27 2016/03/07 18:28:25 dixon Exp $
//   $Revision: 1.27 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DrawFmqPolygonStats: DrawFmqPolygonStats program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <euclid/point.h>
#include <euclid/geometry.h>
#include <Mdv/MdvxPjg.hh>
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "DrawFmqPolygonStats.hh"
#include "Params.hh"

#include "DrawFmqInput.hh"
#include "GenPolyInput.hh"

using namespace std;

// Global variables

DrawFmqPolygonStats *DrawFmqPolygonStats::_instance =
     (DrawFmqPolygonStats *)NULL;

const double DrawFmqPolygonStats::MISSING_VALUE = -9999.0;

/*********************************************************************
 * Constructor
 */

DrawFmqPolygonStats::DrawFmqPolygonStats(int argc, char **argv) :
  _histogramFile(0)
{
  static const string method_name = "DrawFmqPolygonStats::DrawFmqPolygonStats()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (DrawFmqPolygonStats *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = new char[sizeof("unknown")+1];
  strcpy(params_path, "unknown");
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

DrawFmqPolygonStats::~DrawFmqPolygonStats()
{
  // Close the optional histogram output file

  if (_histogramFile != 0)
    fclose(_histogramFile);
  
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

DrawFmqPolygonStats *DrawFmqPolygonStats::Inst(int argc, char **argv)
{
  if (_instance == (DrawFmqPolygonStats *)NULL)
    new DrawFmqPolygonStats(argc, argv);
  
  return(_instance);
}

DrawFmqPolygonStats *DrawFmqPolygonStats::Inst()
{
  assert(_instance != (DrawFmqPolygonStats *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool DrawFmqPolygonStats::init()
{
  static const string method_name = "DrawFmqPolygonStats::init()";
  
  // Initialize the input handler

  switch (_params->input_type)
  {
  case Params::INPUT_DRAW_FMQ :
  {
    DrawFmqInput *input =
      new DrawFmqInput(_params->apply_polygon_to_all_elevations,
		       _params->debug_level >= Params::DEBUG_EXTRA);
    
    if (!input->init(_progName, _params->cidd_draw_fmq))
      return false;
    
    _input = input;
    
    break;
  }
  
  case Params::INPUT_GEN_POLY_SPDB :
  {
    GenPolyInput *input =
      new GenPolyInput(_params->debug_level >= Params::DEBUG_EXTRA);
    
    DateTime start_time(_params->start_time);
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time parameter: "
	   << _params->start_time << endl;
      cerr << "Fix parameters and try again" << endl;
      
      return false;
    }
    
    DateTime end_time(_params->end_time);
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time parameter: "
	   << _params->end_time << endl;
      cerr << "Fix parameters and try again" << endl;
      
      return false;
    }
    
    if (!input->init(_params->input_gen_poly_url, start_time, end_time))
      return false;
    
    _input = input;
    
    break;
  }
  
  } /* endswitch - _params->input_type */
  
  // Initialize the MDV input handler

  if (!_mdvHandler.init(_params->input_mdv_url,
			_params->mdv_search_margin,
			_params->use_mdv_field_names,
			_params->debug_level >= Params::DEBUG_NORM))
    return false;
  
  _mdvHandler.setDbzField(_params->dbz_field.field_name,
			  _params->dbz_field.field_num);
  _mdvHandler.setZdrField(_params->zdr_field.field_name,
			  _params->zdr_field.field_num);
    
  for (int i = 0; i < _params->stat_fields_n; ++i)
  {
    _mdvHandler.addStatisticField(_params->_stat_fields[i].field_name,
				  _params->_stat_fields[i].field_num,
				  _params->_stat_fields[i].is_log);
  }
  
  for (int i = 0; i < _params->discrete_fields_n; ++i)
  {
    _mdvHandler.addDiscreteStatisticField(_params->_discrete_fields[i].field_name,
					  _params->_discrete_fields[i].field_num);
  }
  
  for (int i = 0; i < _params->threshold_fields_n; ++i)
  {
    _mdvHandler.addThresholdField(_params->_threshold_fields[i].field_name,
				  _params->_threshold_fields[i].field_num);
  }
  
  for (int i = 0; i < _params->dropsize_thresh_fields_n; ++i)
  {
    _mdvHandler.addDropsizeThreshField(_params->_dropsize_thresh_fields[i].field_name,
				       _params->_dropsize_thresh_fields[i].field_num);
  }
  
  // Initialize the optional histogram output file

  if (_params->write_histogram_file)
  {
    if ((_histogramFile = fopen(_params->histogram_file_path, "w")) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening histogram output file" << endl;
      perror(_params->histogram_file_path);
      
      return false;
    }
  }
  
  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void DrawFmqPolygonStats::run()
{
  static const string method_name = "DrawFmqPolygonStats::run()";
  
  // Read from the input queue forever, processing messages when appropriate

  while (!_input->endOfInput())
  {
    PMU_auto_register("Waiting for data");
    
    // Get the next message from the input stream

    Human_Drawn_Data_t input_prod;
    
    if (!_input->getNextProd(input_prod))
      continue;

    // Process the message.  Don't check the return value because we want to
    // try the next message even if we have problems processing this one.

    _processInput(input_prod);
    
    // Reclaim space from the input message

    delete [] input_prod.lat_points;
    delete [] input_prod.lon_points;
    
  } /* endwhile - true */
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addDbzZdrStatistics()
 */

bool DrawFmqPolygonStats::_addDbzZdrStatistics(const int vlevel_num,
					       unsigned char *grid_array,
					       unsigned char *dropsize_grid_array,
					       GenPolyStats &polygon)
{
  static const string method_name = "DrawFmqPolygonStats::_addDbzZdrStatistics()";

  const MdvxField *dbz_field = _mdvHandler.getDbzField();
  const MdvxField *zdr_field = _mdvHandler.getZdrField();
  
  Mdvx::field_header_t dbz_field_hdr = dbz_field->getFieldHeader();
  Mdvx::field_header_t zdr_field_hdr = zdr_field->getFieldHeader();
  
  int plane_size = dbz_field_hdr.nx * dbz_field_hdr.ny;
  
  // Create the list of values that fall within the polygon

  fl32 *dbz_data = (fl32 *)dbz_field->getVol();
  fl32 *zdr_data = (fl32 *)zdr_field->getVol();
  fl32 *dbz_data_ptr = dbz_data + (plane_size * vlevel_num);
  fl32 *zdr_data_ptr = zdr_data + (plane_size * vlevel_num);
  
  vector< double > zh_values_log;
  vector< double > zv_values_log;
  vector< double > zdr_values_log;
  
  vector< double > zh_values_linear;
  vector< double > zv_values_linear;
  vector< double > zdr_values_linear;

  vector< double > dmax_z_values;
  vector< double > dmax_zdr_values;
  vector< double > d0_z_values;
  vector< double > d0_zdr_values;
  
  for (int i = 0; i < plane_size; ++i, ++dbz_data_ptr, ++zdr_data_ptr)
  {
    // Ignore values not within the polygon

    if (grid_array[i] <= 0)
      continue;
      
    // Ignore bad and missing data values

    if (*dbz_data_ptr == dbz_field_hdr.bad_data_value ||
	*dbz_data_ptr == dbz_field_hdr.missing_data_value ||
	*zdr_data_ptr == zdr_field_hdr.bad_data_value ||
	*zdr_data_ptr == zdr_field_hdr.missing_data_value)
      continue;
      
    // Calculate the values in both log space and linear space.

    double zh_value_log = *dbz_data_ptr;
    double zdr_value_log = *zdr_data_ptr;
    
    double zh_value_linear = _logToLinear(zh_value_log);
    double zdr_value_linear = _logToLinear(zdr_value_log);
    double zv_value_linear = zh_value_linear / zdr_value_linear;
    
    double zv_value_log = _linearToLog(zv_value_linear);
  
    // Add the values to the appropriate value lists

    zh_values_log.push_back(zh_value_log);
    zv_values_log.push_back(zv_value_log);
    zdr_values_log.push_back(zdr_value_log);
    
    zh_values_linear.push_back(zh_value_linear);
    zv_values_linear.push_back(zv_value_linear);
    zdr_values_linear.push_back(zdr_value_linear);
    
    // Now calculate the dropsize values.  We do this after calculating
    // the other values because additional thresholds are applied to these
    // calculations.

    if (dropsize_grid_array[i] <= 0)
      continue;

    double dropsize_zh = zh_value_log;
    double dropsize_zdr = zdr_value_log;
    
    if (dropsize_zdr < _params->min_dropsize_zdr)
      dropsize_zdr = _params->min_dropsize_zdr;
    else if (dropsize_zdr > _params->max_dropsize_zdr)
      dropsize_zdr = _params->max_dropsize_zdr;
    
    double dmax_z_value = _calcDmaxZh(dropsize_zh);
    
    double dmax_zdr_value = _calcDmaxZdr(dropsize_zdr);
    
    double d0_z_value = _calcD0Zh(dropsize_zh);
    
    double d0_zdr_value = _calcD0Zdr(dropsize_zdr);
  
    dmax_z_values.push_back(dmax_z_value);
    dmax_zdr_values.push_back(dmax_zdr_value);
    d0_z_values.push_back(d0_z_value);
    d0_zdr_values.push_back(d0_zdr_value);
    
  } /* endfor - i */
  
  // Calculate the statistics and add them to the polygon

  double zh_mean =
    _addFieldStatsToPolygon(dbz_field_hdr.field_name,
			    dbz_field_hdr.units,
			    zh_values_linear, zh_values_log, true, polygon);
  
  _addFieldStatsToPolygon("ZV", "dBz", zv_values_linear, zv_values_log,
			  true, polygon);
  
  double zdr_mean =
    _addZdrFieldStatsToPolygon(zdr_field_hdr.field_name,
			       zdr_field_hdr.units,
			       zdr_values_linear, zdr_values_log, 
			       zh_values_linear, zv_values_linear, polygon);
  
  // For the dropsize variables, we are only calculating the mean

  double mean_dmax_z = _calcMean(dmax_z_values);
  double mean_dmax_zdr = _calcMean(dmax_zdr_values);
  double mean_d0_z = _calcMean(d0_z_values);
  double mean_d0_zdr = _calcMean(d0_zdr_values);
  double mean_dmax;
  if (mean_dmax_z == MISSING_VALUE || mean_dmax_zdr == MISSING_VALUE)
    mean_dmax = MISSING_VALUE;
  else
    mean_dmax = (mean_dmax_z + mean_dmax_zdr) / 2.0;
  double mean_d0;
  if (mean_d0_z == MISSING_VALUE || mean_d0_zdr == MISSING_VALUE)
    mean_d0 = MISSING_VALUE;
  else
    mean_d0 = (mean_d0_z + mean_d0_zdr) / 2.0;
  
  polygon.addField("mean dmax_z", mean_dmax_z, "mm");
  polygon.addField("mean dmax_zdr", mean_dmax_zdr, "mm");
  polygon.addField("mean d0_z", mean_d0_z, "mm");
  polygon.addField("mean d0_zdr", mean_d0_zdr, "mm");
  polygon.addField("mean dmax", mean_dmax, "mm");
  polygon.addField("mean d0", mean_d0, "mm");
  
  // Also calculate dropsize means using the zh and zdr means

  double mean_sdmax_z = _calcDmaxZh(zh_mean);
  double mean_sdmax_zdr = _calcDmaxZdr(zdr_mean);
  double mean_sd0_z = _calcD0Zh(zh_mean);
  double mean_sd0_zdr = _calcD0Zdr(zdr_mean);
  double mean_sdmax;
  if (mean_sdmax_z == MISSING_VALUE || mean_sdmax_zdr == MISSING_VALUE)
    mean_sdmax = MISSING_VALUE;
  else
    mean_sdmax = (mean_sdmax_z + mean_sdmax_zdr) / 2.0;
  double mean_sd0;
  if (mean_sd0_z == MISSING_VALUE || mean_sd0_zdr == MISSING_VALUE)
    mean_sd0 = MISSING_VALUE;
  else
    mean_sd0 = (mean_sd0_z + mean_sd0_zdr) / 2.0;

  polygon.addField("mean sdmax_z", mean_sdmax_z, "mm");
  polygon.addField("mean sdmax_zdr", mean_sdmax_zdr, "mm");
  polygon.addField("mean sd0_z", mean_sd0_z, "mm");
  polygon.addField("mean sd0_zdr", mean_sd0_zdr, "mm");
  polygon.addField("mean sdmax", mean_sdmax, "mm");
  polygon.addField("mean sd0", mean_sd0, "mm");
  
  return true;
}


/*********************************************************************
 * _addDiscreteFieldStatistics()
 */

bool DrawFmqPolygonStats::_addDiscreteFieldStatistics(const MdvxField &field,
						      const int vlevel_num,
						      unsigned char *grid_array,
						      GenPolyStats &polygon)
{
  static const string method_name = "DrawFmqPolygonStats::_addDiscreteFieldStatistics()";

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  int plane_size = field_hdr.nx * field_hdr.ny;
  
  // Create the list of values that fall within the polygon

  fl32 *data = (fl32 *)field.getVol();
  fl32 *data_ptr = data + (plane_size * vlevel_num);
  
  vector< double > values;
  
  for (int i = 0; i < plane_size; ++i, ++data_ptr)
  {
    // Ignore values not within the polygon

    if (grid_array[i] <= 0)
      continue;
      
    // Ignore bad and missing data values

    if (*data_ptr == field_hdr.bad_data_value ||
	*data_ptr == field_hdr.missing_data_value)
      continue;
      
    values.push_back(*data_ptr);
    
  } /* endfor - i */
  
  // Calculate the statistics and add them to the polygon

  _addDiscreteFieldStatsToPolygon(field_hdr.field_name, field_hdr.units,
				  values, polygon);
  
  return true;
}


/*********************************************************************
 * _addDiscreteFieldStatsToPolygon()
 */

void DrawFmqPolygonStats::_addDiscreteFieldStatsToPolygon(const string &field_name,
							  const string &field_units,
							  const vector< double > &values,
							  GenPolyStats &polygon) const
{
  // We must have some data values in order to do anything

  if (values.size() == 0)
    return;
  
  // Calculate the histogram and add the percentages to the polygon.

  char stat_name[80];
  map< double, int > histogram = _calcHistogram(polygon, field_name,
						values, _histogramFile);

  for (map<double, int >::const_iterator entry = histogram.begin();
       entry != histogram.end(); ++entry)
  {
    sprintf(stat_name, "perc %d", (int)(round(entry->first)));
    double percent = (double)entry->second / (double)values.size();
      
    _addFieldValue(field_name, field_units, stat_name, percent, polygon);
  }
  
  // Calculate the mode, which relies on the histogram

  vector< double > mode_values = _calcMode(histogram);

  // Now add the values to the polygon

  for (vector< double >::const_iterator mode_value = mode_values.begin();
       mode_value != mode_values.end(); ++mode_value)
    _addFieldValue(field_name, field_units, "mode", *mode_value, polygon);

}


/*********************************************************************
 * _addFieldStatistics()
 */

bool DrawFmqPolygonStats::_addFieldStatistics(const MdvxField &field,
					      const int vlevel_num,
					      const bool is_log,
					      unsigned char *grid_array,
					      GenPolyStats &polygon)
{
  static const string method_name = "DrawFmqPolygonStats::_addFieldStatistics()";

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  int plane_size = field_hdr.nx * field_hdr.ny;
  
  // Create the list of values that fall within the polygon

  fl32 *data = (fl32 *)field.getVol();
  fl32 *data_ptr = data + (plane_size * vlevel_num);
  
  vector< double > values_linear;
  vector< double > values_log;
  
  for (int i = 0; i < plane_size; ++i, ++data_ptr)
  {
    // Ignore values not within the polygon

    if (grid_array[i] <= 0)
      continue;
      
    // Ignore bad and missing data values

    if (*data_ptr == field_hdr.bad_data_value ||
	*data_ptr == field_hdr.missing_data_value)
      continue;
      
    if (is_log)
    {
      values_log.push_back(*data_ptr);
      values_linear.push_back(_logToLinear(*data_ptr));
    }
    else
    {
      values_linear.push_back(*data_ptr);
    }
    
  } /* endfor - i */
  
  // Calculate the statistics and add them to the polygon

  _addFieldStatsToPolygon(field_hdr.field_name, field_hdr.units,
			  values_linear, values_log, is_log, polygon);
  
  return true;
}


/*********************************************************************
 * _addFieldStatsToPolygon()
 */

double DrawFmqPolygonStats::_addFieldStatsToPolygon(const string &field_name,
						    const string &field_units,
						    const vector< double > &values_linear,
						    const vector< double > &values_log,
						    const bool is_log,
						    GenPolyStats &polygon) const
{
  // If we are producing histograms, add this field

  if (_histogramFile != 0)
  {
    if (is_log)
      _calcHistogram(polygon, field_name, values_log, _histogramFile);
    else
      _calcHistogram(polygon, field_name, values_linear, _histogramFile);
  }
  
  // Calculate all of the statistics except the standard deviation
  // which has to be handled specially

  double mean_value = _calcMean(values_linear);
  double median_value = _calcMedian(values_linear);
  double min_value = _calcMinimum(values_linear);
  double max_value = _calcMaximum(values_linear);

  if (is_log)
  {
    mean_value = _linearToLog(mean_value);
    median_value = _linearToLog(median_value);
    min_value = _linearToLog(min_value);
    max_value = _linearToLog(max_value);
  }

  // Calculate the standard deviation value.  This is handled specially
  // because we have to use the actual log values rather than the linear
  // values when working in log space.

  double std_dev_value;
  
  if (is_log)
    std_dev_value = _calcStdDev(values_log);
  else
    std_dev_value = _calcStdDev(values_linear);

  // Now add the values to the polygon

  _addFieldValue(field_name, field_units, "mean", mean_value, polygon);
  _addFieldValue(field_name, field_units, "std dev", std_dev_value, polygon);
  _addFieldValue(field_name, field_units, "median", median_value, polygon);
  _addFieldValue(field_name, field_units, "min", min_value, polygon);
  _addFieldValue(field_name, field_units, "max", max_value, polygon);

  return mean_value;
}


/*********************************************************************
 * _addPolygonStatistics()
 */

bool DrawFmqPolygonStats::_addPolygonStatistics(GenPolyStats &polygon,
						const int scan_time_offset,
						const int vlevel_num,
						const double vlevel_ht,
						const MdvxPjg &proj,
						const double radar_lat,
						const double radar_lon,
						const double radar_alt_km,
						const int scan_mode,
						unsigned char *grid_array,
						const long num_pts_filled) const
{
  static const string method_name = "DrawFmqPolygonStats::_addPolygonStatistics()";

  // Save the scan mode and scan time offset

  polygon.setScanTimeOffset(scan_time_offset);
  polygon.setScanMode(scan_mode);
  
  // Find the actual polygon centroid

  double centroid_lat;
  double centroid_lon;
  
  polygon.calcCentroid(centroid_lat, centroid_lon);
  polygon.setCentroid(centroid_lat, centroid_lon);
  
  /************************ UPDATE *******************************
   * Add y index to the xGrid2km call.
   */

  // Compute the area of the data regions within the polygon

  double x_km = proj.xGrid2km(1);
  double y_km = proj.yGrid2km(1);
  
  double data_area = num_pts_filled * x_km * y_km;
  
  polygon.setDataArea(data_area);
  
  // Find the centroid of the data regions within the polygon.  This is
  // being approximated by the averages of the lat/lon of the center of
  // each grid square being included in the statistics calculations.

  double lat_sum = 0.0;
  double lon_sum = 0.0;
  
  for (int y = 0, index = 0; y < proj.getNy(); ++y)
  {
    for (int x = 0; x < proj.getNx(); ++x, ++index)
    {
      if (grid_array[index] <= 0)
	continue;
      
      double pt_lat;
      double pt_lon;
      
      proj.xyIndex2latlon(x, y, pt_lat, pt_lon);
      
      lat_sum += pt_lat;
      lon_sum += pt_lon;
      
    } /* endfor - x */
  } /* endfor - y, index */
  
  double data_centroid_lat;
  double data_centroid_lon;
  
  if (num_pts_filled <= 0)
  {
    data_centroid_lat = centroid_lat;
    data_centroid_lon = centroid_lon;
  }
  else
  {
    data_centroid_lat = lat_sum / (double)num_pts_filled;
    data_centroid_lon = lon_sum / (double)num_pts_filled;
  }
  
  polygon.setDataCentroid(data_centroid_lat, data_centroid_lon);

  // Save the vertical level information received from CIDD

  polygon.setVlevelIndex(vlevel_num);
  polygon.setElevAngle(vlevel_ht);
  
  // Save the threshold values

  for (int i = 0; i < _params->threshold_fields_n; ++i)
  {
    Mdvx::field_header_t field_hdr =
      _mdvHandler.getThresholdField(i)->getFieldHeader();
    
    polygon.addThreshold(field_hdr.field_name,
			 _params->_threshold_fields[i].value,
			 field_hdr.units);
  }
  
  for (int i = 0; i < _params->dropsize_thresh_fields_n; ++i)
  {
    Mdvx::field_header_t field_hdr =
      _mdvHandler.getDropsizeThreshField(i)->getFieldHeader();
    
    polygon.addDropsizeThresh(field_hdr.field_name,
			      _params->_dropsize_thresh_fields[i].value,
			      field_hdr.units);
  }
  
  // Calculate and save the range and height of the data centroid

  double range_km;
  double theta;
  
  proj.latlon2RTheta(radar_lat, radar_lon, centroid_lat, centroid_lon,
		     range_km, theta);
  
  double earth_sphere_correction = range_km * range_km / 17000.0;
  double height_km = range_km * sin(vlevel_ht * DEG_TO_RAD) +
    earth_sphere_correction + radar_alt_km;
  
  polygon.setDataRange(range_km);
  polygon.setDataHeight(height_km);
  
  return true;
}


/*********************************************************************
 * _addStatistics()
 */

bool DrawFmqPolygonStats::_addStatistics(GenPolyStats &polygon,
					 const double vlevel_ht)
{
  static const string method_name = "DrawFmqPolygonStats::_addStatistics()";

  // Get the necessary information from the MDV file

  Mdvx::master_header_t master_hdr = _mdvHandler.getMasterHeader();

  Mdvx::field_header_t dbz_field_hdr =
    _mdvHandler.getDbzField()->getFieldHeader();
  Mdvx::vlevel_header_t dbz_vlevel_hdr =
    _mdvHandler.getDbzField()->getVlevelHeader();
  
  // Get the vlevel index for this elevation angle

  int vlevel_num = -1;
  
  for (int z = 0; z < dbz_field_hdr.nz; ++z)
  {
    if (dbz_vlevel_hdr.level[z] == vlevel_ht)
      vlevel_num = z;
  }
  
  if (vlevel_num < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot find elevation angle " << vlevel_ht
	 << " in MDV file" << endl;
    cerr << "Skipping polygon" << endl;
    
    return false;
  }
  
  // Update the time in the polygon based on the time in the MDV
  // file.  This is needed because the original MDV data didn't
  // include the lowest elevation so the times changed when the MDV
  // data was regenerated

  int expire_secs = polygon.getExpireTime() - polygon.getTime();
  
  polygon.setTime(master_hdr.time_centroid);
  polygon.setExpireTime(master_hdr.time_centroid + expire_secs);
  
  // Get the polygon grid to use
  
  MdvxPjg proj(dbz_field_hdr);
  
  int plane_size = proj.getNx() * proj.getNy();
  unsigned char *grid_array = new unsigned char[plane_size];
  long num_pts_filled;
  
  if (!_getPolygonGrid(proj, vlevel_num, polygon, grid_array, num_pts_filled))
  {
    delete [] grid_array;
    return false;
  }
  
  // Get the dropsize polygon grid.  The dropsize calculations have
  // additional thresholds added to the regular grid array.

  unsigned char *dropsize_grid_array = new unsigned char[plane_size];
  long dropsize_num_pts_filled = num_pts_filled;
  
  memcpy((void *)dropsize_grid_array, (void *)grid_array,
	 plane_size * sizeof(unsigned char));
  
  if (!_getDropsizePolygonGrid(vlevel_num, dropsize_grid_array,
			       plane_size, dropsize_num_pts_filled))
  {
    delete [] grid_array;
    delete [] dropsize_grid_array;
    return false;
  }
  
  // Calculate the scan time offset for this elevation angle

  int volume_duration = master_hdr.time_end - master_hdr.time_begin;
  double scan_secs = (double)volume_duration / (double)dbz_field_hdr.nz;
  int scan_time =
    master_hdr.time_begin +
    (int)((scan_secs * (double)vlevel_num) + (scan_secs / 2.0));
  int scan_time_offset =
    scan_time - master_hdr.time_centroid;
  
  if (_params->debug_level >= Params::DEBUG_NORM)
  {
    cerr << "   volume duration = " << volume_duration << " secs" << endl;
    cerr << "   scan secs = " << scan_secs << " secs" << endl;
    cerr << "   scan time = " << scan_time << endl;
    cerr << "   scan time offset = " << scan_time_offset << " secs" << endl;
  }
  
  // Add the polygon statistics that aren't related to any of the fields

  int scan_mode = _mdvHandler.getScanMode();
  
  if (!_addPolygonStatistics(polygon, scan_time_offset,
			     vlevel_num, vlevel_ht, proj,
			     master_hdr.sensor_lat, master_hdr.sensor_lon,
			     master_hdr.sensor_alt, scan_mode,
			     grid_array, num_pts_filled))
  {
    return false;
  }
  
  /************************* UPDATE **********************************
   * Should we output statistics set to MISSING_VALUE if the polygon
   * doesn't have any points?
   */

  // If there aren't any points in the polygon, we don't have any data to
  // process

  if (num_pts_filled <= 0)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "No points in polygon" << endl;
    }
    
    delete [] grid_array;
    
    return true;
  }
  
  // Add the DBZ and ZDR statistics

  _addDbzZdrStatistics(vlevel_num, grid_array, dropsize_grid_array, polygon);
  
  // Add the statistics for each continuous field
  
  vector< FieldInfo > stat_fields = _mdvHandler.getStatFieldList();
  
  for (vector< FieldInfo >::const_iterator field_info = stat_fields.begin();
       field_info != stat_fields.end(); ++field_info)
  {
    _addFieldStatistics(*(field_info->getField()), vlevel_num,
			field_info->isLogField(),
			grid_array, polygon);
  } /* endfor - field_info */
  
  // Add the statistics for each discrete field
  
  vector< FieldInfo > discrete_fields = _mdvHandler.getDiscreteStatFieldList();
  
  for (vector< FieldInfo >::const_iterator field_info = discrete_fields.begin();
       field_info != discrete_fields.end(); ++field_info)
  {
    _addDiscreteFieldStatistics(*(field_info->getField()), vlevel_num,
			grid_array, polygon);
  } /* endfor - field_info */
  
  delete [] grid_array;
  
  return true;
}


/*********************************************************************
 * _addZdrFieldStatsToPolygon()
 */

double DrawFmqPolygonStats::_addZdrFieldStatsToPolygon(const string &field_name,
						       const string &field_units,
						       const vector< double > &values_linear,
						       const vector< double > &values_log,
						       const vector< double > &zh_values_linear,
						       const vector< double > &zv_values_linear,
						       GenPolyStats &polygon) const
{
  // If we are producing histograms, add this field

  if (_histogramFile != 0)
  {
    _calcHistogram(polygon, field_name, values_log, _histogramFile);
  }
  
  // Calculate all of the statistics except the standard deviation
  // which has to be handled specially

  double mean_value =
    _linearToLog(_calcMeanZdr(zh_values_linear, zv_values_linear));
  double median_value = _linearToLog(_calcMedian(values_linear));
  double min_value = _linearToLog(_calcMinimum(values_linear));
  double max_value = _linearToLog(_calcMaximum(values_linear));
  
  // Calculate the standard deviation value.  This is handled specially
  // because we have to use the actual log values rather than the linear
  // values when working in log space.

  double std_dev_value = _calcStdDev(values_log);

  // Now add the values to the polygon

  _addFieldValue(field_name, field_units, "mean", mean_value, polygon);
  _addFieldValue(field_name, field_units, "std dev", std_dev_value, polygon);
  _addFieldValue(field_name, field_units, "median", median_value, polygon);
  _addFieldValue(field_name, field_units, "min", min_value, polygon);
  _addFieldValue(field_name, field_units, "max", max_value, polygon);

  return mean_value;
}


/*********************************************************************
 * _applyThreshold()
 */

bool DrawFmqPolygonStats::_applyThreshold(const MdvxField &thresh_field,
					  const int vlevel_num,
					  const Params::thresh_compare_t comparison,
					  const double compare_value,
					  unsigned char *grid_array,
					  const size_t grid_size,
					  long &num_pts_filled) const
{
  static const string method_name = "DrawFmqPolygonStats::_applyThreshold()";

  if (_params->debug_level >= Params::DEBUG_VERBOSE)
  {
    cerr << "---> Entering " << method_name << endl;
    cerr << "     field_name = " << thresh_field.getFieldHeader().field_name << endl;
    cerr << "     compare_value = " << compare_value << endl;
  }
  
  // Remove grid points based on the threshold field

  Mdvx::field_header_t field_hdr = thresh_field.getFieldHeader();
  int plane_size = field_hdr.nx * field_hdr.ny;
  
  fl32 *thresh_data = (fl32 *)thresh_field.getVol();
  fl32 *thresh_data_ptr = thresh_data + (plane_size * vlevel_num);
  
  for (size_t i = 0; i < grid_size; ++i, ++thresh_data_ptr)
  {
    // We don't need to look at points that are outside of the polygon

    if (grid_array[i] == 0)
      continue;
      
    // Data values where the threshold value is missing shouldn't be
    // included in the statistics

    if (*thresh_data_ptr == field_hdr.missing_data_value ||
	*thresh_data_ptr == field_hdr.bad_data_value)
    {
      grid_array[i] = 0;
      num_pts_filled--;
      continue;
    }
      
    switch (comparison)
    {
    case Params::THRESH_GREATER_THAN :
      if (*thresh_data_ptr <= compare_value)
      {
	grid_array[i] = 0;
	num_pts_filled--;
	continue;
      }
      break;
	
    case Params::THRESH_GREATER_THAN_OR_EQUAL :
      if (*thresh_data_ptr < compare_value)
      {
	grid_array[i] = 0;
	num_pts_filled--;
	continue;
      }
      break;
	
    case Params::THRESH_LESS_THAN :
      if (*thresh_data_ptr >= compare_value)
      {
	grid_array[i] = 0;
	num_pts_filled--;
	continue;
      }
      break;
	
    case Params::THRESH_LESS_THAN_OR_EQUAL :
      if (*thresh_data_ptr > compare_value)
      {
	grid_array[i] = 0;
	num_pts_filled--;
	continue;
      }
      break;
	
    case Params::THRESH_EQUAL :
      if (*thresh_data_ptr != compare_value)
      {
	grid_array[i] = 0;
	num_pts_filled--;
	continue;
      }
      break;
	
    case Params::THRESH_NOT_EQUAL :
      if (*thresh_data_ptr == compare_value)
      {
	grid_array[i] = 0;
	num_pts_filled--;
	continue;
      }
      break;
	
    } /* endswitch - comparison */
      
  } /* endfor - i */
    
  if (_params->debug_level >= Params::DEBUG_VERBOSE)
    cerr << "     num_pts_filled = " << num_pts_filled << endl;
  
  return true;
}


/*********************************************************************
 * _calcHistogram()
 */

map< double, int > DrawFmqPolygonStats::_calcHistogram(const GenPolyStats &polygon,
						       const string &field_name,
						       const vector< double > &values,
						       FILE *histogram_file)
{
  // First, copy the values into a map, maintaining the number of times the
  // value was encountered.  Since we generally store radar data as scaled
  // values, we should encounter values multiple times.

  map < double, int > histogram;
  
  vector< double >::const_iterator value;
  for (value = values.begin(); value != values.end(); ++value)
  {
    map< double, int >::iterator value_pos = histogram.find(*value);
    
    if (value_pos == histogram.end())
    {
      histogram[*value] = 1;
    }
    else
    {
      histogram[*value] += 1;
    }
  } /* endfor - value */
  
  // Print out the histogram

  if (histogram_file != 0)
    _writeHistogramFile(histogram_file, polygon, field_name, histogram);
  
  return histogram;
}


/*********************************************************************
 * _calcMaximum()
 */

double DrawFmqPolygonStats::_calcMaximum(const vector< double > &values)
{
  // Check for an empty vector

  if (values.size() == 0)
    return MISSING_VALUE;
  
  // Find the maximum

  double value_maximum;
  
  vector< double >::const_iterator value = values.begin();
  
  value_maximum = *value;
  
  for (++value; value != values.end(); ++value)
  {
    if (value_maximum < *value)
      value_maximum = *value;
  }

  return value_maximum;
}


/*********************************************************************
 * _calcMean()
 */

double DrawFmqPolygonStats::_calcMean(const vector< double > &values)
{
  // Check for an empty vector

  if (values.size() == 0)
    return MISSING_VALUE;
  
  // Calculate the mean of the values

  vector< double >::const_iterator value;
  double value_sum = 0.0;
  
  for (value = values.begin(); value != values.end(); ++value)
    value_sum += *value;
  
  double value_mean;
  
  if (values.size() == 0)
    value_mean = 0.0;
  else
    value_mean = value_sum / (double)values.size();
  
  return value_mean;
}


/*********************************************************************
 * _calcMeanZdr()
 */

double DrawFmqPolygonStats::_calcMeanZdr(const vector< double > &zh_values,
					 const vector< double > &zv_values)
{
  // Check for empty vectors

  if (zh_values.size() == 0 || zv_values.size() == 0)
    return MISSING_VALUE;
  
  // We know that zh_values and zv_values have the same number of elements,
  // so we don't need to check that here.

  size_t num_values = zh_values.size();
  double zh_sum = 0.0;
  double zv_sum = 0.0;
  
  for (size_t i = 0; i < num_values; ++i)
  {
    zh_sum += zh_values[i];
    zv_sum += zv_values[i];
  }
  
  double value_mean;
  
  if (num_values == 0)
    value_mean = 0.0;
  else
    value_mean = zh_sum / zv_sum;
  
  return value_mean;
}


/*********************************************************************
 * _calcStdDev()
 */

double DrawFmqPolygonStats::_calcStdDev(const vector< double > &values)
{
  // Check for an empty vector

  if (values.size() == 0)
    return MISSING_VALUE;
  
  // Calculate the mean

  double value_mean = _calcMean(values);
  
  // Calculate the standard deviation using the mean

  vector< double >::const_iterator value;
  double value_diff_sum = 0.0;
  
  for (value = values.begin(); value != values.end(); ++value)
  {
    double value_diff = *value - value_mean;
    value_diff_sum += value_diff * value_diff;
  }
  
  double value_std_dev;
  
  if (values.size() == 0)
    value_std_dev = 0.0;
  else
    value_std_dev = sqrt(value_diff_sum / (double)values.size());

  return value_std_dev;
}


/*********************************************************************
 * _calcMedian()
 */

double DrawFmqPolygonStats::_calcMedian(const vector< double > &values)
{
  // Check for an empty vector
  
  if (values.size() == 0)
    return MISSING_VALUE;
  
  // Find the median

  vector< double > local_values = values;
  sort(local_values.begin(), local_values.end());
  
  double value_median;
  
  if (local_values.size() == 0)
    value_median = 0.0;
  else if (local_values.size() % 2 == 0)
    value_median =
      (local_values[local_values.size() / 2] +
       local_values[(local_values.size() / 2) - 1]) / 2.0;
  else
    value_median = local_values[local_values.size() / 2];
  
  return value_median;
}


/*********************************************************************
 * _calcMinimum()
 */

double DrawFmqPolygonStats::_calcMinimum(const vector< double > &values)
{
  // Check for an empty vector

  if (values.size() == 0)
    return MISSING_VALUE;
  
  // Find the minimum

  double value_minimum;
  
  vector< double >::const_iterator value = values.begin();
  
  value_minimum = *value;
  
  for (++value; value != values.end(); ++value)
  {
    if (value_minimum > *value)
      value_minimum = *value;
  }

  return value_minimum;
}


/*********************************************************************
 * _calcMode()
 */

vector< double > DrawFmqPolygonStats::_calcMode(const map< double, int > &histogram)
{
  // Initialize the return vector.

  vector< double > modes;
  
  // Check for an empty vector of data values.  At this point the modes
  // vector will also be empty, so we just return it.
  
  if (histogram.size() == 0)
    return modes;
  
  // First, loop through the map to find the greatest frequency for the data
  // values

  int max_values = 1;
  
  for (map< double, int >::const_iterator entry = histogram.begin();
       entry != histogram.end(); ++entry)
  {
    if (max_values < entry->second)
      max_values = entry->second;
  }
  
  // Collect the mode values and return them

  map< double, int >::const_iterator hist_iter;
  for (hist_iter = histogram.begin(); hist_iter != histogram.end(); ++hist_iter)
  {
    if ((*hist_iter).second == max_values)
      modes.push_back((*hist_iter).first);
  }
  
  return modes;
}


/*********************************************************************
 * _createPolygon()
 */

bool DrawFmqPolygonStats::_createPolygon(const Human_Drawn_Data_t &input_prod)
{
  static const string method_name = "DrawFmqPolygonStats::_createPolygon()";
  
  // Create the GenPolyStats object for this message

  GenPolyStats polygon;
  
  polygon.setName(input_prod.prod_label);
  polygon.setId(input_prod.id_no);
  polygon.setTime(input_prod.data_time);
  polygon.setExpireTime(input_prod.data_time + input_prod.valid_seconds);
  polygon.setNLevels(1);
  polygon.setClosedFlag(true);

  for (int i = 0; i < input_prod.num_points; ++i)
  {
    // When we get a pen-up, end the polygon.

    if (input_prod.lat_points[i] < -90.1)
      break;
    
    GenPoly::vertex_t vertex;
    
    vertex.lat = input_prod.lat_points[i];
    vertex.lon = input_prod.lon_points[i];
    polygon.addVertex(vertex);
      
  } /* endfor - i */
  
  // Add the statistics to the polygon

  if (!_addStatistics(polygon, input_prod.vlevel_ht_cent))
    return false;
  
  // Write the polygon to the output database

  DsSpdb spdb;
  spdb.setPutMode(Spdb::putModeOver);
  spdb.addUrl(_params->output_spdb_url);
  
  polygon.assemble();
  
  if (spdb.put(SPDB_GENERIC_POLYLINE_ID,
	       SPDB_GENERIC_POLYLINE_LABEL,
 	       input_prod.id_no,
	       input_prod.data_time,
	       input_prod.data_time + input_prod.valid_seconds,
	       polygon.getBufLen(), polygon.getBufPtr()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing polygon to output SPDB database: "
	 << _params->output_spdb_url << endl;
    cerr << spdb.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _getDropsizePolygonGrid()
 */

bool DrawFmqPolygonStats::_getDropsizePolygonGrid(const int vlevel_num,
						  unsigned char *grid_array,
						  const size_t grid_size,
						  long &num_pts_filled) const
{
  static const string method_name = "DrawFmqPolygonStats::_getDropsizePolygonGrid()";

  // Make sure we have the right number of threshold fields

  int num_dropsize_thresh_fields = _mdvHandler.getNumDropsizeThreshFields();
  
  if (num_dropsize_thresh_fields != _params->dropsize_thresh_fields_n)
  {
    cerr << "PROGRAMMING ERROR: " << method_name << endl;
    cerr << "Wrong number of dropsize fields read in" << endl;
    cerr << "Looking for " << _params->dropsize_thresh_fields_n
	 << " fields" << endl;
    cerr << "Found " << num_dropsize_thresh_fields << " fields." << endl;
    cerr << "Report this to your programmer." << endl;
    
    return false;
  }
  
  // Look at each of the additional dropsize threshold fields

  for (int field_num = 0; field_num < _params->dropsize_thresh_fields_n;
       ++field_num)
  {
    const MdvxField *field = _mdvHandler.getDropsizeThreshField(field_num);
    
    if (!_applyThreshold(*field, vlevel_num,
			 _params->_dropsize_thresh_fields[field_num].comparison,
			 _params->_dropsize_thresh_fields[field_num].value,
			 grid_array, grid_size, num_pts_filled))
      return false;
  } /* endfor - field_num */
  
  return true;
}


/*********************************************************************
 * _getPolygonGrid()
 */

bool DrawFmqPolygonStats::_getPolygonGrid(const MdvxPjg &proj,
					  const int vlevel_num,
					  const GenPolyStats &polygon,
					  unsigned char *grid_array,
					  long &num_pts_filled) const
{
  static const string method_name = "DrawFmqPolygonStats::_getPolygonGrid()";
  
  // Grid the polygon on the field projection

  int num_vertices = polygon.getNumVertices();
  Point_d *vertices = new Point_d[num_vertices + 1];
  
  for (int i = 0; i < num_vertices; ++i)
  {
    GenPoly::vertex_t poly_vertex = polygon.getVertex(i);
    proj.latlon2xy(poly_vertex.lat, poly_vertex.lon,
		   vertices[i].x, vertices[i].y);
  } /* endfor - i */
  
  vertices[num_vertices] = vertices[0];
  
  int plane_size = proj.getNx() * proj.getNy();
  memset(grid_array, 0, plane_size * sizeof(unsigned char));
  
  num_pts_filled =
    EG_fill_polygon(vertices, num_vertices+1,
		    proj.getNx(), proj.getNy(),
		    proj.getMinx(), proj.getMiny(),
		    proj.getDx(), proj.getDy(),
		    grid_array, 1);
  
  // Remove grid points based on the threshold field

  for (int field_num = 0; field_num < _params->threshold_fields_n;
       ++field_num)
  {
    const MdvxField *field = _mdvHandler.getThresholdField(field_num);
    
    if (!_applyThreshold(*field, vlevel_num,
			 _params->_threshold_fields[field_num].comparison,
			 _params->_threshold_fields[field_num].value,
			 grid_array, plane_size, num_pts_filled))
      return false;
  } /* endfor - field_num */
  
  return true;
}


/*********************************************************************
 * _processInput()
 */

bool DrawFmqPolygonStats::_processInput(const Human_Drawn_Data_t &input_prod)
{
  static const string method_name = "DrawFmqPolygonStats::_processInput()";
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << "Processing input..." << endl;
    
  if (_params->debug_level >= Params::DEBUG_NORM)
  {
    cerr << "   issueTime = " << DateTime::str(input_prod.issueTime) << endl;
    cerr << "   data_time = " << DateTime::str(input_prod.data_time) << endl;
    cerr << "   id_no = " << input_prod.id_no << endl;
    cerr << "   valid_seconds = " << input_prod.valid_seconds << endl;
    cerr << "   num_points = " << input_prod.num_points << endl;
    cerr << "   vlevel_num = " << input_prod.vlevel_num << endl;
    cerr << "   vlevel_ht_min = " << input_prod.vlevel_ht_min << endl;
    cerr << "   vlevel_ht_cent = " << input_prod.vlevel_ht_cent << endl;
    cerr << "   vlevel_ht_max = " << input_prod.vlevel_ht_max << endl;
    cerr << "   id_label = " << input_prod.id_label << endl;
    cerr << "   prod_label = " << input_prod.prod_label << endl;
    cerr << "   sender = " << input_prod.sender << endl;
    for (int i = 0; i < input_prod.num_points; ++i)
      cerr << "   lat = " << input_prod.lat_points[i]
	   << ", lon = " << input_prod.lon_points[i] << endl;
  }
    
  // Read in the MDV data file

  if (!_mdvHandler.getData(input_prod.data_time))
  {
    return false;
  }
  
  // Create the polygons.  If the elevation angle is set to -1, then
  // we need to create polygons for each elevation angle in the MDV file.

  if (input_prod.vlevel_num < 0)
  {
    Mdvx::field_header_t dbz_field_hdr =
      _mdvHandler.getDbzField()->getFieldHeader();
    Mdvx::vlevel_header_t dbz_vlevel_hdr =
      _mdvHandler.getDbzField()->getVlevelHeader();
    
    // Make a copy of the product so we don't change the input message.

    Human_Drawn_Data_t input_prod_copy = input_prod;
    input_prod_copy.lat_points = new double[input_prod.num_points];
    input_prod_copy.lon_points = new double[input_prod.num_points];
    memcpy(input_prod_copy.lat_points, input_prod.lat_points,
	   input_prod.num_points * sizeof(double));
    memcpy(input_prod_copy.lon_points, input_prod.lon_points,
	   input_prod.num_points * sizeof(double));
  
    int id_no = input_prod.id_no;
    
    // Create a polygon for each vertical level.  Be sure to update the
    // ID number so we don't overwrite other polygons.

    for (int z = 0; z < dbz_field_hdr.nz; ++z)
    {
      input_prod_copy.id_no = id_no + z;
      input_prod_copy.vlevel_num = z;
      input_prod_copy.vlevel_ht_min = dbz_vlevel_hdr.level[z];
      input_prod_copy.vlevel_ht_cent = dbz_vlevel_hdr.level[z];
      input_prod_copy.vlevel_ht_max = dbz_vlevel_hdr.level[z];
      
      if (!_createPolygon(input_prod_copy))
      {
	delete [] input_prod_copy.lat_points;
	delete [] input_prod_copy.lon_points;
	
	return false;
      }
      
    } /* endfor - z */
    
    delete [] input_prod_copy.lat_points;
    delete [] input_prod_copy.lon_points;
    
  }
  else
  {
    if (!_createPolygon(input_prod))
      return false;
  }
  
  return true;
}


/*********************************************************************
 * _writeHistogramFile()
 */

void DrawFmqPolygonStats::_writeHistogramFile(FILE *histogram_file,
					      const GenPolyStats &polygon,
					      const string &field_name,
					      const map< double, int > &histogram)
{
  static const string method_name = "DrawFmqPolygonStats::_writeHistogramFile()";
  
  fprintf(histogram_file, "\nTime: %s\n",
	  DateTime::str(polygon.getTime()).c_str());
  fprintf(histogram_file, "Polygon name: %s\n", polygon.getName().c_str());
  fprintf(histogram_file, "Polygon ID: %d\n", polygon.getId());
  fprintf(histogram_file, "Field name: %s\n", field_name.c_str());
  
  map< double, int >::const_iterator hist_iter;
  for (hist_iter = histogram.begin(); hist_iter != histogram.end();
       ++hist_iter)
    fprintf(histogram_file, "%10.2f %5d\n",
	    (*hist_iter).first, (*hist_iter).second);
}
