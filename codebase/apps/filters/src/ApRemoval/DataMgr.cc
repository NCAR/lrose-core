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
 * @file DataMgr.cc
 *
 * @class DataMgr
 *
 * DataMgr manages all of the data and the processing for the ApRemoval
 * application.
 *  
 * @date 4/10/2002
 *
 */

using namespace std;

#include <string>
#include <unistd.h>

#include "ApRemoval.hh"
#include "DataMgr.hh"
#include "FilterList.hh"

//
// Constants
//
const float DataMgr::M_TO_KM = 0.001;


/**
 * Constructor
 */

DataMgr::DataMgr() :
  _eovTrigger(true),
  _minElev(0.0),
  _maxElev(0.0),
  _topDownScanning(false),
  _currTilt(0),
  _prevTilt(0),
  _inputQueue(0),
  _outputQueue(0),
  _apInterestQueue(0),
  _scInterestQueue(0),
  _pInterestQueue(0),
  _apConfidenceQueue(0),
  _scConfidenceQueue(0),
  _pConfidenceQueue(0),
  _featureQueue(0)
{
}


/**
 * Destructor
 */

DataMgr::~DataMgr() 
{
  delete _currTilt;
  delete _prevTilt;
  delete _inputQueue;
  delete _outputQueue;
  delete _apInterestQueue;
  delete _scInterestQueue;
  delete _pInterestQueue;
  delete _apConfidenceQueue;
  delete _scConfidenceQueue;
  delete _pConfidenceQueue;
  delete _featureQueue;

  vector< string* >::iterator it;
  for (it = _filterFieldNames.begin(); it != _filterFieldNames.end(); ++it)
    delete *it;

  _filterFieldNames.erase(_filterFieldNames.begin(), _filterFieldNames.end());
}


/**
 * init()
 */

