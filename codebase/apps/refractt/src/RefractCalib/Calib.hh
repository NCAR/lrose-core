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
 * @file Calib.hh
 * Class for doing all of the calibration data processing
 * @class Calib
 * Class for doing all of the calibration data processing
 */

#ifndef Calib_HH
#define Calib_HH

#include "Params.hh"
#include "Reader.hh"
#include <Refract/FieldWithData.hh>
#include <Refract/FieldDataPair.hh>
#include <dataport/port_types.h>
#include <toolsa/LogStream.hh>
#include <string>
#include <vector>
#include <cmath>

class DateTime;
class MdvxField;
class DsMdvx;

class Calib
{
 public:

  /**
   * @brief Constructor
   */

  Calib(const Params &params);
  
  /**
   * @brief Destructor
   */
  virtual ~Calib();

  /**
   * @brief Set the debug MDV URL.
   *
   * @param[in] debug_mdv_url The desired debug MDV URL.
   */
  inline void setDebugMdvUrl(const std::string &debug_mdv_url)
  {
    _writeDebugMdvFiles = true;
    _debugMdvUrl = debug_mdv_url;
  }
  
  /**
   * @brief Compute the refractive index associated with the period of
   *        interest based on parameter file values and the average phase
   *        of each targets for that period.
   *
   * @param[in] file_list List of calibration files.
   * @param[in] required_gate_spacing The gate spacing expected in the input
   *                                  files.  If any of the calibration files
   *                                  use a different gate spacing, an error
   *                                  will be thrown.
   *
   * @return Returns true on success, false on failure.
   */

  bool calibTargets(const std::vector< std::string > &file_list,
		    const double required_gate_spacing);
  

  /**
   * @brief Go through a list of files containing ground echo phase data and
   *        use them to determine their suitability (or their reliability) for
   *        the computation of N.
   *
   * @param[in] file_list List of target identification files.
   * @param[out] gate_spacing Gate spacing found in the input files.  This 
   *                          must be constant across all of the input files.
   *
   * @return Returns true on success, false on faillure.
   */

  bool findReliableTargets(const std::vector< std::string > &file_list,
			   double &gate_spacing);
  
 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief The minimum number of target identification files required to
   *        perform the target identification portion of the calibration.
   */
  static const size_t MINIMUM_TARGET_LIST_FILES;
  
  
  /////////////////////
  // Private members //
  /////////////////////

  const Params &_params;
  Reader _inputHandler;  /**< Input data handler */
  int _numAzim;  /**< Number of azimuths in the data. */
  int _numRangeBins;  /**< Number of range bins in the data. */
  int _rMin; /**< Minimum range gate with ground echo. */
  double _beamWidth;  /**< Antenna beam width in degrees. */
  double _sideLobePower;  /**< Echoes below this power are considered to be
			   *   from sidelobes. */
  std::string _refFilePath;  /**< Full path of the output reference file. */
  std::string _refUrl;  /**< Url for reference data */
  double _nValue;  /**< The reference N value used for this calibration. */
  float *_fluctSnr;  /**< Array for recording fluctuations in the SNR values. */
  FieldWithData _meanSnrField;  /**< Mean SNR field. */
  int *_pixelCount;  /**< Array for storing the pixel count. */
  FieldDataPair _sumABField;
  FieldWithData _sumPField;
  FieldDataPair _difPrevScanIQField; /**< IQ difference from the previous scan*/
  FieldWithData _oldSnrField; /**< The SNR field from the previous scan. */

  /**
   * Fields with _calb in their name are stored in the final calibration file
   */
  FieldWithData _calibStrengthField; /**<  SNR-based strength of reference
				      * target.  */
  FieldDataPair _calibAvIQField;  /**< Average IQ of target for N equal to
				   * the reference N value (_nValue).   */
  FieldWithData _calibPhaseErField; /**< Expected error of target phase value.*/
  FieldWithData _calibQualityField; /**< Quality of the target. */
  FieldWithData _calibNcpField; /**< The calculated NCP value for the target.*/

