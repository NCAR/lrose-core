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
 * @file Filter.hh
 *
 * @class Filter
 *
 * Filter is a class that handles the filtering of radar data
 *  
 * @date 9/18/2002
 *
 */

#ifndef Filter_HH
#define Filter_HH

using namespace std;

#include <vector>
#include <map>

#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsBeamData.hh>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsFieldParams.hh>

#include "FilterBeamInfo.hh"
#include "InterestFunction.hh"


/** 
 * @class Filter
 */

class Filter 
{
  public:

  //////////////////
  // Public types //
  //////////////////

  /** 
   * @brief Type of terrain over which to calculate a given interest field.
   */

  enum TerrainType{ LAND, WATER, ALL };


  /** 
   * @brief Type of combination to use when combining the final interest
   *        value for this filter with the final interest for other filters.
   */

  enum CombineType{ COMBINE_AND, COMBINE_OR };
  


  //////////////////////
  // Public constants //
  //////////////////////

  /** 
   * @brief A radar params message should be included in the output FMQ
   *        once every RADAR_PARAMS_COUNT output beams.
   */

  static const int    RADAR_PARAMS_COUNT;

  /** 
   * @brief A summary debug message is printed once every RADAR_SUMMARY_COUNT
   *        output beams.
   */

  static const int    RADAR_SUMMARY_COUNT;


  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  Filter();

  /**
   * @brief Destructor
   */

  virtual ~Filter();

  /**
   * @brief Initialize the parameters that remain constant throughout the
   *        whole time the object is in memory
   *
   * @param[in] gate_spacing Gate spacing used for the input and output data.
   * @param(in] final_threshold Threshold value used for applying the results
   *                            of this filter.
   * @param[in] azimuth_radius
   * @param[in] terrain_use Enumerated value indicating where in the terrain
   *                        to apply the filter (land, water, all)
   * @param[in] interest_queue Output FMQ to use for storing the interest
   *                           fields associated with this filter.  Note that
   *                           the calling method retains control of this
   *                           pointer and must delete the pointer when
   *                           processing is finished.
   * @param[in] confidence_queue Output FMQ to use for storing the confidence
   *                             fields associated with this filter.  Note that
   *                             the calling method retains control of this
   *                             pointer and must delete the pointer when
   *                             processing is finished.
   * @param[in] ignore_low_dbz Flag indicating whether the filter should be
   *                           applied in areas of low reflectivity.
   * @param[in] low_dbz_thresh Threshold to use when ignoring areas of low
   *                           reflectivity.  If ignore_low_dbz is set, the
   *                           final interest value in areas where the input
   *                           reflectivity is below this value will be set
   *                           to missing.
   */

  void init(const double gate_spacing, const double final_threshold,
	    const int azimuth_radius, 
	    const TerrainType terrain_use,
	    DsRadarQueue *interest_queue, DsRadarQueue *confidence_queue,
	    const bool ignore_low_dbz,
	    const double low_dbz_thresh);

  /**
   * @brief Get the azimuth radius.
   *
   * @return The azimuth raduis.
   */

  int getAzRadius()
  {
    return _azRadius;
  }

  /**
   * @brief Set the combination type for this filter.
   *
   * @param[in] combine_type The new combination type value.
   */

  void setCombineType(const CombineType combine_type)
  {
    _combineType = combine_type;
  }

  /**
   * @brief Get the combination type for this filter.
   *
   * @return The combination type for this filter.
   */

  CombineType getCombineType()
  {
    return _combineType;
  }

  /**
   * @brief Set up for the current tilt.
   *
   * @param[in] num_gates Number of gates in each beam in the current tilt.
   */

  void setTilt(const int num_gates);

  /**
   * @brief Set up the filter field information.
   *
   * @param[in] index Index of the input field within the input dataset.
   * @param(in] missing_val Missing value used for the input field.
   */

  void setFieldInfo(const int index, const int missing_val);

  /**
   * @brief Clear the filter field information.
   */

  void clearFieldInfo();

  /**
   * @brief Set up for the current beam.
   *
   * @param[in] azimuth Azimuth value for the current beam.
   */

  void setBeam(const double azimuth);

  /**
   * @brief Get the number of beams in the interest data.
   *
   * @return The number of beams in the interest data.
   */

  int getNumBeams()
  {
    return _beamList.size();
  }

  /**
   * @brief Set the current global _terrainOk flag depending on whether the
   *        current terrain value indicates land or water and on where in the
   *        terrain we are calculating the interest.
   *
   * @param[in] terrain_val The current terrain mask value.
   */

  void setTerrain(const float terrain_val);
   