int DataMgr::init(Params &params, MsgLog &msg_log) 
{
  // Do we want to trigger by end of volume?

  _eovTrigger = (params.eov_trigger ? true : false);
   
  // Gate spacing

  float gate_spacing_km = params.gate_spacing * M_TO_KM;
   
  // Elevation parameters

  _minElev = params.filter_range.min_elev;
  _maxElev = params.filter_range.max_elev;
  _topDownScanning = params.radar_scans_top_down;
   
  // Set up the input queue

  _inputQueue = new DsRadarQueue;
  if (_inputQueue->initReadBlocking(params.input_fmq_url,
				    PROGRAM_NAME,
				    DEBUG_ENABLED,
				    DsFmq::END,
				    -1, &msg_log) != 0)
  {
    POSTMSG(ERROR, "Could not initialize input fmq %s",
	    params.input_fmq_url);
    return -1;
  }
   
  // Set up the output queue

  _outputQueue = new DsRadarQueue;
  if (_outputQueue->initCreate(params.output_fmq_url,
			       PROGRAM_NAME,
			       DEBUG_ENABLED,
			       false,
			       params.output_fmq_nslots,
			       params.output_fmq_size,
			       &msg_log) != 0)
  {
    POSTMSG(ERROR, "Could not initialize output fmq %s",
	    params.output_fmq_url);
    return -1;
  }

  // Set up the ap output queues if necessary

  if (params.write_ap_interest)
  {
    _apInterestQueue = new DsRadarQueue;
    if (_apInterestQueue->initCreate(params.ap_interest_fmq_url,
				     PROGRAM_NAME,
				     DEBUG_ENABLED,
				     false,
				     params.ap_interest_fmq_nslots,
				     params.ap_interest_fmq_size,
				     &msg_log) != 0)
    {
      POSTMSG(ERROR, "Could not initialize interest fmq %s",
	      params.ap_interest_fmq_url);
      return -1;
    }
  }

  if (params.write_ap_confidence)
  {
    _apConfidenceQueue = new DsRadarQueue;
    if (_apConfidenceQueue->initCreate(params.ap_confidence_fmq_url,
				       PROGRAM_NAME,
				       DEBUG_ENABLED,
				       false,
				       params.ap_confidence_fmq_nslots,
				       params.ap_confidence_fmq_size,
				       &msg_log) != 0)
    {
      POSTMSG(ERROR, "Could not initialize confidence fmq %s",
	      params.ap_confidence_fmq_url);
      return -1;
    }
  }

  // Set up the sea clutter output queues if necessary

  if (params.write_sc_interest)
  {
    _scInterestQueue = new DsRadarQueue;
    if (_scInterestQueue->initCreate(params.sc_interest_fmq_url,
				     PROGRAM_NAME,
				     DEBUG_ENABLED,
				     false,
				     params.sc_interest_fmq_nslots,
				     params.sc_interest_fmq_size,
				     &msg_log) != 0)
    {
      POSTMSG(ERROR, "Could not initialize interest fmq %s",
	      params.sc_interest_fmq_url);
      return -1;
    }
  }

  if (params.write_sc_confidence)
  {
    _scConfidenceQueue = new DsRadarQueue;
    if (_scConfidenceQueue->initCreate(params.sc_confidence_fmq_url,
				       PROGRAM_NAME,
				       DEBUG_ENABLED,
				       false,
				       params.sc_confidence_fmq_nslots,
				       params.sc_confidence_fmq_size,
				       &msg_log) != 0)
    {
      POSTMSG(ERROR, "Could not initialize confidence fmq %s",
	      params.sc_confidence_fmq_url);
      return -1;
    }
  }

  // Set up the precipitation output queues if necessary

  if (params.write_p_interest)
  {
    _pInterestQueue = new DsRadarQueue;
    if (_pInterestQueue->initCreate(params.p_interest_fmq_url,
				    PROGRAM_NAME,
				    DEBUG_ENABLED,
				    false,
				    params.p_interest_fmq_nslots,
				    params.p_interest_fmq_size,
				    &msg_log) != 0)
    {
      POSTMSG(ERROR, "Could not initialize interest fmq %s",
	      params.p_interest_fmq_url);
      return -1;
    }
  }

  if (params.write_p_confidence)
  {
    _pConfidenceQueue = new DsRadarQueue;
    if (_pConfidenceQueue->initCreate(params.p_confidence_fmq_url,
				      PROGRAM_NAME,
				      DEBUG_ENABLED,
				      false,
				      params.p_confidence_fmq_nslots,
				      params.p_confidence_fmq_size,
				      &msg_log) != 0)
    {
      POSTMSG(ERROR, "Could not initialize confidence fmq %s",
	      params.p_confidence_fmq_url);
      return -1;
    }
  }

  // Set up the feature queue if necessary

  if (params.write_feature_data)
  {
    _featureQueue = new DsRadarQueue;
    if (_featureQueue->initCreate(params.feature_fmq_url,
				  PROGRAM_NAME,
				  DEBUG_ENABLED,
				  false,
				  params.feature_fmq_nslots,
				  params.feature_fmq_size,
				  &msg_log) != 0)
    {
      POSTMSG(ERROR, "Could not initialize feature fmq %s",
	      params.feature_fmq_url);
      return -1;
    }
  }

  // Initialize terrain stuff

  Filter::TerrainType ap_terr_use_type = Filter::ALL;
  Filter::TerrainType sc_terr_use_type = Filter::ALL;
  Filter::TerrainType p_terr_use_type = Filter::ALL;
   
  if (params.use_terrain)
  {
    // Set up terrain mask

    int status = 
      _terrainMask.init(params.terrain_url, 
			params.terrain_field_name,
			params.max_num_gates, 
			gate_spacing_km, 
			params.delta_azimuth, 
			params.radar_location.latitude,
			params.radar_location.longitude);

    if (status != 0)
    {
      POSTMSG(ERROR, "Could not initialize terrain mask");
      return -1;
    }

    // Write out the mask if necessary

    if (params.write_terrain)
      _terrainMask.writeMask(params.terrain_output_path);

    // Set the ap terrain use type

    switch (params.ap_terrain_use_type)
    {
    case Params::LAND:
      ap_terr_use_type = Filter::LAND;
      break;
          
    case Params::WATER:
      ap_terr_use_type = Filter::WATER;
      break;
          
    case Params::ALL:
      ap_terr_use_type = Filter::ALL;
      break;
    }

    // Set the sea clutter terrain use type

    switch (params.sc_terrain_use_type)
    {
    case Params::LAND:
      sc_terr_use_type = Filter::LAND;
      break;
          
    case Params::WATER:
      sc_terr_use_type = Filter::WATER;
      break;
          
    case Params::ALL:
      sc_terr_use_type = Filter::ALL;
      break;
    }
      
    // Set the precipitation terrain use type

    switch (params.p_terrain_use_type)
    {
    case Params::LAND:
      p_terr_use_type = Filter::LAND;
      break;
          
    case Params::WATER:
      p_terr_use_type = Filter::WATER;
      break;
          
    case Params::ALL:
      p_terr_use_type = Filter::ALL;
      break;
    }

  }

  // Set up filtering objects

  int max_az_radius = MAX(params.dbz_radius_info.azimuth_radius,
                          params.vel_radius_info.azimuth_radius);

  _apFilter.init(gate_spacing_km, params.ap_interest_threshold, max_az_radius,
		 ap_terr_use_type, _apInterestQueue, _apConfidenceQueue,
		 false, 0.0);

  _scFilter.init(gate_spacing_km, params.sc_interest_threshold, max_az_radius,
		 sc_terr_use_type, _scInterestQueue, _scConfidenceQueue,
		 false, 0.0);

  _pFilter.init(gate_spacing_km, params.p_interest_threshold, max_az_radius,
		p_terr_use_type, _pInterestQueue, _pConfidenceQueue,
		params.p_filter_low_dbz, params.p_low_dbz_threshold);

  // Set up ap interest functions

  for (int i = 0; i < params.ap_interest_func_n; ++i)
  {
    int status = _setInterestFunction(params._ap_interest_func[i],
				      FilterBeamInfo::AP_SPIN, _apFilter);
     
    if (status != 0)
      return status;
  }

  // Set up ap confidence functions

  for (int i = 0; i < params.ap_confidence_func_n; ++i)
  {
    int status = _setConfidenceFunction(params._ap_confidence_func[i],
					FilterBeamInfo::AP_SPIN, _apFilter);
     
    if (status != 0)
      return status;
  }

  // Set up sea clutter interest functions

  for (int i = 0; i < params.sc_interest_func_n; ++i)
  {
    int status = _setInterestFunction(params._sc_interest_func[i],
				      FilterBeamInfo::SC_SPIN, _scFilter);
     
    if (status != 0)
      return status;
  }

  // Set up sea clutter confidence functions

  for (int i = 0; i < params.sc_confidence_func_n; ++i)
  {
    int status = _setConfidenceFunction(params._sc_confidence_func[i],
					FilterBeamInfo::SC_SPIN, _scFilter);
     
    if (status != 0)
      return status;
  }

  // Set up precipitation interest functions

  for (int i = 0; i < params.p_interest_func_n; ++i)
  {
    int status = _setInterestFunction(params._p_interest_func[i],
				      FilterBeamInfo::P_SPIN, _pFilter);
     
    if (status != 0)
      return status;
  }
  
  // Set up precipitation confidence functions

  for (int i = 0; i < params.p_confidence_func_n; ++i)
  {
    int status = _setConfidenceFunction(params._p_confidence_func[i],
					FilterBeamInfo::P_SPIN, _pFilter);
     
    if (status != 0)
      return status;
  }

  // Build the list of filters based on how the filters will be combined.
  // First, set the combine type for the first filter so that missing data
  // values will be handled properly.  If there are multiple filters, we
  // need to make sure that the combine type for the first filter matches
  // the type for the second filter.  If there is only one filter, it needs
  // to be set to an OR filter so that missing data is interpreted as false.

  if (params.filter_combination_n == 1)
    params._filter_combination[0].comb_type = Params::OR_FILTER;
  else
    params._filter_combination[0].comb_type =
      params._filter_combination[1].comb_type;
  
  FilterList filter_list;
   
  for (int i = 0; i < params.filter_combination_n; ++i)
  {
    Filter::CombineType combine_type = Filter::COMBINE_AND;
     
    switch (params._filter_combination[i].comb_type)
    {
    case Params::AND_FILTER :
      combine_type = Filter::COMBINE_AND;
      break;
       
    case Params::OR_FILTER :
      combine_type = Filter::COMBINE_OR;
      break;
    }
     
    Filter *filter = 0;
     
    switch (params._filter_combination[i].filter_type)
    {
    case Params::APDA :
      filter = &_apFilter;
      break;
       
    case Params::SCDA :
      filter = &_scFilter;
      break;
       
    case Params::PDA :
      filter = &_pFilter;
      break;
    }
     
    filter->setCombineType(combine_type);
    filter_list.addFilter(filter);
  } /* endfor - i */
   
  // Set up the feature object

  _featureFields.init(gate_spacing_km, _featureQueue);

  // Set up range weight function.  Give it an arbitrary weight value of 1.0.
  // This is okay since this weight value will never actually be used.

  int status = _rangeWtFunc.setFunction(params.range_weight_func.x1,
					params.range_weight_func.y1,
					params.range_weight_func.x2,
					params.range_weight_func.y2,
					params.range_weight_func.x3,
					params.range_weight_func.y3,
					params.range_weight_func.x4,
					params.range_weight_func.y4,
					params.range_weight_func.x5,
					params.range_weight_func.y5,
					params.range_weight_func.x6,
					params.range_weight_func.y6,
					1.0);

  if (status != 0)
    return status;

  // Set up vector of field names that we will be filtering

  for (int i = 0; i < params.filter_field_names_n; ++i)
  {
    string *new_name = new string(params._filter_field_names[i]);
    _filterFieldNames.push_back(new_name);
  }

  // Set up radar tilt

  _currTilt = new RadarTilt(*_outputQueue, 
			    params.dbz_radius_info.azimuth_radius,
			    params.vel_radius_info.azimuth_radius,
			    params.dbz_radius_info.gate_radius,
			    params.vel_radius_info.gate_radius,
			    gate_spacing_km,
			    params.max_num_gates,
			    params.refl_field_name,
			    params.vel_field_name,
			    params.sw_field_name,
			    params.sc_spin_threshold,
			    params.ap_spin_threshold,
			    params.p_spin_threshold,
			    params.delta_azimuth,
			    params.slant_range_dist,
			    _terrainMask,
			    filter_list,
			    _featureFields,
			    _rangeWtFunc,
			    _filterFieldNames);

  _prevTilt = new RadarTilt(*_outputQueue, 
                            params.dbz_radius_info.azimuth_radius,
                            params.vel_radius_info.azimuth_radius,
                            params.dbz_radius_info.gate_radius,
                            params.vel_radius_info.gate_radius,
                            gate_spacing_km,
                            params.max_num_gates,
                            params.refl_field_name,
                            params.vel_field_name,
                            params.sw_field_name,
                            params.sc_spin_threshold,
                            params.ap_spin_threshold,
                            params.p_spin_threshold,
                            params.delta_azimuth,
                            params.slant_range_dist,
                            _terrainMask,
                            filter_list,
                            _featureFields,
                            _rangeWtFunc,
                            _filterFieldNames);

  return status;
}