  bool _writeDebugMdvFiles; /**< Flag for generating debug MDV output */
  std::string _debugMdvUrl;  /**< URL for the debug MDV files. */
  
  /////////////////////
  // Private methods //
  /////////////////////

  bool _calibTargetsOneFile(int file_num, const std::string &file_iter,
			    time_t &lastDataTime);
  
  bool _findReliableTargetsOneFile(int file_num, const std::string &filename,
				   double &gate_spacing,
				   bool &first_file,
				   FieldDataPair **av_iq_field,
				   FieldDataPair **dif_from_ref_iq_field);
  bool _findReliableTargetsFirstFile(const DsMdvx &input_file,
				     const FieldDataPair &iq,
				     const FieldWithData &SNR,
				     double &gate_spacing, 
				     FieldDataPair **av_iq_field,
				     FieldDataPair **dif_from_ref_iq_field);
  bool _findReliableTargetsNonFirstFile(const DsMdvx &input_file,
					const FieldDataPair &iq,
					const FieldWithData &SNR,
					double gate_spacing,
					FieldDataPair &av_iq_field,
					FieldDataPair &dif_from_ref_iq_field);

  void _reliability(void);
  
  /**
   * @brief Allocate space for the global fields.
   *
   * @param[in] input_file The current input file. This is used as a reference
   *                       for how to size the global arrays.
   *
   * @return Returns true on success, false on failure.
   */

  bool _allocateGlobalFields(const FieldDataPair &IQ);
  

  /**
   * @brief Use the given scan to update the reliable targets information.
   *        This method is used for the first target identification scan 
   *        when we don't have any previous scan information.
   *
   * @param[in] input_file The current scan data.
   * @param[out] av_i_field The calculated average I field.
   * @param[out] av_q_field The calculated average Q field.
   *
   * @return Returns true on success, false on failure.
   */

  bool _findReliableTargets(const FieldDataPair &iq,
			    FieldDataPair &av_iq_field) const;
  

  /**
   * @brief Use the givenscan to update the reliable targets information.
   *        This method is used when we have previous scan information
   *        available.
   *
   * @param[in] input_file The current scan data.
   * @param[in,out] av_i_field The calculated average I field.
   * @param[in,out] av_q_field The calculated average Q field.
   * @param[out] dif_from_ref_i_field The I component of the phase difference
   *                                  between this scan and the reference scan.
   * @param[out] dif_from_ref_q_field The Q component of the phase difference
   *                                  between this scan and the reference scan.
   *
   * @return Returns true on success, false on failure.
   */

  bool _findReliableTargets(const FieldDataPair &iq,
			    const FieldWithData &SNR,
			    FieldDataPair &av_iq_field,
			    FieldDataPair &dif_from_ref_iq_field);
  

  /**
   * @brief Update the values in the master header for the debug file.
   *
   * @param[in,out] debug_file The debug file.
   * @param[in] input_file The associated input file.
   */

  void _updateDebugMasterHdr(DsMdvx &debug_file,
			     const DsMdvx &input_file) const;
  

  /**
   * @brief Write the calibration file.
   *
   * @param[in] data_time The timestamp to use for the calibration data.
   *
   * @return Returns true on success, false on failure.
   */

  bool _writeCalibrationFile(const DateTime &data_time) const;
  

  /**
   * @brief Write the debug phase calibration files.
   *
   * @param[in] phase_targ The phase target data.
   * @param[in] num_azim Number of azimuths in the data.
   * @param[in] num_gates Number of gates in the data.
   */

  void _writeDebugPhaseCalibFiles(const std::vector<double> &phase_targ,
				  const int num_azim,
				  const int num_gates) const;
  
  bool _setupInputs(DsMdvx &input_file, FieldDataPair &iq,
		    FieldWithData &SNR) const;

};


#endif
