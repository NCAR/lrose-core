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
 * @file DataMgr.hh
 *
 * @class DataMgr
 *
 * DataMgr manages all of the data and the processing for the ApRemoval
 * application.
 *  
 * @date 4/3/2002
 *
 */

#ifndef DataMgr_HH
#define DataMgr_HH

using namespace std;

#include <Fmq/DsRadarQueue.hh>
#include <toolsa/MsgLog.hh>

#include "Feature.hh"
#include "FilterBeamInfo.hh"
#include "InterestFunction.hh"
#include "Params.hh"
#include "RadarTilt.hh"
#include "TerrainMask.hh"

#include "GTFilter.hh"
#include "LTFilter.hh"


/** 
 * @class DataMgr
 */

class DataMgr
{
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  DataMgr();

  /** 
   * @brief Destructor
   */

  ~DataMgr();
   
  /**
   * @brief Initialize the object.
   *
   * @param[in] params Parameters read from the parameter file.
   * @param[in] msg_log Message log to use during processing.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int init(Params& params, MsgLog &msg_log);

  /**
   * @brief Process the data.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int processData();

private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief Factor used to convert meters to kilometers.
   */

  static const float M_TO_KM;
   

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Flag indicating whether we want to trigger filtering when we
   *        encounter an end-of-volume flag in the input.
   */

  bool _eovTrigger;
   
  /**
   * @brief Minimum elevation to be filtered.
   */

  double _minElev;

  /**
   * @brief Maximum elevation to be filtered.
   */

  double _maxElev;

  /**
   * @brief Radar scans top-down
   */

  bool _topDownScanning;
   
  /**
   * @brief Pointer to the information for the tilt that is currently being
   *        read in.
   */

  RadarTilt *_currTilt;

  /**
   * @brief Pointer to the information for the current tilt.
   */

  RadarTilt *_prevTilt;

  /**
   * @brief Pointer to the FMQ containing the input data stream.
   */

  DsRadarQueue *_inputQueue;

  /**
   * @brief Pointer to the FMQ used for writing the filtered data.
   */

  DsRadarQueue *_outputQueue;

  /**
   * @brief Pointer to the FMQ used for writing the AP interest fields.
   *        Will be 0 if we are not outputting these fields.
   */

  DsRadarQueue *_apInterestQueue;

  /**
   * @brief Pointer to the FMQ used for writing the sea clutter interest
   *        fields.  Will be 0 if we are not outputting these fields.
   */

  DsRadarQueue *_scInterestQueue;

  /**
   * @brief Pointer to the FMQ used for writing the precipitation interest
   *        fields.  Will be 0 if we are not outputting these fields.
   */

  DsRadarQueue *_pInterestQueue;

  /**
   * @brief Pointer to the FMQ used for writing the AP confidence fields.
   *        Will be 0 if we are not outputting these fields.
   */

  DsRadarQueue *_apConfidenceQueue;

  /**
   * @brief Pointer to the FMQ used for writing the sea clutter confidence
   *        fields.  Will be 0 if we are not outputting these fields.
   */

  DsRadarQueue *_scConfidenceQueue;

  /**
   * @brief Pointer to the FMQ used for writing the precipitation confidence
   *        fields.  Will be 0 if we are not outputting these fields.
   */

  DsRadarQueue *_pConfidenceQueue;

  /**
   * @brief Pointer to the FMQ used for writing the feature fields.  Will
   *        be 0 if we are not outputting these fields.
   */

  DsRadarQueue *_featureQueue;

  /**
   * @brief Terrain mask object used for masking the interest fields based
   *        on terrain.
   */

  TerrainMask _terrainMask;

  /**
   * @brief The AP filter.
   */

  GTFilter _apFilter;

  /**
   * @brief The sea clutter filter.
   */

  GTFilter _scFilter;

  /**
   * @brief The precipitation filter.
   */

  LTFilter _pFilter;
  
  /**
   * @brief The feature field manager.
   */

  Feature _featureFields;

  /**
   * @brief Function used for weighting feature fields by range.
   */

  InterestFunction _rangeWtFunc;

  /**
   * @brief List of input fields to be filtered.
   *
   * @todo This really should just be a vector of strings rather than a vector
   *       of pointers to strings.
   */

  vector< string* > _filterFieldNames;


  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Set the defined interest function for the given filter.
   *
   * @param[in] int_func_params The parameter file information that defines
   *                            the interest function.
   * @param[in] spin_interest_type The type of spin feature field to use for
   *                                this filter.
   * @param[in,out] filter The filter to add this interest function to.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int _setInterestFunction(const Params::interest_func_t int_func_params,
			   const FilterBeamInfo::InterestType spin_interest_type,
			   Filter &filter);
  
  /**
   * @brief Set the defined confidence function for the given filter.
   *
   * @param[in] conf_func_params The parameter file information that defines
   *                             the confidence function.
   * @param[in] spin_interest_type The type of spin feature field to use for
   *                                this filter.
   * @param[in,out] filter The filter to add this confidence function to.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int _setConfidenceFunction(const Params::interest_func_t conf_func_params,
			     const FilterBeamInfo::InterestType spin_interest_type,
			     Filter &filter);
  
  /**
   * @brief Process the currently complete tilts.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int _processTilts();
  int _processTopDownTilts();

  /**
   * @brief Process the end-of-volume flag in the input message.
   *
   * @param[in] input_msg The input message.
   * @param[in] contents Mask indicating the type of information in the input
   *                     message.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int _processEOV();

};

#endif