/**
 * _setInterestFunction()
 */

int DataMgr::_setInterestFunction(const Params::interest_func_t int_func_params,
				  const FilterBeamInfo::InterestType spin_interest_type,
				  Filter &filter)
{
  FilterBeamInfo::InterestType interestType;
           
  switch (int_func_params.interest_field)
  {
  case Params::TDBZ :
    interestType = FilterBeamInfo::TDBZ;
    break;

  case Params::GDZ :
    interestType = FilterBeamInfo::GDZ;
    break;
               
  case Params::MVE :
    interestType = FilterBeamInfo::MVE;
    break;
               
  case Params::MSW :
    interestType = FilterBeamInfo::MSW;
    break;
               
  case Params::SDVE :
    interestType = FilterBeamInfo::SDVE;
    break;
               
  case Params::SDSW :
    interestType = FilterBeamInfo::SDSW;
    break;
               
  case Params::SPIN :
    interestType = spin_interest_type;
    break;
               
  case Params::SIGN :
    interestType = FilterBeamInfo::SIGN;
    break;
               
  case Params::RGDZ :
    interestType = FilterBeamInfo::RGDZ;
    break;
               
  case Params::SRDZ :
    interestType = FilterBeamInfo::SRDZ;
    break;
               
  case Params::RSINZ :
    interestType = FilterBeamInfo::RSINZ;
    break;
  }

  return filter.setInterestFunc(interestType,
				int_func_params.x1,
				int_func_params.y1,
				int_func_params.x2,
				int_func_params.y2,
				int_func_params.x3,
				int_func_params.y3,
				int_func_params.x4,
				int_func_params.y4,
				int_func_params.x5,
				int_func_params.y5,
				int_func_params.x6,
				int_func_params.y6,
				int_func_params.weight);
}


