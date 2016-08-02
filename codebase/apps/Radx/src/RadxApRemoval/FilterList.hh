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
/**
 *
 * @file FilterList.hh
 *
 * @class FilterList
 *
 * FilterList is a class the controls access to a list of filters.
 *  
 * @date 7/17/2008
 *
 */

#ifndef FilterList_HH
#define FilterList_HH

using namespace std;

#include <vector>

#include "Filter.hh"


/** 
 * @class FilterList
 */

class FilterList
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  FilterList();

  /**
   * @brief Destructor
   */

  virtual ~FilterList();
   
  /**
   * @brief Add a filter to the list.
   *
   * @param[in] filter Pointer to the filter to be added to the list.
   *
   * @note Note that this class assumes that the calling method retains
   *       control of the Filter pointer and will delete the pointer when
   *       processing is finished.  This class does NOT delete the Filter
   *       pointer.
   */

  void addFilter(Filter *filter);
  
   
  /**
   * @brief Set up for the current tilt in all of the filters.
   *
   * @param[in] n_gates Number of gates in the current tilt.
   */

  void setTilt(const int n_gates);


  /**
   * @brief Clear the filter field information in all of the filters.
   */

  void clearFieldInfo();


  /**
   * @brief Set up the filter field information in all of the filters.
   *
   * @param[in] index Field index of current input field.
   * @param[in] missing_val Scaled missing data value for current input field.
   */

   void setFieldInfo(const int index, const int missing_val);


  /**
   * @brief Set up for the current beam.
   *
   * @param[in] azimuth Azimuth angle for current beam.
   */

  void setBeam(const double azimuth);


  /**
   * @brief Set terrain value.
   *
   * @param[in] terrain_val Terrain value.
   */

  void setTerrain(const float terrain_val);


  /**
   * @brief Calculate the interest values.
   *
   * @param[in] int_type Interest field type.
   * @param[in] field_val Value of feature field for which to calculate
   *                      interest.
   * @param[in] ibeam Current beam index.
   * @param[in] igate Current gate index.
   */

  void calcInterest(const FilterBeamInfo::InterestType int_type,
		    const double field_val,
		    const int ibeam, const int igate);


  /**
   * @brief Calculate the final interest values.
   *
   * @param[in] ibeam Current beam index.
   * @param[in] igate Current gate index.
   * @param[in] dbz_val Reflectivity value at current location.
   */

  void calcFinal(const int ibeam, const int igate, const float dbz_val);


  /**
   * @brief Filter the data.
   *
   * @param[in] data_ptrs Pointers to the data to be filtered.
   */

  void filterData(vector< DsBeamData > &data_ptrs);


  /**
   * @brief Write a start of volume flag to the filter output queue.
   *
   * @note Volume start and end flags must be written separately from the
   *       method that writes the data because we are not guaranteed that
   *       we will be filtering a tilt that has the start/end of volume
   *       flags attached.
   *
   * @param[in] start_time Volume start time.
   * @param[in] volume_num Volume number.
   */

  void putStartOfVolume(const time_t start_time, const int volume_num);


  /**
   * @brief Write out interest data for the current tilt.
   *
   * @param[in] start_time Tilt start time.
   * @param[in] end_time Tilt end time.
   * @param[in] sample_params Sample radar params.
   * @param[in] elev_angle Tilt elevation angle.
   * @param[in] volume_num Volume number.
   * @param[in] tilt_num Tilt number.
   *
   * @return 0 on success, -1 on error.
   */

  int writeInterest(const time_t start_time, const time_t end_time, 
		    DsRadarParams& sample_params, const double elev_angle,
		    const int volume_num, const int tilt_num);


  /**
   * @brief Write an end of volume flag to the filter output queue.
   *
   * @note Volume start and end flags must be written separately from the
   *       method that writes the data because we are not guaranteed that
   *       we will be filtering a tilt that has the start/end of volume
   *       flags attached.
   *
   * @param[in] end_time Volume end time.
   * @param[in] volume_num Volume number.
   */

  void putEndOfVolume(const time_t end_time, const int volume_num);


private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief _filterList is the vector containing the pointers to the Filter
   *                    objects.
   */

  vector< Filter* > _filterList;

  /**
   * @brief List of input fields that are to be filtered.  The index is
   *        the index of the input field in the field data and the value
   *        is the scaled missing data value for that field.
   *
   * @todo The missing data value in this map is actually never used in this
   *       class.  This can be changed to a vector of input field indices to
   *       make the code more maintainable.
   */

  map< int, int, less<int> > _fieldInfo;


};

#endif
