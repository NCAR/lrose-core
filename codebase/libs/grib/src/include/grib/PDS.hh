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
///////////////////////////////////////////////
// PDS - Product Definition Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
//////////////////////////////////////////////
#ifndef _PDS_
#define _PDS_

#include <grib/GribSection.hh>
#include <grib/GribVertType.hh>
#include <toolsa/DateTime.hh>

using namespace std;

class PDS : public GribSection {

public:
   
  typedef enum {
    VL_SURFACE = 1,
    VL_SIGMA_P = 2,
    VL_PRESSURE = 3,
    VL_Z = 4,
    VL_SIGMA_Z = 5,
    VL_ETA = 6,
    VL_THETA = 7,
    VL_MIXED = 8,
    VL_ELEV = 9,
    VL_COMPOSITE = 10,
    VL_CROSS_SEC = 11,
    VL_SATELLITE_IMAGE = 12,
    VL_VARIABLE_ELEV = 13,
    VL_FIELDS_VAR_ELEV = 14,
    VL_FLIGHT_LEVEL = 15
  } vertical_level_t;
   
  typedef enum
  {
    TIME_UNIT_MINUTES = 0,
    TIME_UNIT_HOURS = 1,
    TIME_UNIT_DAYS = 2,
    TIME_UNIT_MONTHS = 3,
    TIME_UNIT_YEARS = 4,
    TIME_UNIT_DECADES = 5,
    TIME_UNIT_NORMAL = 6,     // 30 years
    TIME_UNIT_CENTURIES = 7,
    
    TIME_UNIT_3HOURS = 10,
    TIME_UNIT_6HOURS = 11,
    TIME_UNIT_12HOURS = 12,

    TIME_UNIT_SECONDS = 254
  } time_period_units_t;

  typedef enum
  {
    TIME_RANGE_NORMAL = 0,
    TIME_RANGE_INITIALIZED_ANALYSIS = 1,
    TIME_RANGE_VALID_TIME_RANGE = 2,
    TIME_RANGE_AVERAGE = 3,
    TIME_RANGE_ACCUMULATION = 4,
    TIME_RANGE_DIFFERENCE = 5,
    P1_OCCUPIES_19_AND_20 = 10,
    TIME_RANGE_CLIMO_MEAN_VALUE = 51,
    TIME_RANGE_AVG_N_FCSTS_1 = 113,
    TIME_RANGE_ACCUM_N_FCSTS_1 = 114,
    TIME_RANGE_AVG_N_FCSTS_2 = 115,
    TIME_RANGE_ACCUM_N_FCSTS_2 = 116,
    TIME_RANGE_AVG_N_FCSTS_3 = 117,
    TIME_RANGE_TEMPORAL_VARIANCE = 118,
    TIME_RANGE_AVG_N_UNINIT_ANALYSES = 123,
    TIME_RANGE_ACCUM_N_UNINIT_ANALYSES = 124
  } time_range_indicator_t;

  PDS();
  ~PDS(){};
   
  int unpack( ui08 *pdsPtr );
  int pack( ui08 *pdsPtr ) ;

  void getTime(DateTime &generate_time, int &forecast_secs) const;
  
  int getForecastTime() const;
  time_t getGenerateTime() const;

  void setTime(const DateTime &generate_time,
	       const int forecast_period,
	       const time_period_units_t forecast_period_units);
  void setTime(const DateTime &generate_time,
	       const int time_period1, const int time_period2,
	       const time_period_units_t forecast_period_units);
  
  inline int getDecimalScale() const { return( _decimalScale ); }
  inline int getGridId() const { return( _gridId ); }
  inline int getParameterId() const { return( _parameterId ); }
  inline int getLevelId() const { return (int)_vertType.getLevelType(); }
  inline string getLongName() const { return( _parameterLongName ); }
  inline string getName() const { return( _parameterName ); }
  inline string getUnits() const { return( _parameterUnits ); }

  inline GribVertType getVertType() const { return _vertType; }
  
  inline GribVertType::vert_type_t getVerticalLevelType() const { return _vertType.getLevelType(); }
  inline bool isOneLevelVal() const { return _vertType.isSingleLevelValue(); }
  inline int getLevelVal() const { return _vertType.getLevelValue(); }
  inline int getLevelValTop() const { return _vertType.getLevelValueTop(); }
  inline int getLevelValBottom() const { return _vertType.getLevelValueBottom(); }
  
  inline bool gdsUsed() const { return( _gdsPresent ); }
  inline bool bmsUsed() const { return( _bmsPresent ); }

  inline void setGdsUsed(const bool gds_used)
    { _gdsPresent = gds_used; }
  inline void setBmsUsed(const bool bms_used)
    { _bmsPresent = bms_used; }

  inline void setGribTablesVersion(const int grib_tables_version)
    { _tableVersion = grib_tables_version; }
  inline void setOriginatingCenter(const int originating_center)
    { _centerId = originating_center; }
  inline void setSubcenterId(const int subcenter_id)
    { _subCenterId = subcenter_id; }
  inline void setGeneratingProcessId(const int generating_process_id)
    { _processGenId = generating_process_id; }
  inline void setParameterId(const int parameter_id)
    { _parameterId = parameter_id; }
  inline void setTimeRangeId(const int time_range_id)
    { _timeRangeId = time_range_id; }
  inline void setExpectedSize(const int size)
    { _expectedSize = size; }
  
  
  inline void setGridId(const int grid_id)
    { _gridId = grid_id; }
  
  inline void setVertLevel(const GribVertType::vert_type_t vert_level_type,
			   const int vert_level_value_top,
			   const int vert_level_value_bottom)
  {
    _vertType.set(vert_level_type,
		  vert_level_value_top, vert_level_value_bottom);
  }
  
  inline void setDecimalScale(const int decimal_scale)
    { _decimalScale = decimal_scale; }
  
  //
  // add static methods so that field descriptions can be 
  // retrieved from _parmTable
  //
  static char* getLongName(const int& id, const int &layer_type, const int &v1, const int &v2);
  static const char* getName(const int& id);
  static const char* getUnits(const int& id);

  void print(FILE *) const;
  void print(ostream &stream) const;

  // ensemble data
  bool isEnsemble();
  int getEnsembleProduct();

private:

  static const int NUM_SECTION_BYTES = 28;
  static const int EXPECTED_SIZE = 40;
  int   _expectedSize;
  int   _tableVersion;
  int   _centerId;
  int   _processGenId;
  int   _gridId;
  int   _parameterId;
  string _parameterLongName;
  string _parameterName;
  string _parameterUnits;
  
  bool  _gdsPresent;
  bool  _bmsPresent;
   
  GribVertType _vertType;
  
  int   _year;
  int   _month;
  int   _day;
  int   _hour;
  int   _min;
   
  int   _forecastUnitId;
  int   _forecastPeriod1;
  int   _forecastPeriod2;
   
  int   _timeRangeId;
  int   _avgNum;
  int   _avgNumMissing;
   
  int   _subCenterId;
  int   _decimalScale;
   
  typedef struct {
    string name;
    string long_name;
    string units;
  } _ParmTable;

  static const _ParmTable _parmTable[256];
  static const int _parmTableSize;
  
  void _levels(ui08 pds_b9, ui08 pds_b10, ui08 pds_b11);
  void _packLevels(ui08 *buffer);

};

#endif

   
