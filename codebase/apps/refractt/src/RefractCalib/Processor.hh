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
 * @file Processor.hh
 *
 * @class Processor
 *
 * Class for doing all of the data processing.
 *  
 * @date 1/15/2008
 *
 */

#ifndef Processor_HH
#define Processor_HH

#include <math.h>
#include <string>
#include <vector>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Refract/RefFile.hh>
#include <Refract/TargetData.hh>

#include "Input.hh"

using namespace std;


/** 
 * @class Processor
 */

class Processor
{
 public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief Invalid data value.
   */

  static const float INVALID;

  /**
   * @brief Very large data value.
   */

  static const float VERY_LARGE;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  Processor();
  

  /**
   * @brief Destructor
   */

  virtual ~Processor();
  

  /**
   * @brief Initialize the local data.
   *
   * @param[in] num_azim Number of azimuths in the data.
   * @param[in] num_range_bins Number of range bins in the data.
   * @param[in] r_min Minimum range gate with ground echo.
   * @param[in] beam_width Antenna beam width in degrees.
   * @param[in] side_lobe_power Echoes below this value of integrated power
   *                            are considered to be from sidelobes.
   * @param[in] ref_file_path Reference file path.
   * @param[in] input_handler Input handler object.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init(const int num_azim, const int num_range_bins,
	    const int r_min, const double beam_width,
	    const double side_lobe_power,
	    const string &ref_file_path,
	    Input *input_handler,
	    const bool debug_flag = false, const bool verbose_flag = false);
  

  /**
   * @brief Set the reference N value to the given value.
   *
   * @param[in] n_value The N value to use.
   */

  void setNValue(const double n_value)
  {
    _nValue = n_value;

    cerr << "---> Setting N value to " << _nValue << endl;
  }
  

  /**
   * @brief Set the reference N value to the value calculated from the
   *        given values.
   *
   * @param[in] pressure Pressure value in mb.
   * @param[in] temperature Temperature value in C.
   * @param[in] dewpoint_temperature Dewpoint temperature value in C.
   */

  void setNValue(const double pressure, const double temperature,
		 const double dewpoint_temperature)
  {
    double vapor_pres = 6.112 * exp(17.67 * dewpoint_temperature /
				    (dewpoint_temperature + 243.5));
    _nValue = 77.6 * pressure / (temperature + 273.16) + 373250 *
      vapor_pres / SQR(temperature + 273.16);

    cerr << "---> Setting N value to " << _nValue << endl;
  }
  

  /**
   * @brief Set the debug MDV URL.
   *
   * @param[in] debug_mdv_url The desired debug MDV URL.
   */

  void setDebugMdvUrl(const string &debug_mdv_url)
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

