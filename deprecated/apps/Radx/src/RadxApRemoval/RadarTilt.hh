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
 * @file RadarTilt.hh
 *
 * @class RadarTilt
 *
 * RadarTilt manages the processing of a single radar tilt.
 *  
 * @date 4/5/2002
 *
 */

#ifndef RadarTilt_HH
#define RadarTilt_HH

using namespace std;

#include <string>
#include <vector>
#include <map>

#include <dataport/port_types.h>
#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsBeamData.hh>
#include <rapformats/DsBeamDataFieldParms.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsRadarParams.hh>

#include "Feature.hh"
#include "Filter.hh"
#include "FilterBeamInfo.hh"
#include "FilterList.hh"
#include "InterestFunction.hh"
#include "TerrainMask.hh"


/** 
 * @class RadarTilt
 */

class RadarTilt 
{
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] output_queue The FMQ to use for outputing the filtered fields.
   * @param[in] dbz_az_radius The number of beams on either side of the current
   *                          beam used for calculating reflectivity-based
   *                          feature fields.
   * @param[in] vel_az_radius The number of beams on either side of the current
   *                          beam used for calculating velocity-based and
   *                          spectrum width-based feature fields.
   * @param[in] dbz_gate_radius The number of gates on either side of the
   *                            current gate used for calculating
   *                            reflectivity-based feature fields.
   * @param[in] vel_gate_radius The number of gates on either side of the
   *                            current gate used for calculating
   *                            velocity-based and spectrum width-based
   *                            feature fields.
   * @param[in] gate_spacing The expected gate spacing, in km, for the input
   *                         data.  Beams with a different gate spacing on
   *                         the input will be ignored.
   * @param[in] max_gates The maximum number of gates in any beam in the input
   *                      data.  Beams with more gates than this will be
   *                      ignored.
   * @param[in] dbz_name Name of the reflectivity field in the input.
   * @param[in] vel_name Name of the velocity field in the input.
   * @param[in] sw_name Name of the spectrum width field in the input.
   * @param[in] sc_spin_thresh Threshold value used for calculating the sea
   *                           clutter spin feature field.
   * @param[in] ap_spin_thresh Threshold value used for calculating the AP
   *                           spin feature field.
   * @param[in] p_spin_thresh Threshold value used for calculating the
   *                          precipitation spin feature field.
   * @param[in] delta_az
   * @param[in] slant_range_dist
   * @param[in] terrain_mask Reference to the terrain mask object to use in
   *                         the filters.
   * @param[in] filter_list The list of filters to use.
   * @param[in] feature_fields Reference to the feature field manager.
   * @param[in] range_wt_func
   * @param[in] filter_names List of input fields to filter.
   */

  RadarTilt(DsRadarQueue& output_queue,
	    const int dbz_az_radius, const int vel_az_radius,
	    const int dbz_gate_radius, const int vel_gate_radius,
	    const double gate_spacing, const int max_gates,
	    const char *dbz_name, const char *vel_name, const char* sw_name, 
	    const double sc_spin_thresh, const double ap_spin_thresh,
	    const double p_spin_thresh,
	    const double delta_az, const double slant_range_dist, 
	    TerrainMask& terrain_mask, const FilterList &filter_list,
	    Feature& feature_fields, InterestFunction& range_wt_func,
	    vector< string* >& filter_names);

  /**
   * @brief Destructor
   */

  ~RadarTilt();
   
  /**
   * @brief Add a message to this tilt.
   *
   * @param[in] radar_msg The message to add to the tilt.
   * @param[in] content The message content mask indicating what information
   *                    is included in the message.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int addMsg(DsRadarMsg &radar_msg, const int content);

  /**
   * @brief Set up the data to use in filtering.
   */

  void setData();

  /**
   * @brief Filter this tilt.
   *
   * @param[in] upper_tilt The tilt above the current tilt.  The upper tilt
   *                       is needed for calculating some of the feature
   *                       fields.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int filter(RadarTilt *upper_tilt = NULL);

  /**
   * @brief Write the tilt and interest if necessary.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int write();

  /**
   * @brief Clear out the list of messages, etc.
   */

  void clear();

  /**
   * @brief Get the volume number for this tilt.
   *
   * @return Returns the volume number.
   */

  int getVolNum() { return _volNum; }

  /**
   * @brief Get the tilt number for this tilt.
   *
   * @return Returns the tilt number.
   */

  int getTiltNum() { return _tiltNum; }
  
  /**
   * @brief Get the elevation angle for this tilt.
   *
   * @return Returns the elevation angle in degrees.
   */

  double getElevAngle() { return _elevAngle; }

  /**
   * @brief See if there is a reflectivity field in the input.
   *
   * @return Returns true if the input data has a reflectivity field, false
   *         otherwise.
   */

  bool dbzExists() const { return ( _dbzFieldIndex >= 0 ); }

  /**
   * @brief Get the reflectivity value for the given gate as a float.
   *
   * @param[in] ibeam Beam index for the gate.
   * @param[in] igate Gate index for the gate.
   *
   * @return Returns the unscaled reflectivity value for the given gate.
   */

  float getDbzValF(const int ibeam, const int igate) const;

  /**
   * @brief Get the elevation for the given beam.
   *
   * @param[in] ibeam Index for the beam.
   *
   * @return Returns the elevation angle for the given beam.
   */

  double getBeamElev(const int ibeam) const;

  /**
   * @brief Get the beam index for the beam with the given azimuth angle.
   *
   * @param[in] azimuth Azimuth angle in degrees.
   *
   * @return Returns the beam index for the angle.
   */

  int getAzimuthIdex(const double azimuth);

  /**
   * @brief Check to see if the current tilt has any radar data in it.
   *
   * @return Returns true if the tilt is empty, false otherwise.
   */

  bool tiltEmpty() { return _empty; }


