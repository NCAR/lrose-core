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
 * @file RefractInput.hh
 * @class RefractInput
 * Base class for Refract input classes.
 * @date 12/1/2008
 *
 */

#ifndef RefractInput_H
#define RefractInput_H

#include <string>
#include <Refract/RefDebug.hh>
#include <Refract/FieldWithData.hh>
class DsMdvx;
class MdvxField;
class DateTime;

/** 
 * @class RefractInput
 */

class RefractInput : public RefDebug
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] raw_iq_in_input Flag indicating whether the input stream
   *                            contains raw I/Q values.  If false, the input
   *                            stream must contain NIQ/AIQ values.
   * @param[in] raw_i_field_name The name of the raw I field in the input
   *                             stream, if included.
   * @param[in] raw_q_field_name The name of the raw ! field in the input
   *                             stream, if included.
   * @param[in] niq_field_name The name of the NIQ field in the input stream,
   *                           if included.
   * @param[in] aiq_field_name The name of the AIQ field in the input stream,
   *                           if included.
   * @param[in] quality_from_width
   * @param[in] quality_field_name
   * @param[in] snr_in_input
   * @param[in] snr_field_name The name of the signal-to-noise ratio field in
   *                           the input stream.
   * @param[in] power_field_name
   * @param[in] input_niq_scale Scale value for input NIQ values.  The NIQ
   *                            values from the input file are multiplied
   *                            by this value before they are used.
   * @param[in] invert_target_angle_sign Flag indicating whether to invert the
   *                                     target angle sign when computing I/Q
   *                                     from NIQ/AIQ.
   * @param[in] elevation_num Elevation number to use in the input MDV files.
   * @param[in] num_output_beams Number of beams in the output fixed grid.
   * @param[in] num_output_gates Number of gates in the output fixed grid.
   * @param[in] debug_lat Debug point
   * @param[in] debug_lon Debug point
   * @param[in] debug_npt Debug number of points
   */
  RefractInput(bool raw_iq_in_input,
	       const std::string &raw_i_field_name,
	       const std::string &raw_q_field_name,
	       const std::string &niq_field_name,
	       const std::string &aiq_field_name,
	       bool quality_from_width,
	       const std::string &quality_field_name,
	       bool snr_in_input,
	       const std::string &snr_field_name,
	       const std::string &power_field_name,
	       double input_niq_scale,
	       bool invert_target_angle_sign,
	       int elevation_num,
	       int num_output_beams, int num_output_gates,
	       double debug_lat, double debug_lon, int debug_npt);
  
  /**
   * @brief Constructor
   *
   * @param[in] raw_iq_in_input Flag indicating whether the input stream
   *                            contains raw I/Q values.  If false, the input
   *                            stream must contain NIQ/AIQ values.
   * @param[in] raw_i_field_name The name of the raw I field in the input
   *                             stream, if included.
   * @param[in] raw_q_field_name The name of the raw ! field in the input
   *                             stream, if included.
   * @param[in] niq_field_name The name of the NIQ field in the input stream,
   *                           if included.
   * @param[in] aiq_field_name The name of the AIQ field in the input stream,
   *                           if included.
   * @param[in] quality_from_width
   * @param[in] quality_field_name
   * @param[in] snr_in_input
   * @param[in] snr_field_name The name of the signal-to-noise ratio field in
   *                           the input stream.
   * @param[in] power_field_name
   * @param[in] input_niq_scale Scale value for input NIQ values.  The NIQ
   *                            values from the input file are multiplied
   *                            by this value before they are used.
   * @param[in] invert_target_angle_sign Flag indicating whether to invert the
   *                                     target angle sign when computing I/Q
   *                                     from NIQ/AIQ.
   * @param[in] min_elevation_angle The minimum elevation angle for sweeps.
   * @param[in] max_elevation_angle The maximum elevation angle for sweeps.
   * @param[in] num_output_beams Number of beams in the output fixed grid.
   * @param[in] num_output_gates Number of gates in the output fixed grid.
   * @param[in] debug_lat Debug point
   * @param[in] debug_lon Debug point
   * @param[in] debug_npt Debug number of points
   */
  RefractInput(bool raw_iq_in_input,
	       const std::string &raw_i_field_name,
	       const std::string &raw_q_field_name,
	       const std::string &niq_field_name,
	       const std::string &aiq_field_name,
	       bool quality_from_width,
	       const std::string &quality_field_name,
	       bool snr_in_input,
	       const std::string &snr_field_name,
	       const std::string &power_field_name,
	       double input_niq_scale,
	       bool invert_target_angle_sign,
	       double min_elevation_angle,
	       double max_elevation_angle,
	       int num_output_beams, int num_output_gates,
	       double debug_lat, double debug_lon, int debug_npt);

  
  /**
   * @brief Destructor
   */

  virtual ~RefractInput();

  /**
   * Read in the data for a particular scan to process.  Interpolate the
   *        data onto an indexed grid.
   *
   * @param[in] data_time  Time to search for
   * @param[in] search_margin  Allowed time diff
   * @param[in] url  Where to look
   * @param[out] mdvx The MDV file.
   *
   * @return Returns true on success, false on failure.
   */

  bool getScan(const DateTime &data_time, int search_margin,
	       const std::string &url, DsMdvx &mdvx);

  /**
   * @brief Read in the data for the next scan to process.  Interpolate the
   *        data onto an indexed grid.
   *
   * @param[in] input_file_path Input file path.
   * @param[in] host  Machine with file
   * @param[out] mdvx The MDV file.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool getNextScan(const std::string &input_file_path,
			   const std::string &host, DsMdvx &mdvx);
    
  /**
   * @return field for the raw I field
   *
   * @param[in] source  Object that should have I
   */
  FieldWithData getI(DsMdvx &source) const;

  /**
   * @return field for raw Q
   *
   * @param[in] source  Object that should have Q
   */
  FieldWithData getQ(DsMdvx &source) const;

  /**
   * @return field for SNR
   *
   * @param[in] source  Object that should have SNR
   */
  FieldWithData getSNR(DsMdvx &source) const;

  /**
   * @return field for Quality
   *
   * @param[in] source  Object that should have Quality
   */
  FieldWithData getQuality(DsMdvx &source) const;

  /**
   * @return field for Phase Error
   *
   * @param[in] source  Object that should have Phase Error
   */
  FieldWithData getPhaseError(DsMdvx &source) const;

  /**
   * @return field name for I
   */
  inline std::string fieldNameI(void) const {return _rawIFieldName;}

  /**
   * @return field name for Q
   */
  inline std::string fieldNameQ(void) const {return _rawQFieldName;}
  
  /**
   * @return field name for SNR
   */
  inline std::string fieldNameSNR(void) const {return _snrFieldName;}

  /**
   * @return field name for quality
   */
  inline std::string fieldNameQuality(void) const {return _qualityFieldName;}


protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * Allowed extra noise in NIQ data
   */
  static const double OFFSET_ABOVE_AVERAGE;

  /**
   *  Maximum SNR value considered noise
   */
  static const double SNR_NOISE_MAX;
  
  /**
   *  noise = 10^DM_NOISE / 10
   */ 
  static const double DM_NOISE;
  
  /**
   * @brief Flag indicating whether the input stream contains raw I/Q values.
   *        If false, the input stream must contain NIQ/AIQ values.
   */
  bool _rawIQinInput;
  
  /**
   * @brief The name of the raw I field in the input stream, if included.
   */
  std::string _rawIFieldName;
  
  /**
   * @brief The name of the raw ! field in the input stream, if included.
   */
  std::string _rawQFieldName;
  
  /**
   * @brief The name of the NIQ field in the input stream, if included.
   */
  std::string _niqFieldName;
  
  /**
   * @brief The name of the AIQ field in the input stream, if included.
   */
  std::string _aiqFieldName;
  
  
  /**
   * True if quality is computed from input width data
   */
  bool _qualityFromWidth;
  
  /**
   * @brief The name of the quality field in the input stream, if included.
   */
  std::string _qualityFieldName;
  
  /**
   * True if SNR data is in input
   */
  bool _snrInInput;

  /**
   * @brief The name of the signal-to-noise ratio field in the input stream.
   */
  std::string _snrFieldName;

  /**
   * Name of power in the input
   */
  std::string _powerFieldName;

  /**
   * Name of phase error in the input
   */
  std::string _phaseErrorFieldName;

  /**
   * @brief Scale value for input NIQ values.  The NIQ values from the
   *        input file are multiplied by this value before they are used.
   */
  double _inputNiqScale;
  
  /**
   * @brief Flag indicating whether to invert the target angle sign when
   *        computing I/Q from NIQ/AIQ.
   */
  bool _invertTargetAngleSign;
  
  /**
   * @brief Flag indicating whether to use elevation number or elevation
   *        angle limits when finding sweeps.
   */
  bool _useElevationNum;
  
  /**
   * @brief Elevation number to use from the input MDV files.
   */
  size_t _elevationNum;
  
  /**
   * @brief The minimum elevation angle to use.
   */
  double _minElevationAngle;
  
  /**
   * @brief The maximum elevation angle to use.
   */
  double _maxElevationAngle;
  
  /**
   * @brief Number of gates in the output data.
   */
  int _numOutputGates;
  
  /**
   * @brief Number of beams in the output data.
   */
  int _numOutputBeams;
  
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Calculate the I/Q fields from the NIQ/AIQ fields given in the
   *        input file.  Replace the NIQ/AIQ fields with the calculated I/Q
   *        fields so the file will look the same as it would if the I/Q
   *        fields were read from disk.
   *
   * @param[in,out] niq_field The input NIQ field which is replaced with
   *                          the I field on return.
   * @param[in,out] aiq_field The input AIQ field which is replaced with
   *                          the Q field on return.
   * @param[in] snr_field The input SNR field.
   */

  void _calcIQ(MdvxField &niq_field, MdvxField &aiq_field,
	       const MdvxField &snr_field) const;
  

  /**
   * @brief Read the specified input file.  The returned file will have the
   *        raw I field as the first field and the raw Q field as the second
   *        and the SNR field as the third.
   *
   * @param[in] file_path The full path of the input file.
   * @param[out] mdvx The input file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readInputFile(DsMdvx &mdvx);
  

  void _calcSnr(MdvxField &power_field) const;
  
  /**
   * @brief Reposition the input data into the locations required for the
   *        output data.
   *
   * @param[in,out] mdvx The MDV file.
   */

  void _repositionData(DsMdvx &mdvx) const;
  

};

#endif