  bool calibTargets(const vector< string > &file_list,
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

  bool findReliableTargets(const vector< string > &file_list,
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

  /**
   * @brief Debug flag.
   */

  bool _debug;

  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  
  /**
   * @brief Input handler object.
   */

  Input *_inputHandler;
  
  /**
   * @brief Number of azimuths in the data.
   */

  int _numAzim;

  /**
   * @brief Number of range bins in the data.
   */

  int _numRangeBins;
  
  /**
   * @brief Minimum range gate with ground echo.
   */

  int _rMin;

  /**
   * @brief Antenna beam width in degrees.
   */

  double _beamWidth;

  /**
   * @brief Echoes below this value of integrated power are considered to be
   *        from sidelobes.
   */

  double _sideLobePower;
  
  /**
   * @brief Full path of the reference file.
   */

  string _refFilePath;
  
  /**
   * @brief The reference N value used for this calibration.
   */

  double _nValue;

  /**
   * @brief Array for recording fluctuations in the SNR values.
   */

  float *_fluctSnr;

  /**
   * @brief Mean SNR field.
   */

  MdvxField *_meanSnrField;

  /**
   * @brief Array for storing the pixel count.
   */

  int *_pixelCount;

  /**
   * @brief
   */

  MdvxField *_sumAField;

  /**
   * @brief
   */

  MdvxField *_sumBField;

  /**
   * @brief
   */

  MdvxField *_sumPField;

  /**
   * @brief Field containing the I component difference from the previous
   *        scan.
   */

  MdvxField *_difPrevScanIField;

  /**
   * @brief Field containing the Q component difference from the previous
   *        scan.
   */

  MdvxField *_difPrevScanQField;

  /**
   * @brief The SNR field from the previous scan.
   */

  MdvxField *_oldSnrField;

  /**
   * @brief The SNR-based strength of the reference target.  This field is
   *        stored in the final calibration file.
   */

  MdvxField *_calibStrengthField;

  /**
   * @brief The average I component of the target for N equal to the reference
   *        N value (_nValue).  This field is stored in the final calibration
   *        file.
   */

  MdvxField *_calibAvIField;

  /**
   * @brief The average Q component of the target for N equal to the reference
   *        N value (_nValue).  This field is stored in the final calibration
   *        file.
   */

  MdvxField *_calibAvQField;

  /**
   * @brief The expected error of the target phase value.  This field is
   *        stored in the final calibration file.
   */

  MdvxField *_calibPhaseErField;

  /**
   * @brief The quality of the target.  This field is stored in the final
   *        calibration file.
   */

  MdvxField *_calibQualityField;

  /**
   * @brief The calculated NCP value for the target.  This field is stored
   *        in the final calibration file.
   */

  MdvxField *_calibNcpField;
  
  /**
   * @brief Flag indicating whether to generate debug MDV files.
   */

  bool _writeDebugMdvFiles;

  /**
   * @brief URL for the debug MDV files.
   */

  string _debugMdvUrl;
  
  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Calculate the square of the given value.
   *
   * @param[in] Value to square.
   *
   * @return Returns the square of the given value.
   */

  static double SQR(double value)
  {
    return value * value;
  }
  

  /**
   * @brief Allocate space for the global fields.
   *
   * @param[in] input_file The current input file. This is used as a reference
   *                       for how to size the global arrays.
   *
   * @return Returns true on success, false on failure.
   */

  bool _allocateGlobalFields(const DsMdvx &input_file);
  

  /**
   * @brief Create the given MDV field and fill the data with missing values.
   *
   * @param[in] base_field The base field used for determining the new field
   *                       attributes.
   * @param[in] field_name The new field's name.
   * @param[in] field_name_long The new field's long name.
   * @param[in] units The new field's units.
   *
   * @return Returns a pointer to the newly created field on success, 0 on
   *         failure.
   */

  MdvxField *_createField(const MdvxField &base_field,
			  const string &field_name,
			  const string &field_name_long,
			  const string &units) const;
  
  
  /**
   * @brief Create the given MDV field and fill the data with the given
   *        initial data value.
   *
   * @param[in] base_field The base field used for determining the new field
   *                       attributes.
   * @param[in] field_name The new field's name.
   * @param[in] field_name_long The new field's long name.
   * @param[in] units The new field's units.
   * @param[in] initial_data_value The initial value to use for the data.
   *
   * @return Returns a pointer to the newly created field on success, 0 on
   *         failure.
   */

  MdvxField *_createField(const MdvxField &base_field,
			  const string &field_name,
			  const string &field_name_long,
			  const string &units,
			  const fl32 initial_data_value) const;
  

  /**
   * @brief Create the given MDV field and fill the data with the given
   *        initial data values.
   *
   * @param[in] base_field The base field used for determining the new field
   *                       attributes.
   * @param[in] field_name The new field's name.
   * @param[in] field_name_long The new field's long name.
   * @param[in] units The new field's units.
   * @param[in] initial_data The array of initial values to use for the data.
   *
   * @return Returns a pointer to the newly created field on success, 0 on
   *         failure.
   */

  MdvxField *_createField(const MdvxField &base_field,
			  const string &field_name,
			  const string &field_name_long,
			  const string &units,
			  const fl32 *initial_data) const;
  

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

  bool _findReliableTargets(const DsMdvx &input_file,
			    MdvxField &av_i_field,
			    MdvxField &av_q_field) const;
  

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

  bool _findReliableTargets(const DsMdvx &input_file,
			    MdvxField &av_i_field,
			    MdvxField &av_q_field,
			    MdvxField &dif_from_ref_i_field,
			    MdvxField &dif_from_ref_q_field) const;
  

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

  void _writeDebugPhaseCalibFiles(const float *phase_targ,
				  const int num_azim,
				  const int num_gates) const;
  

};


#endif
