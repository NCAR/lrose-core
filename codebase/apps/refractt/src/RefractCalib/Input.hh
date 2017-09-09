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
 * @file Input.hh
 * @class Input
 * Base class for Refract input classes.
 * @date 12/1/2008
 *
 */

#ifndef Input_H
#define Input_H

#include <string>
class DsMdvx;
class MdvxField;

/** 
 * @class Input
 */

class Input
{
  
public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief The index of the I field in the input file.
   */

  static const int I_FIELD_INDEX;

  /**
   * @brief The index of the Q field in the input file.
   */

  static const int Q_FIELD_INDEX;

  /**
   * @brief The index of the SNR field in the input file.
   */

  static const int SNR_FIELD_INDEX;

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
   * @param[in] snr_field_name The name of the signal-to-noise ratio field in
   *                           the input stream.
   * @param[in] input_niq_scale Scale value for input NIQ values.  The NIQ
   *                            values from the input file are multiplied
   *                            by this value before they are used.
   * @param[in] invert_target_angle_sign Flag indicating whether to invert the
   *                                     target angle sign when computing I/Q
   *                                     from NIQ/AIQ.
   * @param[in] elevation_num Elevation number to use in the input MDV files.
   * @param[in] num_output_beams Number of beams in the output fixed grid.
   * @param[in] num_output_gates Number of gates in the output fixed grid.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   */

  Input(const bool raw_iq_in_input,
	const std::string &raw_i_field_name,
	const std::string &raw_q_field_name,
	const std::string &niq_field_name,
	const std::string &aiq_field_name,
	const std::string &snr_field_name,
	const double input_niq_scale,
	const bool invert_target_angle_sign,
	const int elevation_num,
	const int num_output_beams,
	const int num_output_gates,
	const bool debug_flag = false,
	const bool verbose_flag = false);
  
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
   * @param[in] snr_field_name The name of the signal-to-noise ratio field in
   *                           the input stream.
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
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   */

  Input(const bool raw_iq_in_input,
	const std::string &raw_i_field_name,
	const std::string &raw_q_field_name,
	const std::string &niq_field_name,
	const std::string &aiq_field_name,
	const std::string &snr_field_name,
	const double input_niq_scale,
	const bool invert_target_angle_sign,
	const double min_elevation_angle,
	const double max_elevation_angle,
	const int num_output_beams,
	const int num_output_gates,
	const bool debug_flag = false,
	const bool verbose_flag = false);
  
  /**
   * @brief Destructor
   */

  virtual ~Input();


  /**
   * @brief Read in the data for the next scan to process.  Interpolate the
   *        data onto an indexed grid.
   *
   * &param[in] input_file_path Input file path.
   * @param[out] mdvx The MDV file.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool getNextScan(const std::string &input_file_path,
			   const std::string &host, DsMdvx &mdvx);


protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief 
   */

  static const double OFFSET_ABOVE_AVERAGE;

  /**
   * @brief
   */

  static const double SNR_NOISE_MAX;
  
  /**
   * @brief
   */

  static const double DM_NOISE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  
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
   * @brief The name of the signal-to-noise ratio field in the input stream.
   */

  std::string _snrFieldName;
  
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
  
  
  ////////////////////
  // Static methods //
  ////////////////////

  /**
   * @brief Calculate the square of the given value.
   *
   * @param[in] value Value to square.
   *
   * @return Returns the square of the given input value.
   */

  static double SQR(double value)
  {
    return value * value;
  }
  

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

  void _calcIQ(MdvxField &niq_field,
	       MdvxField &aiq_field,
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

  bool _readInputFile(const std::string &file_path,
		      const std::string &host,
		      DsMdvx &mdvx) const;
  

  /**
   * @brief Reposition the input data into the locations required for the
   *        output data.
   *
   * @param[in,out] mdvx The MDV file.
   */

  void _repositionData(DsMdvx &mdvx) const;
  

};

#endif
