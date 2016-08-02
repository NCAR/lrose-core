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
 *
 * @class Input
 *
 * Base class for Refract input classes.
 *  
 * @date 12/1/2008
 *
 */

#ifndef Input_H
#define Input_H

#include <iostream>
#include <string>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>

using namespace std;


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
   * @brief The index of the quality field in the input file.
   */

  static const int QUALITY_FIELD_INDEX;
  
  /**
   * @brief The index of the signal-to-noise ratio field in the input file.
   */

  static const int SNR_FIELD_INDEX;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  Input();
  
  /**
   * @brief Initialize the object.
   *
   * @param[in] input_url The URL for the input files.
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
   * @param[in] quality_field_name The name of the quality field in the
   *                               input stream.
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

  bool init(const string &input_url,
	    const bool raw_iq_in_input,
	    const string &raw_i_field_name,
	    const string &raw_q_field_name,
	    const string &niq_field_name,
	    const string &aiq_field_name,
	    const string &quality_field_name,
	    const string &snr_field_name,
	    const double input_niq_scale,
	    const bool invert_target_angle_sign,
	    const int elevation_num,
	    const int num_output_beams, const int num_output_gates,
	    const bool debug_flag = false, const bool verbose_flag = false);
  
  /**
   * @brief Initialize the object.
   *
   * @param[in] input_url The URL for the input files.
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
   * @param[in] quality_field_name The name of the quality field in the
   *                               input stream.
   * @param[in] snr_field_name The name of the signal-to-noise ratio field in
   *                           the input stream.
   * @param[in] input_niq_scale Scale value for input NIQ values.  The NIQ
   *                            values from the input file are multiplied
   *                            by this value before they are used.
   * @param[in] invert_target_angle_sign Flag indicating whether to invert the
   *                                     target angle sign when computing I/Q
   *                                     from NIQ/AIQ.
   * @param[in] min_elevation_angle Minimum elevation angle to process.
   * @param[in] max_elevation_angle Maximum elevation angle to process.
   * @param[in] num_output_beams Number of beams in the output fixed grid.
   * @param[in] num_output_gates Number of gates in the output fixed grid.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   */

  bool init(const string &input_url,
	    const bool raw_iq_in_input,
	    const string &raw_i_field_name, const string &raw_q_field_name,
	    const string &niq_field_name, const string &aiq_field_name,
	    const string &quality_field_name,
	    const string &snr_field_name,
	    const double input_niq_scale,
	    const bool invert_target_angle_sign,
	    const double min_elevation_angle,
	    const double max_elevation_angle,
	    const int num_output_beams, const int num_output_gates,
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
   * &param[in] data_time Desired data time for the input file.
   * @param[out] mdvx The MDV file.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool getNextScan(const DateTime &data_time,
			   DsMdvx &mdvx);


  /**
   * @brief Get the current input path, if this object uses input files.
   */

  virtual string getInputPath() const
  {
    return "";
  }
  

protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief
   */

  static const double MIN_DATA_COVER;

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
   * @brief The URL for the input files.
   */

  string _inputUrl;
  
  /**
   * @brief Flag indicating whether the input stream contains raw I/Q values.
   *        If false, the input stream must contain NIQ/AIQ values.
   */

  bool _rawIQinInput;
  
  /**
   * @brief The name of the raw I field in the input stream, if included.
   */

  string _rawIFieldName;
  
  /**
   * @brief The name of the raw ! field in the input stream, if included.
   */

  string _rawQFieldName;
  
  /**
   * @brief The name of the NIQ field in the input stream, if included.
   */

  string _niqFieldName;
  
  /**
   * @brief The name of the AIQ field in the input stream, if included.
   */

  string _aiqFieldName;
  
  /**
   * @brief The name of the quality field in the input stream.  This will
   *        be either spectrum width or CPA.
   */

  string _qualityFieldName;
  
  /**
   * @brief The name of the signal-to-noise ratio field in the input stream.
   */

  string _snrFieldName;
  
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
  
  int _numOutputBeams;
  double _outputGateSpacing;
  
  
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
   * @param[in] data_time Time for the desired input file.
   * @param[out] mdvx The input file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readInputFile(const DateTime &data_time,
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
