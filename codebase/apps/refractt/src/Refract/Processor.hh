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
 * @date 12/1/2008
 *
 */

#ifndef Processor_HH
#define Processor_HH

#include <string>
#include <sys/stat.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "TargetData.hh"
#include "TargetInfo.hh"

using namespace std;


/** 
 * @class Processor
 */

class Processor
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    QUALITY_FROM_WIDTH,
    QUALITY_FROM_CPA
  } quality_type_t;
  
    
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

  static const double VERY_LARGE;


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

  virtual ~Processor(void);
  

  /**
   * @brief Initialize the local data.
   *
   * @param[in] calib_file Pointer to the calibration file information.
   * @param[in] num_beams Number of beams in the data.
   * @param[in] num_gates Number of gates in the data.
   * @param[in] r_min Minimum rage gate of ground echo.
   * @param[in] frequency Frequency of the radar.
   * @param[in] quality_type The type of quality field being used.  This
   *                         determines the quality calculation to perform.
   * @param[in] min_consistency Minimum consistency of phase to accept the
   *                            N/deltaN measurements.  Higher means smaller
   *                            coverage of better data.
   * @param[in] do_relax Flag indicating whether to perform the relaxation
   *                     stage of the phase-fitting algorithm.
   * @param[in] n_smoothing_side_len Smoothing side length, in meters, to use
   *                                 in the N calculation.
   * @param[in] dn_smoothing_side_len Smoothing side length, in meters, to use
   *                                  in the delta N calculation.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init(DsMdvx *calib_file,
	    const int num_beams,
	    const int num_gates,
	    const int r_min,
	    const double frequency,
	    const quality_type_t quality_type,
	    const double min_consistency,
	    const bool do_relax,
	    const double n_smoothing_side_len,
	    const double dn_smoothing_side_len,
	    const bool debug_flag = false,
	    const bool verbose_flag = false);
  

  /**
   * @brief Process the given scan data.
   *
   * @param[in,out] data_file The file containing the scan data.  The output
   *                          fields are added to this file before returning.
   *
   * @return Returns true on success, false on failure.
   */

  bool processScan(DsMdvx &data_file);
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief The field name of the average I field in the calibration file.
   */

  static const string CALIB_AV_I_FIELD_NAME;
  
  /**
   * @brief The field name of the average Q field in the calibration file.
   */

  static const string CALIB_AV_Q_FIELD_NAME;
  
  /**
   * @brief The field name of the phase error field in the calibration file.
   */

  static const string CALIB_PHASE_ER_FIELD_NAME;
  
  /**
   * @brief The speed of light in a vacuum.
   */

  static const double C_VACUUM;

  /**
   * @brief Abruptness of the transition between good and bad.  Small equals
   *        slow transition.
   */

  static const double ABRUPT_FACTOR;

  /**
   * @brief "Maximum" "spectrum width" in m/s.
   */

  static const double THRESH_WIDTH;
  
  /**
   * @brief Minimum allowable output N value.
   */

  static const double MIN_N_VALUE;
  
  /**
   * @brief Maximum allowable output N value.
   */

  static const double MAX_N_VALUE;
  
  /**
   * @brief Minimum allowable output delta N value.
   */

  static const double MIN_DN_VALUE;
  
  /**
   * @brief Maximum allowable output delta N value.
   */

  static const double MAX_DN_VALUE;
  
  /**
   * @brief Minimum allowable output sigma N value.
   */

  static const double MIN_SIGMA_N_VALUE;
  
  /**
   * @brief Maximum allowable output sigma N value.
   */

  static const double MAX_SIGMA_N_VALUE;
  
  /**
   * @brief Minimum allowable output sigma delta N value.
   */

  static const double MIN_SIGMA_DN_VALUE;
  
  /**
   * @brief Maximum allowable output sigma delat N value.
   */

  static const double MAX_SIGMA_DN_VALUE;
  

  /////////////////////////////////////////////
  // Private members -- algorithm parameters //
  /////////////////////////////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  
  /**
   * @brief Pointer to the calibration file.  This pointer is owned by the
   *        parent class and must not be deleted here.
   */

  DsMdvx *_calibFile;
  
  /**
   * @brief Number of azimuths used in data processing.
   */

  int _numBeams;
  
  /**
   * @brief Number of range bins.
   */

  int _numGates;
  
  /**
   * @brief The type of quality calculation to perform.
   */

  quality_type_t _qualityType;
  
  /**
   * @brief Minimum range gate of ground echo.
   */

  int _rMin;
  
  /**
   * @brief Minimum consistency of phase to accept N (DN) measurement.
   *        Higher means smaller coverage of (hopefully) better data.
   */

  double _minConsistency;
  
  /**
   * @brief Flag indicating whether to do the relaxation stage.
   */

  bool _doRelax;
  
  /**
   * @brief N smoothing side lenth in meters.
   */

  double _nSmoothingSideLen;
  
  /**
   * @brief DN smoothing side length in meters.
   */

  double _dnSmoothingSideLen;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Flag indicating whether we are currently processing the first
   *        input file.
   */
  
  bool _firstFile;

  /**
   * @brief Phase difference between current and reference target phase.
   *        There are _numBeams * _numGates values in this array.
   */

  TargetData *_difFromRef;
  
  /**
   * @brief Phase (in I/Q form) difference between 2 successive scans.
   *        There are _numBeams * _numGates values in this array.
   */

  TargetData *_difPrevScan;

  /**
   * @brief In-phase (I) and quadrature (Q) components of phase of target.
   *        There are _numBeams * _numGates values in this array.
   */

  TargetData *_rawPhase;

  /**
   * @brief Information on targets at each azimuth-range cell.  There are
   *        _numBeams * _numGates values in this array.
   */

  TargetInfo *_target;

  /**
   * @brief The calculated N field.
   */

  MdvxField *_nField;

  /**
   * @brief The calculated delta N field.
   */

  MdvxField *_dnField;

  /**
   * @brief The calculated sigma N field.
   */

  MdvxField *_sigmaNField;

  /**
   * @brief The calculated sigma delta N field.
   */

  MdvxField *_sigmaDnField;

  /**
   * @brief The wavelength of the radar beam.
   */

  double _wavelength;
  

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
   * @brief Create the indicated MDV field and fill in the data with
   *        missing data values.
   *
   * @param[in] base_field The base field that this field should look like.
   * @param[in] field_name The field name.
   * @param[in] units the field units.
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   */

  MdvxField *_createMdvField(const MdvxField &base_field,
			     const string &field_name,
			     const string &units) const;
  

  /**
   * @brief Create the output field objects, filling in the data values with
   *        missing values.
   *
   * @param[in] base_field The base field that the output fields should look
   *                       like.
   *
   * @return Returns true on success, false on failure.
   */

  bool _createOutputFields(const MdvxField &base_field);
  

  /**
   * @brief Compute the phase difference between the last two scans as well
   *        as between the current scan and the reference scan.
   *
   * @return Returns true on success, false on failure.
   */

  bool _difPhase();
  

  /**
   * @brief Manage the generation of estimates of N(x,y) and dN(x,y)/dt.  This
   *        method directs the phase smoothing and N generation processes for
   *        both the current-to-previous-scan (dN/dt) and the current-to-
   *        reference (N) phase difference fields.
   *
   * @param[in] gate_spacing Data gate spacing in meters.
   *
   * @return Returns true on success, false on failure.
   */

  bool _fitPhases(const double gate_spacing);
  

  /**
   * @brief Free all allocated arrays.
   */

  void _freeArrays();
  

  /**
   * @brief Use the information from signal-to-noise ratio and spectrum width
   *        fields to determine the "quality" of a target for refractive index
   *        calculation purposes.  Then normalize the average I & Q arrays to
   *        that quality value.
   *
   * @param[in] snr_field The SNR field.
   * @param[in] sw_field The SW field.
   *
   * @return Returns true on success, false on failure.
   */

  bool _getQuality(const MdvxField &snr_field,
		   const MdvxField &sw_field);
  

  /**
   * @brief Create the masked output field to be added to the output
   *        file.
   *
   * @param[in] mask_field The mask field.
   * @param[in] output_field The unmasked output field.
   * @param[in] min_data_value Minimum allowable output data value.
   * @param[in] max_data_value Maximum allowable output data value.
   *
   * @return Returns a pointer to the newly created masked output field.
   */

  MdvxField *_maskOutputField(const MdvxField &mask_field,
			      const MdvxField &output_field,
			      const double min_data_value,
			      const double max_data_value) const;
  

};


#endif