  /**
   * @brief Set the interest function and weight for the given interest type.
   *
   * @param[in] int_type Interest field for which to set the interest function.
   * @param[in] x1 X (data) value for the first point in the interest
   *               function.
   * @param[in] y1 Y (interest) value for the first point in the interest
   *               function.
   * @param[in] x2 X (data) value for the second point in the interest
   *               function.
   * @param[in] y2 Y (interest) value for the second point in the interest
   *               function.
   * @param[in] x3 X (data) value for the third point in the interest
   *               function.
   * @param[in] y3 Y (interest) value for the third point in the interest
   *               function.
   * @param[in] x4 X (data) value for the fourth point in the interest
   *               function.
   * @param[in] y4 Y (interest) value for the fourth point in the interest
   *               function.
   * @param[in] x5 X (data) value for the fifth point in the interest
   *               function.
   * @param[in] y5 Y (interest) value for the fifth point in the interest
   *               function.
   * @param[in] x6 X (data) value for the sixth point in the interest
   *               function.
   * @param[in] y6 Y (interest) value for the sixth point in the interest
   *               function.
   * @param[in] weight Weight to use for this interest function when
   *                   calculating the final interest for this filter.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int setInterestFunc(const FilterBeamInfo::InterestType int_type,
		      const double x1, const double y1,
		      const double x2, const double y2,
		      const double x3, const double y3,
		      const double x4, const double y4,
		      const double x5, const double y5,
		      const double x6, const double y6,
		      const double weight);

  /**
   * @brief Set the confidence function and weight for the given feature type.
   *
   * @param[in] int_type Feature field for which to set the confidence
   *                     function.
   * @param[in] x1 X (data) value for the first point in the confidence
   *               function.
   * @param[in] y1 Y (interest) value for the first point in the confidence
   *               function.
   * @param[in] x2 X (data) value for the second point in the confidence
   *               function.
   * @param[in] y2 Y (interest) value for the second point in the confidence
   *               function.
   * @param[in] x3 X (data) value for the third point in the confidence
   *               function.
   * @param[in] y3 Y (interest) value for the third point in the confidence
   *               function.
   * @param[in] x4 X (data) value for the fourth point in the confidence
   *               function.
   * @param[in] y4 Y (interest) value for the fourth point in the confidence
   *               function.
   * @param[in] x5 X (data) value for the fifth point in the confidence
   *               function.
   * @param[in] y5 Y (interest) value for the fifth point in the confidence
   *               function.
   * @param[in] x6 X (data) value for the sixth point in the confidence
   *               function.
   * @param[in] y6 Y (interest) value for the sixth point in the confidence
   *               function.
   * @param[in] weight Weight to use for this confidence function when
   *                   calculating the final interest for this filter.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int setConfidenceFunc(const FilterBeamInfo::InterestType int_type,
			const double x1, const double y1,
			const double x2, const double y2,
			const double x3, const double y3,
			const double x4, const double y4,
			const double x5, const double y5,
			const double x6, const double y6,
			const double weight);

  /**
   * @brief Calculate the interest value for the current gate.
   *
   * @param[in] int_type Interest field for which to calculate the interest.
   * @param[in] field_value Value of the feature field at the current gate.
   * @param[in] ibeam Beam index for the current gate in the current tilt.
   * @param[in] igate Gate index for the current gate in the current tilt.
   */

  void calcInterest(const FilterBeamInfo::InterestType int_type,
		    const double field_value,
		    const int ibeam, const int igate);

  /**
   * @brief Calculate the final interest value for the current gate.
   *
   * @param[in] ibeam Beam index for the current gate in the current tilt.
   * @param[in] igate Gate index for the current gate in the current tilt.
   * @param[in] dbz_val The input reflectivity value for the current gate.
   */

  void calcFinal(const int ibeam, const int igate, const float dbz_val);

  /**
   * @brief Get the filter flag for this gate.
   *
   * @param[in] beam_num Beam number of gate in the interest data.
   * @param[in] gate_num Gate number of gate.
   *
   * @return Returns true if the filter indicates that this gate should be
   *         filtered, false otherwise.
   */

  virtual bool getFilterFlag(const int beam_num, const int gate_num) = 0;

  /**
   * @brief Write the current tilt interest data to the output FMQ used for
   *        this filter.
   *
   * @param[in] start_time Start time for current tilt.
   * @param[in] end_time End time for current tilt.
   * @param[in] sample_params
   * @param[in] elev_angle Elevation angle for current tilt.
   * @param[in] volume_num Volume number for current tilt.
   * @param[in] tilt_num Tilt number for current tilt.
   */

  int writeInterest(const time_t start_time, const time_t end_time, 
		    const DsRadarParams& sample_params,
		    const double elev_angle,
		    const int volume_num, const int tilt_num);

  /**
   * @brief Write start of volume flag to the interest FMQ for this filter.
   *
   * @param[in] start_time Start time for current tilt.
   * @param[in] volume_num Volume number for current tilt.
   *
   * @note Need to write volume flags separate from the method that writes
   *       the data for the tilt, because we are not guaranteed that we will
   *       be filtering a tilt that has the start/end of volume flags attached.
   */