/**
 * _setConfidenceFunction()
 */

int DataMgr::_setConfidenceFunction(const Params::interest_func_t conf_func_params,
				    const FilterBeamInfo::InterestType spin_interest_type,
				    Filter &filter)
{
  FilterBeamInfo::InterestType interestType;
           
  switch (conf_func_params.interest_field)
  {
  case Params::TDBZ :
    interestType = FilterBeamInfo::TDBZ;
    break;

  case Params::GDZ :
    interestType = FilterBeamInfo::GDZ;
    break;
               
  case Params::MVE :
    interestType = FilterBeamInfo::MVE;
    break;
               
  case Params::MSW :
    interestType = FilterBeamInfo::MSW;
    break;
               
  case Params::SDVE :
    interestType = FilterBeamInfo::SDVE;
    break;
               
  case Params::SDSW :
    interestType = FilterBeamInfo::SDSW;
    break;
               
  case Params::SPIN :
    interestType = spin_interest_type;
    break;
               
  case Params::SIGN :
    interestType = FilterBeamInfo::SIGN;
    break;
               
  case Params::RGDZ :
    interestType = FilterBeamInfo::RGDZ;
    break;
               
  case Params::SRDZ :
    interestType = FilterBeamInfo::SRDZ;
    break;
               
  case Params::RSINZ :
    interestType = FilterBeamInfo::RSINZ;
    break;
  }

  return filter.setConfidenceFunc(interestType,
				  conf_func_params.x1,
				  conf_func_params.y1,
				  conf_func_params.x2,
				  conf_func_params.y2,
				  conf_func_params.x3,
				  conf_func_params.y3,
				  conf_func_params.x4,
				  conf_func_params.y4,
				  conf_func_params.x5,
				  conf_func_params.y5,
				  conf_func_params.x6,
				  conf_func_params.y6,
				  conf_func_params.weight);
}


