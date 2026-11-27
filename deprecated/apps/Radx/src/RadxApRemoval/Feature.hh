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
 * @file Feature.hh
 *
 * @class Feature
 *
 * Feature stores the values of the feature fields.
 *  
 * @date 9/18/2002
 *
 */

#ifndef Feature_HH
#define Feature_HH

using namespace std;

#include <map>
#include <vector>

#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsRadarParams.hh>

#include "FilterBeamInfo.hh"

/** 
 * @class Feature
 */

class Feature 
{
public:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /** 
   * @brief A radar params message should be included in the output FMQ
   *        once every RADAR_PARAMS_COUNT output beams.
   */

  static const int RADAR_PARAMS_COUNT;

  /** 
   * @brief A summary debug message is printed once every RADAR_SUMMARY_COUNT
   *        output beams.
   */

  static const int RADAR_SUMMARY_COUNT;

  /** 
   * @brief The number of feature fields.
   */

  static const int N_FEATURE_FIELDS;

  /** 
   * @brief The scale used for scaling the TDBZ feature field.
   */

  static const float TDBZ_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the TDBZ feature field.
   */

  static const float TDBZ_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the GDZ feature field.
   */

  static const float GDZ_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the GDZ feature field.
   */

  static const float GDZ_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the MVE feature field.
   */

  static const float MVE_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the MVE feature field.
   */

  static const float MVE_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the MSW feature field.
   */

  static const float MSW_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the MSW feature field.
   */

  static const float MSW_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the SDVE feature field.
   */

  static const float SDVE_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the SDVE feature field.
   */

  static const float SDVE_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the SDSW feature field.
   */

  static const float SDSW_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the SDSW feature field.
   */

  static const float SDSW_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the SPIN feature fields.
   */

  static const float SPIN_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the SPIN feature fields.
   */

  static const float SPIN_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the SIGN feature field.
   */

  static const float SIGN_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the SIGN feature field.
   */

  static const float SIGN_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the RGDZ feature field.
   */

  static const float RGDZ_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the RGDZ feature field.
   */

  static const float RGDZ_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the SRDZ feature field.
   */

  static const float SRDZ_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the SRDZ feature field.
   */

  static const float SRDZ_FIELD_BIAS;

  /** 
   * @brief The scale used for scaling the RSINZ feature field.
   */

  static const float RSINZ_FIELD_SCALE;

  /** 
   * @brief The bias used for scaling the RSINZ feature field.
   */

  static const float RSINZ_FIELD_BIAS;

  /** 
   * @brief The value used for flagging missing data in the feature fields.
   */

  static const float MISSING_VALUE;

  /** 
   * @brief The value used for flagging missing data in the scaled
   *        feature fields.
   */

  static const int SCALED_MISSING_VALUE;

  /** 
   * @brief The minimum value that can be used to represent good data in the
   *        scaled data.  This value depends on the number of bytes used in
   *        the feature FMQ and which scaled value is used to represent
   *        missing data.
   */

  static const int SCALED_MIN_VALUE;

  /** 
   * @brief The maximum value that can be used to represent good data in the
   *        scaled data.  This value depends on the number of bytes used in
   *        the feature FMQ and which scaled value is used to represent
   *        missing data.
   */

  static const int SCALED_MAX_VALUE;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  Feature();

  /**
   * @brief Destructor
   */

  ~Feature();

  /**
   * @brief Initialize the parameters that remain constant throughout the
   *        whole time the object is in memory
   *
   * @param[in] gate_spacing Gate spacing used for the input and output data.
   * @param[in] output_queue Output FMQ to use for storing the feature values,
   *                         if requested.  Note that the calling method
   *                         retains control of this pointer and must delete
   *                         the pointer when processing is finished.
   */

  void init(const double gate_spacing, DsRadarQueue *output_queue = 0);

  /**
   * @brief Set up for the current tilt.
   *
   * @param[in] num_gates Number of gates in each beam in the new tilt.
   */

  void setTilt(const int num_gates);

  /**
   * @brief Set up for the current beam.
   *
   * @param[in] azimuth The azimuth for the new beam.
   */

  void setBeam(const double azimuth);

  /**
   * @brief Set the feature data at the given point.
   *
   * @param[in] int_type The feature type to be set.
   * @param[in] field_val The value of the feature field.
   * @param[in] ibeam The beam index for this gate.
   * @param[in] igate The gate index for this gate.
   */

  void setData(const FilterBeamInfo::InterestType int_type,
	       const double field_val, 
	       const int ibeam, const int igate);

  /**
   * @brief Write the feature data for the current tilt to the output FMQ.
   *
   * @param[in] start_time The start time for the current tilt.
   * @param[in] end_time The end time for the current tilt.
   * @param[in] smaple_params
   * @param[in] elev_angle The elevation angle for the current tilt.
   * @param[in] vol_num The volume number for the current tilt.
   * @param[in] tilt_num The tilt number for the current tilt.
   */

  int write(const time_t start_time, const time_t end_time,
	    const DsRadarParams& sample_params,
	    const double elev_angle,
	    const int vol_num, const int tilt_num);

  /**
   * @brief Write a start of volume flag to the output FMQ.  We need to keep
   *        the volume flags separate from the method that writes the data
   *        for the tilt because we are not guaranteed that we will be
   *        calculating feature data for the tilt that has the start/end of
   *        volume flags attached.
   *
   * @param[in] start_time The start time for the current volume.
   * @param[in] vol_num The volume number for the current volume.
   */

  void putStartOfVolume(const time_t start_time, const int vol_num);

  /**
   * @brief Write an end of volume flag to the output FMQ.  We need to keep
   *        the volume flags separate from the method that writes the data
   *        for the tilt because we are not guaranteed that we will be
   *        calculating feature data for the tilt that has the start/end of
   *        volume flags attached.
   *
   * @param[in] end_time The end time for the current volume.
   * @param[in] vol_num The volume number for the current volume.
   */

  void putEndOfVolume(const time_t start_time, const int vol_num);

private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief The number of gates in the input and output data.
   */

  int _nGates;

  /**
   * @brief The gate spacing in the input and output data in km.
   */

  double _gateSpacing;

  /**
   * @brief Pointers to the feature field data for the current tilt.  There
   *        is one pointer for each beam in the tilt.  Within the beam data,
   *        there is a value for every feature field for every gate.  The
   *        beams are in the same order as the _azimuths vector.
   */

  vector< ui16* > _featurePtrs;

  /**
   * @brief The azimuths in the current tilt.
   */

  vector< double > _azimuths;

  /**
   * @brief Pointer to the output FMQ for the feature fields.  The queue
   *        itself is owned by the calling method and should not be deleted
   *        within this object.
   */

  DsRadarQueue *_featureQueue;

  /**
   * @brief Object used for writing the feature fields to the output FMQ.
   *        A global member is used so that we don't need to reest the field
   *        and radar params in the message every time we write something out.
   */

  DsRadarMsg _featureMsg;

  /**
   * @brief Map associating each feature field type with its field params.
   *        We need to save this mapping so that we can easily retrieve the
   *        scale and bias information for each feature field.
   */

  map< FilterBeamInfo::InterestType, DsFieldParams* > _featureFieldMap;


  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Clear out all of the information to get ready for processing a
   *        new tilt.
   */

  void _clear();

};

#endif
