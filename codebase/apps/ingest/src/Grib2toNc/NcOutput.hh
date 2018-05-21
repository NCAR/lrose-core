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
///////////////////////////////////////////////////
// NcOutput.hh 
//
// Jason Craig  -  Jan 2015
///////////////////////////////////////////////////
#ifndef _NC_OUTPUT_
#define _NC_OUTPUT_

#include <Ncxx/Nc3File.hh>

#include "Params.hh"
#include "Grib2Nc.hh"

class NcOutput {
  
public:

  NcOutput( Params *params, char* outputFile );
 
  ~NcOutput();


  void addField(fl32 * field, Grib2Nc::FieldInfo fieldInfo);

  int writeNc(time_t gen_time, long int lead_secs );

  void clear();

  int numFields();

  const static char* units_missing;
  const static char* _360_day;
  const static char* _365_day;
  const static char* _366_day;
  const static char* FillValue;
  const static char* add_offset;
  const static char* albers_conical_equal_area;
  const static char* all_leap;
  const static char* ancillary_variables;
  const static char* area;
  const static char* axis;
  const static char* azimuthal_equidistant;
  const static char* bounds;
  const static char* calendar;
  const static char* central_latitude;
  const static char* central_longitude;
  const static char* cell_measures;
  const static char* cell_methods;
  const static char* cf_version;
  const static char* comment;
  const static char* compress;
  const static char* conventions;
  const static char* coordinates;
  const static char* degrees;
  const static char* degrees_east;
  const static char* degrees_north;
  const static char* detection_minimum;
  const static char* down;
  const static char* earth_radius;
  const static char* false_easting;
  const static char* false_northing;
  const static char* flag_meanings;
  const static char* flag_values;
  const static char* formula_terms;
  const static char* forecast_period;
  const static char* forecast_period_long;
  const static char* forecast_reference_time;
  const static char* forecast_reference_time_long;
  const static char* gregorian;
  const static char* grid_latitude;
  const static char* grid_longitude;
  const static char* grid_mapping;
  const static char* grid_mapping_attribute;
  const static char* grid_mapping_name;
  const static char* grid_north_pole_latitude;
  const static char* grid_north_pole_longitude;
  const static char* history;
  const static char* hybrid_level_standard_name;
  const static char* institution;
  const static char* inverse_flattening;
  const static char* julian;
  const static char* lambert_azimuthal_equal_area;
  const static char* lambert_conformal_conic;
  const static char* latitude;
  const static char* latitude_longitude;
  const static char* latitude_of_projection_origin;
  const static char* layer;
  const static char* leap_month;
  const static char* leap_year;
  const static char* level;
  const static char* long_name;
  const static char* longitude;
  const static char* longitude_of_central_meridian;
  const static char* longitude_of_prime_meridian;
  const static char* longitude_of_projection_origin;
  const static char* maximum;
  const static char* mean;
  const static char* median;
  const static char* mercator;
  const static char* mid_range;
  const static char* minimum;
  const static char* missing_value;
  const static char* mode;
  const static char* month_lengths;
  const static char* noleap;
  const static char* none;
  const static char* number_of_observations;
  const static char* perspective_point_height;
  const static char* point;
  const static char* polar_radar;
  const static char* polar_stereographic;
  const static char* positive;
  const static char* projection_x_coordinate;
  const static char* projection_y_coordinate;
  const static char* proleptic_gregorian;
  const static char* references;
  const static char* reference_date;
  const static char* region;
  const static char* rotated_latitude_longitude;
  const static char* lat_rotated_pole;
  const static char* lon_rotated_pole;
  const static char* scale_factor;
  const static char* scale_factor_at_central_meridian;
  const static char* scale_factor_at_projection_origin;
  const static char* seconds;
  const static char* secs_since_jan1_1970;
  const static char* semi_major_axis;
  const static char* semi_minor_axis;
  const static char* sigma_level;
  const static char* source;
  const static char* standard;
  const static char* standard_deviation;
  const static char* standard_error;
  const static char* standard_name;
  const static char* standard_parallel;
  const static char* start_time;
  const static char* status_flag;
  const static char* stereographic;
  const static char* stop_time;
  const static char* straight_vertical_longitude_from_pole;
  const static char* sum;
  const static char* time;
  const static char* time_long;
  const static char* time_bounds;
  const static char* title;
  const static char* transverse_mercator;
  const static char* units;
  const static char* up;
  const static char* valid_max;
  const static char* valid_min;
  const static char* valid_range;
  const static char* variance;
  const static char* vertical;
  const static char* vertical_perspective;
  const static char* volume;

protected:
  
private:

  typedef struct {
    double ht;
    int indexLower;
    int indexUpper;
    double ghtLower;
    double ghtUpper;
    double wtLower;
    double wtUpper;
  } interp_pt_t;

  Params *_params;

  char* _outputFile;

  Nc3File *_ncFile;
  Nc3Error *_ncErr;

  Nc3Dim *_timeDim;
  Nc3Dim *_boundsDim;
  Nc3Var *_timeVar;
  Nc3Var *_forecastPeriodVar;
  Nc3Var *_forecastReferenceVar;

  vector <Grib2Nc::FieldInfo> _fieldInfo;
  vector <fl32 *> _fieldData;
  int _numUniqueGrid;
  vector <int> _uniqueGrid;
  int _numUniqueVertical;
  vector <int> _uniqueVertical;

  vector <Nc3Var *> _fieldVar;

  vector <Nc3Dim *> _uniqueGridxDim;
  vector <Nc3Dim *> _uniqueGridyDim;
  vector <Nc3Var *> _uniqueGridxVar;
  vector <Nc3Var *> _uniqueGridyVar;
  vector <Nc3Var *> _uniqueGridlatVar;
  vector <Nc3Var *> _uniqueGridlonVar;
  vector <Nc3Var *> _uniqueGridprojVar;

  vector <Nc3Dim *> _uniqueVerticalzDim;
  vector <Nc3Var *> _uniqueVerticalzVar;


  int _openNcFile(const string &path);
  void _closeNcFile();
  int _writeNcFile(time_t genTime, long int leadSecs);
  int _addGlobalAttributes();
  int _addDimensions();
  int _addTimeVariables(time_t genTime, long int leadSecs);
  int _addCoordinateVariables();
  int _addProjectionVariables();
  int _addFieldDataVariables();
  int _putTimeVariables(time_t genTime, long int leadSecs);
  int _putCoordinateVariables();
  int _putFieldDataVariables();

  void _remap(fl32 *data, Grib2Nc::FieldInfo* fieldInfo);

  //void _remapLambertLambert(Grib2Nc::FieldInfo, fl32* data);

  //float _interp2(Grib2Nc::FieldInfo, 
  //               double x, double y, int z, float *field);

};

#endif