/**
 * processData()
 */

int DataMgr::processData() 
{
  int contents = 0;
  DsRadarMsg *input_msg = new DsRadarMsg;   
  DsRadarFlags &radar_flags    = input_msg->getRadarFlags();

  int tilt_num = -1;
  int prev_tilt_num = -1;

  while (true)
  {
    // Get a message from the queue

    if (_inputQueue->getDsMsg(*input_msg, &contents))
    {
      POSTMSG( ERROR, "Could not get beam from input fmq" );
      sleep(1);
      continue;
    }
    
    // Find the tilt number
    
    if (contents & DsRadarMsg::RADAR_BEAM)
    {
      tilt_num = input_msg->getRadarBeam().tiltNum;
    }
    else if (contents & DsRadarMsg::RADAR_FLAGS)
    {
      if (radar_flags.endOfVolume)
      {
	tilt_num = prev_tilt_num;
      }
      else if (radar_flags.startOfVolume)
      {
	tilt_num = 0;
      }
      else
      {
	tilt_num = radar_flags.tiltNum;
      }
    }
    else
    {
      // This is some other kind of message.  Add it to the tilt so it can
      // be passed along later and then process the next message.

      _currTilt->addMsg(*input_msg, contents);
      
      continue;
    }

    PMU_auto_register("Is this a new tilt?");

    // Is this the end of a volume? Applies only to bottom-up scanning.

    if (!_topDownScanning)
    {

      if (contents & DsRadarMsg::RADAR_FLAGS)
      {

        if (radar_flags.endOfVolume && _eovTrigger)
        {
          
          // End of volume
          
          // add message to tilt
          if (_currTilt->addMsg(*input_msg, contents) != 0)
          {
            POSTMSG(WARNING, "Could not add end of volume flag");
          }
          
          // process 2 remaining tilts
          if (_processEOV() != 0)
          {
            POSTMSG(ERROR, "Could not process tilts at end of volume");
            return -1;
          }
          
          // Go on to the next message now
          
          continue;

        } // if (radar_flags.endOfVolume && _eovTrigger)
      
      } // if (contents & DsRadarMsg::RADAR_FLAGS)

    } // if (!_topDownScanning)


    // Is this a new tilt?

    if (tilt_num != prev_tilt_num)
    {

      // New tilt
      // Filter the correct tilt
      
      if (_topDownScanning)
      {
        if (_processTopDownTilts() != 0)
        {
          POSTMSG(ERROR, "Could not process tilts");
          return -1;
        }
      }
      else
      {
        if (_processTilts() != 0)
        {
          POSTMSG(ERROR, "Could not process tilts");
          return -1;
        }
      }

      // Switch the current and previous tilts for next pass

      RadarTilt *temp = _prevTilt;
      _prevTilt = _currTilt;
      _currTilt   = temp;

      // Set filterTiltNum for next pass

      prev_tilt_num = tilt_num;

      // Clear out the tilt object so that it is ready to be
      // used for next one

      _currTilt->clear();

      PMU_auto_register("Next tilt");
    }
            
    // Add the message to the tilt

    if (_currTilt->addMsg(*input_msg, contents) != 0)
      continue;
    
  } /* endwhile - true */
  
  return 0;
}


