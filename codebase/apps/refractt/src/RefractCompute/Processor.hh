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
 * Class for doing all of the data processing.
 *
 * @class Processor
 * Class for doing all of the data processing.
 */

#ifndef Processor_HH
#define Processor_HH

#include <string>
#include "Params.hh"
#include "TargetVector.hh"
#include "CalibDayNight.hh"
#include <Refract/RefParms.hh>
#include <Refract/FieldWithData.hh>
#include <Refract/FieldDataPair.hh>

class DsMdvx;
class MdvxField;

class Processor
{
 public:

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
   * @param[in] calib_file Pointer to the calibration file information.
   * @param[in] parms  params
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init(CalibDayNight *calib, const RefParms &refparms,
	    const Params &params);

  /**
   * @brief Process the given scan data.
   *
   * @param[in,out] data_file The file containing the scan data.  The output
   *                          fields are added to this file before returning.
   *
   * @return true on success, false on failure.
   */

  bool processScan(const RefractInput &input, const time_t &t,
		   DsMdvx &data_file);
  

 private:

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
  

  /**
   * @brief Pointer to the calibration file.  This pointer is owned by the
   *        parent class and must not be deleted here.
   */
  CalibDayNight *_calib;

  RefParms _refparms;  /**< Parameters */
  Params _parms;       /**< Parameters */

  /**
   * @brief Flag indicating whether we are currently processing the first
   *        input file.
   */
  bool _firstFile;

  /**
   * @brief Phase difference between current and reference target phase.
   */
  FieldDataPair _difFromRef;
  
  /**
   * @brief Phase (in I/Q form) difference between 2 successive scans.
   */
  FieldDataPair _difPrevScan;

  /**
   * @brief In-phase (I) and quadrature (Q) components of phase of target.
   */
  FieldDataPair _rawPhase;

  /**
   * @brief Information on targets at each azimuth-range cell.
   */
  TargetVector _target;

  /**
   * @brief The calculated N field.
   */
  FieldWithData _nField;

  /**
   * @brief The calculated delta N field.
   */
  FieldWithData _dnField;

  /**
   * @brief The calculated sigma N field.
   */
  FieldWithData _sigmaNField;

  /**
   * @brief The calculated sigma delta N field.
   */
  FieldWithData _sigmaDnField;

  /**
   * @brief The wavelength of the radar beam.
   */
  double _wavelength;
  
  /**
   * Number of points per scan
   */
  int _nptScan;

  /**
   * True for first scan
   */
  bool _first;


  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Create the output field objects, filling in the data values with
   *        missing values.
   *
   * @param[in] base_field The base field that the output fields should look
   *                       like.
   *
   * @return Returns true on success, false on failure.
   */

  bool _createOutputFields(const FieldDataPair &iq);
  

  /**
   * @brief Compute the phase difference between the last two scans as well
   *        as between the current scan and the reference scan.
   *
   * @return Returns true on success, false on failure.
   */

  bool _difPhase(const time_t &t);
  

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

  bool _fitPhases(int numBeams, int numGates,  double gate_spacing);
  

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

  bool _getQuality(const FieldWithData &snr, const FieldWithData &quality,
		   const time_t &t, FieldWithData &qualityOutputField);
  
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
			      const FieldWithData &output_field,
			      const double min_data_value,
			      const double max_data_value) const;
  
  MdvxField *_maskOutputField(const MdvxField &mask_field,
			      const FieldWithData &output_field,
			      const double min_data_value,
			      const double max_data_value,
			      const FieldWithData &quality,
			      double quality_thresh) const;

  MdvxField *_maskOutputField(const MdvxField &mask_field,
			      const MdvxField &mask_field2,
			      const FieldWithData &output_field,
			      const double min_data_value,
			      const double max_data_value,
			      const FieldWithData &quality,
			      double quality_thresh) const;

  MdvxField *_maskOutputField(const MdvxField &mask_field,
			      const MdvxField &mask_field2,
			      double mask_field2_threshold,
			      const FieldWithData &output_field,
			      const double min_data_value,
			      const double max_data_value,
			      const FieldWithData &quality,
			      double quality_thresh) const;

};


#endif
