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
/////////////////////////////////////////////
// gribrecord - Main class for manipulating GRIB records.
//
////////////////////////////////////////////

#ifndef gribrecord_HH
#define gribrecord_HH

#include <grib/IdSec.hh>
#include <grib/PDS.hh>
#include "gds_.hh"
#include <grib/BMS.hh>
#include <grib/BDS.hh>
#include <grib/ES.hh>
#include <toolsa/DateTime.hh>
#include "gds_equidistant.hh"
#include "gds_lambert.hh"


using namespace std;

class gribrecord
{

public:

  gribrecord();
  virtual ~gribrecord();
  
  int unpack(ui08 **filePtr);
  ui08 *pack();
  
  void print(FILE *, const bool print_bitmap_flag = false,
	     const bool print_data_flag = false,
	     const bool print_min_datum_flag = false,
	     const bool print_max_datum_flag = false) const;
  
  // Get methods

  inline int getRecordSize() const { return _is.getTotalSize(); }
  
  inline int getParameterId() const { return _pds.getParameterId(); }
  inline int getLevelId() const { return _pds.getLevelId(); }
  
  inline void getTime(DateTime &generate_time,
		      int &forecast_secs) const
  {
    _pds.getTime(generate_time, forecast_secs);
  }
  
  const fl32 *getData(gds::gds_t &projection,
		      GribVertType &vert_type) const;
  
  // Set methods

  inline void setGribTablesVersion(const int grib_tables_version)
    { _pds.setGribTablesVersion(grib_tables_version); }
  inline void setOriginatingCenter(const int originating_center)
    { _pds.setOriginatingCenter(originating_center); }
  inline void setGeneratingProcessId(const int generating_process_id)
    { _pds.setGeneratingProcessId(generating_process_id); }

  inline void setGribCode(const int grib_code)
    { _pds.setParameterId(grib_code); }
  
  inline void setTime(const DateTime &generate_time,
		      const int forecast_period,
		      const PDS::time_period_units_t forecast_period_units)
    { _pds.setTime(generate_time, forecast_period, forecast_period_units); }
  
  inline void setTimeRangeId(const int& time_range_id) 
  { 
    _pds.setTimeRangeId(time_range_id); 
  }

  inline void setGridId(const int& grid_id) 
  { 
    _pds.setGridId(grid_id); 
  }

  inline void setGridOrientation(const gds::grid_orientation_t& orientation) 
  { 
    _gds_ptr->setGridOrientation(orientation); 
  }

  inline void setDataOrdering(const gds::data_ordering_t& ordering) 
  { 
    _gds_ptr->setDataOrdering(ordering); 
  }

  void setData(const gds::gds_t &projection,
	       const GribVertType::vert_type_t vert_level_type,
	       const int vert_level_value_top,
	       const int vert_level_value_bottom,
	       const double min_data_value, const double max_data_value,
	       const int precision,
	       const int max_bit_len,
	       fl32 *new_data,
	       ui08 *bitmap = 0);
  
  inline void setPdsExpectedSize(const int size)
    { _pds.setExpectedSize(size); }

  inline void setGdsExpectedSize(const int size)
    { _gds_ptr->setExpectedSize(size); }
  
//   void setDataScaling(const int decimal_scale,
// 		      const int bit_len);
  
  
private:
  
  IdSec _is;
  PDS   _pds;
  gds   *_gds_ptr;
  BMS   _bms;
  BDS   _bds;
  ES    _es;
  
  void _setGdsClass(int gridType)
  {
    if(_gds_ptr != NULL) {
      
    }
    switch ( gridType )
      {
      case gds::EQUIDISTANT_CYL_PROJ_ID:
	_gds_ptr = new gds_equidistant();
        break;
      case gds::LAMBERT_CONFORMAL_PROJ_ID:
	_gds_ptr = new gds_lambert();
	break;
      default:
	cout << "ERROR: Invalid or unimplemented grid projection ID." << endl;
      }
    
  }

  void _resetRecordSize()
  {
    int total_size = _is.getSize() + _pds.getSize() +
      _bds.getSize() + _es.getSize();
    
    if (_pds.gdsUsed() && _gds_ptr != NULL) total_size += _gds_ptr->getSize();
    if (_pds.bmsUsed()) total_size += _bms.getSize();
    
    _is.setTotalSize(total_size);
  }
  
};

#endif