/**
 * _processTilts()
 */

int DataMgr::_processTilts() 
{

  // See about filtering and writing out data if neither
  // tilt is empty -
  //   This check is essential to guarantee tilt buffering

  if (!_prevTilt->tiltEmpty() && !_currTilt->tiltEmpty())
  {
    // Get the elevation of the two tilts

    double prev_elev = _prevTilt->getElevAngle();
    double curr_elev = _currTilt->getElevAngle();

    // Is the tilt that we want to filter in filtering range?
    // If so filter it.

    if (prev_elev <= _maxElev && prev_elev >= _minElev)
    {
      // If the elevations are appearing in the order
      // we expect, filter using the next elevation

      if (prev_elev < curr_elev)
      {
	if (_prevTilt->filter(_currTilt) != 0)
	{
	  POSTMSG(WARNING, "Filtering failed at %f degrees", prev_elev);
	  PMU_auto_register("Filtering");
	}
      }
      else
      {
	// If the prev elevation is greater than or equal
	// to the next elevation, we could have an end of 
	// volume situation, repeated elevations, elevations 
	// coming through as the radar comes down for the next 
	// volume or some error in the radar data.  In any case, 
	// we should try to filter the data we have and go on.  
	// But we cannot use the next tilt, so we will not be 
	// able to compute GDZ.  

	// Don't print out any warning if this is an
	// end of volume situation

	if (_prevTilt->getVolNum() == _currTilt->getVolNum())
	{
	  POSTMSG(WARNING, "Repeated elevations or elevations "
		  "appearing out of order:  ");
	  POSTMSG(WARNING, "prev elevation = %f, next elevation = %f",
		  prev_elev, curr_elev);
	  POSTMSG(WARNING, "Could not compute GDZ field");
	}
            
	if (_prevTilt->filter() != 0)
	{
	  POSTMSG(WARNING, "Filtering failed at %f degrees", prev_elev);
	  PMU_auto_register("Filtering");
	}
      }
    }

    // Write out the tilt

    if (_prevTilt->write() != 0)
      return -1;

    PMU_auto_register("Writing");
  }

  return 0;
}