   void putStartOfVolume(const time_t start_time, const int volume_num);

  /**
   * @brief Write end of volume flag to the interest FMQ for this filter.
   *
   * @param[in] end_time End time for current tilt.
   * @param[in] volume_num Volume number for current tilt.
   *
   * @note Need to write volume flags separate from the method that writes
   *       the data for the tilt, because we are not guaranteed that we will
   *       be filtering a tilt that has the start/end of volume flags attached.
   */

  void putEndOfVolume(const time_t end_time, const int volume_num);


 protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief String indicating we are currently outputting interest data.
   *        This string is used to determine which data to retrieve from the
   *        FilterBeamInfo object and is included in the debugging output.
   */

  static const string INTEREST_OUTPUT;

  /**
   * @brief String indicating we are currently outputting confidence data.
   *        This string is used to determine which data to retrieve from the
   *        FilterBeamInfo object and is included in the debugging output.
   */

  static const string CONFIDENCE_OUTPUT;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Number of gates in the current tilt.
   */

  int _nGates;

  /**
   * @brief 
   */

  int _azRadius;

  /**
   * @brief Gate spacing for the input/output data in km.
   */

  double _gateSpacing;

  /**
   * @brief The threshold value for the final flitering of the input data.
   *        The input fields will be set to missing wherever the final
   *        interest value for this filter is greater than this value.
   */

  double _finalThreshold;

  /**
   * @brief The type of combination to use when combining the final interest
   *        value for this filter with the final interest for other filters.
   */

  CombineType _combineType;

  /**
   * @brief Flag indicating whether to ignore gates with low input reflectivity
   *        values when creating the final interest field.
   */

  bool   _ignoreLowDbz;

  /**
   * @brief The low reflectivity threshold value.  If _ignoreLowDbz is true,
   *        any gates that have an input reflectivity value less than this
   *        threshold will have their final interest value set to missing.
   */

  double _lowDbzThreshold;
  
  /**
   * @brief Flag indicating whether the terrain at the current gate matches
   *        the terrain used for this filter.
   */

  bool _terrainOk;

  /**
   * @brief Type of terrain where this filter is applied.  Can be LAND, WATER
   *        or ALL.
   */

  TerrainType _terrainUseType;

  /**
   * @brief Ordered list of beam information.  There is a FilterBeamInfo
   *        object for each beam in the tilt.
   */

  vector< FilterBeamInfo* > _beamList;
  
  /**
   * @brief This member maps each feature field with that field's interest
   *        function.  The index in this map is the feature field type and
   *        the value is the interest function object.
   */

  map< FilterBeamInfo::InterestType, InterestFunction* > _interestFunc;

  /**
   * @brief Flag indicating whether there are any confidence functions
   *        associated with this filter.
   */

  bool _applyConfidence;
  
  /**
   * @brief This member maps each feature field with that field's confidence
   *        function.  The index in this map is the feature field type and
   *        the value is the confidence function object.
   */

  map< FilterBeamInfo::InterestType, InterestFunction* > _confidenceFunc;

  /**
   * @brief Pointer to the interest output FMQ.  This pointer is actually
   *        owned by the calling method and must not be deleted within this
   *        class.
   */

  DsRadarQueue *_interestQueue;

  /**
   * @brief Pointer to the confidence output FMQ.  This pointer is actually
   *        owned by the calling method and must not be deleted within this
   *        class.
   */

  DsRadarQueue *_confidenceQueue;

  /**
   * @brief Object used for writing the interest fields for this filter to
   *        the output FMQ.  A global member is used so that we don't need to
   *        reest the field and radar params in the message every time we
   *        write something out.
   */

  DsRadarMsg _interestMsg;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Clear out all of the current interest information.
   */

  void _clear();

  /**
   * @brief Write the current tilt interest data to the output FMQ used for
   *        this filter.
   *
   * @param[in,out] queue FMQ to use for output.  If 0, no writing will
   *                      be done.
   * @param[in] output_type Type of output we are doing.  Can be either
   *                        INTEREST_OUTPUT or CONFIDENCE_OUTPUT.
   * @param[in] start_time Start time for current tilt.
   * @param[in] end_time End time for current tilt.
   * @param[in] sample_params
   * @param[in] elev_angle Elevation angle for current tilt.
   * @param[in] volume_num Volume number for current tilt.
   * @param[in] tilt_num Tilt number for current tilt.
   */

  int _writeInterest(DsRadarQueue *queue, const string &output_type,
		     const time_t start_time, const time_t end_time, 
		     const DsRadarParams& sample_params,
		     const double elev_angle,
		     const int volume_num, const int tilt_num);

  /**
   * @brief Copy constructor.
   *
   * @param[in] filter The Filter object to copy.
   *
   * @note The copy constructor is private and undefined in this class
   *       because it is complicated to write and the default copy constructor
   *       cannot be used.
   */

  Filter(const Filter &filter);
  
};

#endif