private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief Number of indices included in the _azIndices array.
   */

  static const int AZ_INDEX_LEN;

  /**
   * @brief Azimuth tolerance.
   */

  static const int AZ_TOL;

  /**
   * @brief A summary debug message is printed once every RADAR_SUMMARY_COUNT
   *        output beams.
   */

  static const int RADAR_SUMMARY_COUNT;

  /**
   * @brief The minimum fraction of gates in a beam that must contain valid
   *        reflectivity data in order for the reflectivity-based feature
   *        fields to be calculated.
   */

  static const float MIN_GOOD_FRACTION;

  /**
   * @brief When comparing the data gate spacing to the expected gate
   *        spacing, the values must be within this tolerance.  This is
   *        used to account for rounding problems.
   */

  static const float GATE_SPACING_TOL;

  /**
   * @brief Data value used to indicate a missing velocity or spectrum
   *        width median value.
   */

  static const double MISSING_MED_VALUE;


  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Flag indicating whether we are processing the first beam
   *        in the current tilt.
   */

  bool _firstBeamInTilt;

  /**
   * @brief Flag indicating whether the current tilt contains any radar
   *        data.
   */

  bool _empty;

  /**
   * @brief Flag indicating whether the data in this tilt has been filtered.
   */

  bool _filtered;

  /**
   * @brief Flag indicating whether the tilt initiation has failed.  If the
   *        initiation fails, no other processing can be done.
   */

  bool _initFailed;

  /**
   * @brief Flag indicating whether this tilt contained a start-of-volume
   *        flag.  This is currently used to control whether a start-of-volume
   *        flag is written to the output feature and interest queues.  I'm
   *        wondering if this is necessary since I believe we trigger off of
   *        the start/end-of-volume flags so we must have received them.
   */

  bool _startOfVolume;

  /**
   * @brief Flag indicating whether this tilt contained a end-of-volume
   *        flag.  This is currently used to control whether a end-of-volume
   *        flag is written to the output feature and interest queues.  I'm
   *        wondering if this is necessary since I believe we trigger off of
   *        the start/end-of-volume flags so we must have received them.
   */

  bool _endOfVolume;

  /**
   * @brief Flag indicating whether the data in this tilt has been set.
   *        The data setting includes handling the data wrapping and 
   *        calculating the needed median values.
   */

  bool _dataSet;

  /**
   * @brief Index of the reflectivity field in the input data stream.
   */

  int _dbzFieldIndex;

  /**
   * @brief Index of the velocity field in the input data stream.
   */

  int _velFieldIndex;

  /**
   * @brief Index of the spectrum width field in the input data stream.
   */

  int _swFieldIndex;

  /**
   * @brief Name of the reflectivity field in the input data stream.
   */

  string _dbzFieldName;

  /**
   * @brief Name of the velocity field in the input data stream.
   */

  string _velFieldName;

  /**
   * @brief Name of the spectrum width field in the input data stream.
   */

  string _swFieldName;

  /**
   * @brief Flag indicating whether the reflectivity field exists in
   *        the tilt above this one.
   */

  bool _upperDbzExists;

  /**
   * @brief The start time for the tilt.
   */

  time_t _startTime;

  /**
   * @brief The end time for the tilt.
   */

  time_t _endTime;

  /**
   * @brief The number of fields in the input data stream.
   */

  int _nFields;

  /**
   * @brief The maximum number of gates allowed in the input data stream.
   *        If a beam with a larger number of gates is received, that beam
   *        will be ignored.
   */

  int _maxGates;

  /**
   * @brief The number of bytes used for representing the data in the input
   *        stream.  All fields in the input stream must use the same byte
   *        width.
   */

  int _byteWidth;

  /**
   * @brief The number of beams in this tilt.
   */

  int _nBeams;

  /**
   * @brief The expected gate spacing for the input data stream in km.
   *
   */

  double _gateSpacing;

  /**
   * @brief The beam width of the stored data.  The stored data is basically
   *        remapped to a regular "grid" with a constant delta azimuth.
   */

  double _deltaAzimuth;

  /**
   * @brief The distance to the first gate of the input data in km.
   */

  double _startRange;

  /**
   * @brief The elevation angle for this tilt.
   */

  double _elevAngle;

  /**
   * @brief The noise floor for the input data.
   */

  double _noiseFloor;

  /**
   * @brief The radar constant for the input data.
   */

  double _radarConstant;
  
  /**
   * @brief The tilt number for this tilt.
   */

  int _tiltNum;
  
  /**
   * @brief The volume number for this tilt.
   */

  int _volNum;
  
  /**
   * @brief Pointer to the radar parameters in the first received radar
   *        message.
   *
   * @todo Figure out what this is really used for and document appropriately.
   */

  DsRadarParams *_sampleParams;
  
  /**
   * @brief List of field paramters for the input data stream.
   */

  vector< DsFieldParams* > _inputFieldParams;
  
  /**
   * @brief List of received radar messages for this tilt.
   */

  // vector< RadarMsg* > _radarMsgs;

  /**
   * @brief Number of gates in the terrain dataset.
   */

  int _terrainNGates;

  /**
   * @brief Pointer to the terrain mask data.  Note that this class does not
   *        own the memory associated with this data.
   */

  float *_terrainVals;

  /**
   * @brief List of input fields to filter.
   */

  vector< string* > &_filterFieldNames;

  /**
   * @brief Beam index.  I don't really understand how this is being used.
   */

  int _beamIndex;

  /**
   * @brief The maximum number of azimuths on either side of the current
   *        beam that will be used when calculating the feature fields.
   */

  int _maxAzRadius;

  /**
   * @brief The number of azimuths on either side of the current beam
   *        that will be used when calculating the reflectivity-based
   *        feature fields.
   */

  int _dbzAzRadius;

  /**
   * @brief The number of azimuths on either side of the current beam
   *        that will be used when calculating the velocity-based feature
   *        fields.
   */

  int _velAzRadius;

  /**
   * @brief The number of gates on either side of the current gate that will
   *        be used when calculating the reflectivity-based feature fields.
   */

  int _dbzGateRadius;

  /**
   * @brief The number of gates on either side of the current gate that will
   *        be used when calculating the velocity-based feature fields.
   */

  int _velGateRadius;

  /**
   * @brief 
   */

  int _slantRangeNGates;
  
  /**
   * @brief Reflectivity change threshold used when calculating the sea
   *        clutter spin feature field.
   */

  double _scSpinThreshold;

  /**
   * @brief Reflectivity change threshold used when calculating the AP
   *        spin feature field.
   */

  double _apSpinThreshold;

  /**
   * @brief Reflectivity change threshold used when calculating the
   *        precipitation spin feature field.
   */

  double _pSpinThreshold;

  /**
   * @brief Azimuth difference tolerance used to check for missing beams.
   */

  double _azimuthDiffTol;

  /**
   * @brief Stores the elevation angle for each beam.  Used in calculating
   *        RSINZ.
   */

  vector< double > _elevAngles;

  /**
   * @brief Stores the azimuth angle for each beam.
   */

  vector< double > _azimuths;

  /**
   * @brief The list of beam data parameters for each field in the tilt.
   */

  vector< DsBeamDataFieldParms > _dataParams;

  /**
   * @brief The data for all of the beams in the tilt.e
   */

  vector< DsBeamData > _dataPtrs;

  /**
   * @brief This somehow maps the azimuth angle value into a beam index
   *        value.  I haven't figured out exactly how this works yet.
   */

  int *_azIndices;

  /**
   * @brief List of filters to be applied to the input data.
   */

  FilterList _filterList;

  /**
   * @brief The median velocity values for this tilt.  There is a pointer
   *        for each azimuth in the tilt and a double value for each gate
   *        within the azimuth.
   */

  vector< double* > _medianVel;

  /**
   * @brief The median spectrum width values for this tilt.  There is a pointer
   *        for each azimuth in the tilt and a double value for each gate
   *        within the azimuth.
   */

  vector< double* > _medianSw;

  /**
   * @brief Range weight function.
   */

  InterestFunction &_rangeWtFunc;

  /**
   * @brief The object that manages the feature fields.
   */

  Feature &_featureFields;

  /**
   * @brief Counter used for determining when to display output information
   *        to the user.
   */

  int _summaryCount;

  /**
   * @brief The FMQ used for writing the output data.
   */

  DsRadarQueue &_outputQueue;


  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Initialize the stored radar information based on the radar
   *        parameters in this first message.
   *
   * @param[in] radar_msg The message containing the radar information.
   *
   * @pre radar_msg is assumed to have all of the radar parameter information
   *      included.
   */

  // void _initMsgs(RadarMsg &radarMsg);

  /**
   * @brief Reset all of the information in the internal data arrays.
   */

  void _resetDataArrays();

  /**
   * @brief Calculate the GDZ feature fields.
   *
   * @param[in] upper_tilt Pointer to the information for the tilt above
   *            this one.
   * @param[in] ibeam Beam index for the beam in this tilt.
   * @param[in] upper_ibeam Beam index for the beam in the upper tilt.
   * @param[in] igate Gate number for the gate.  The gate number will be the
   *                  same for both tilts.
   */

  void _calcGdzFields(const RadarTilt* upper_tilt, 
		      const int ibeam, const int upper_ibeam,
		      const int igate);

  /**
   * @brief Calculate the spin feature field value.
   *
   * @param[in] first_val First reflectivity value along the beam.
   * @param[in] second_val Second reflectivity value along the beam.
   * @param[in] threshold The reflectivity difference threshold used to
   *                      determine spin.
   * @param[in] first_gate Flag indicating whether this is the first gate
   *                       in the beam.
   * @param[in,out] last_spin_positive Flag indicating whether the previous
   *                                   spin was positive or negative.  If
   *                                   true, the last spin was positive; if
   *                                   false, the last spin was negative.
   * @param[in,out] found_first_delta Flag indicating whether we have found
   *                                  a place in the current beam where the
   *                                  reflectivity difference has exceeded
   *                                  the threshold.
   * @param[in,out] cnd_spin_change The number of spin changes that have
   *                                occurred so far in this beam.
   */

  void _spinEval(const float first_val, 
		 const float second_val,
		 const float threshold,
		 const bool first_gate, 
		 bool &last_spin_positive,
		 bool &found_first_delta, 
		 int &cnt_spin_change) const;

  /**
   * @brief Save the given feature value and calculate the interest values
   *        associated with it.
   *
   * @param[in] int_type The type of feature field.
   * @param[in] field_val The feature field value.
   * @param[in] ibeam The beam index for this gate.
   * @param[in] igate The gate index for this gate.
   */

  void _calcInterest(const FilterBeamInfo::InterestType int_type, 
		     const double field_val, const int ibeam, 
		     const int igate);

  /**
   * @brief Calculate the median values for the tilt.
   *
   * @param[in] field_index    The field to use for the calculations.
   * @param[out] median_vector The associated median vector.
   */

  void _medianFilter(const size_t field_index,
		     vector< double* > &median_vector);

  /**
   * @brief Find the median for the given list of values.
   *
   * @param[in] val_array List of values.
   * @param[in] count Number of values in the list.
   */

  double _calcMedian(double *val_array, const int count) const;

  /**
   * @brief Write the filtered tilt data to the output FMQ.
   *
   * @return Returns 0 on success; -1 on failure.
   */

  int _writeFiltered();

  /**
   * @brief Write the unfiltered tilt data to the output FMQ.
   *
   * @return Returns 0 on success; -1 on failure.
   *
   * @todo Determine how this is different from writeFiltered() and see if
   *       we can combine the two methods.
   */

  int _passThrough();


  ////////////////////
  // Static members //
  ////////////////////

  /**
   * @brief Comparison function for double values used by qsort.
   *
   * @param[in] d1 First data value to compare.
   * @param[in] d2 Second data value to compare.
   *
   * @return Returns -1 if the first value is less than the second
   *         value, 1 if the first value is greater than the second value,
   *         and 0 if the values are equal.
   */

  static int _doubleCompare(const void *d1, const void *d2)
  {
    if (*(double *)d1 < *(double *)d2)
      return -1;
    if (*(double *)d1 == *(double *)d2)
      return 0;
    if (*(double *)d1 > *(double *)d2)
      return 1;

    return 0;
  }

};

#endif