/**
 * _processTopDownTilts() for top-down scanning
 */

int DataMgr::_processTopDownTilts() 
{

  // check we have data in current tilt
  
  if (_currTilt->tiltEmpty()) {
    return 0;
  }

  // is this the first tilt in volume (i.e. the top one)

  if (_prevTilt->tiltEmpty()) {

    // first tilt - do not filter, just write out

    PMU_auto_register("Writing");
    if (_currTilt->write() != 0) {
      return -1;
    }

    return 0;

  }

  // Both current and previous tilts have data

  // Get the elevation of the two tilts
  
  double prev_elev = _prevTilt->getElevAngle();
  double curr_elev = _currTilt->getElevAngle();
  
  // Is the tilt that we want to filter in filtering range?
  // If so filter it.
  
  if (curr_elev <= _maxElev && curr_elev >= _minElev)
  {
    // If the elevations are appearing in the order
    // we expect, filter using the next elevation
    
    if (curr_elev < prev_elev)
    {
      PMU_auto_register("Filtering");
      if (_currTilt->filter(_prevTilt) != 0)
      {
        POSTMSG(WARNING, "Filtering failed at %f degrees", curr_elev);
      }
    }
    else
    {
      // If the next elevation is greater than or equal
      // to the prev elevation, we could have an end of 
      // volume situation, repeated elevations, elevations 
      // coming through as the radar comes down for the next 
      // volume or some error in the radar data.  In any case, 
      // we should try to filter the data we have and go on.  
      // But we cannot use the next tilt, so we will not be 
      // able to compute GDZ.  
      
      // Don't print out any warning if this is an
      // end of volume situation
      
      if (_prevTilt->getVolNum() == _currTilt->getVolNum())
      {
        POSTMSG(WARNING, "Repeated elevations or elevations "
                "appearing out of order:  ");
        POSTMSG(WARNING, "prev elevation = %f, next elevation = %f",
                prev_elev, curr_elev);
        POSTMSG(WARNING, "Could not compute GDZ field");
      }
      
      if (_currTilt->filter() != 0)
      {
        PMU_auto_register("Filtering");
        POSTMSG(WARNING, "Filtering failed at %f degrees", curr_elev);
      }
    }
  }
  
  // Write out the tilt
  
  PMU_auto_register("Writing");
  if (_currTilt->write() != 0) {
    return -1;
  }

  return 0;

}

/**
 * _processEOV()
 */

int DataMgr::_processEOV() 
{

  if (_processTilts() != 0)
  {
    POSTMSG(ERROR, "Could not process tilts");
    return -1;
  }
   
  // We can process _currTilt now and finish the volume

  if (!_currTilt->tiltEmpty())
  {
    double curr_elev = _currTilt->getElevAngle();

    // Is the tilt that we want to filter in filtering range?
    // If so filter it.

    if (curr_elev <= _maxElev && curr_elev >= _minElev)
    {
      if (_currTilt->filter() != 0)
      {
	POSTMSG(WARNING, "Filtering failed at %f degrees", curr_elev);
	PMU_auto_register("Filtering");
      }
    }

    // Write out the tilt

    if (_currTilt->write() != 0)
      return -1;

    PMU_auto_register("Writing");
  }
   
  // Clean up for next volume

  _prevTilt->clear();
  _currTilt->clear();

  return 0;
}


